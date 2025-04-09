#include "Common.h"
#include <thread>
#include <atomic>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sstream>
#include <iostream>
#include <fstream>

// Atomic flag to stop all threads
static std::atomic<bool> stopAll(false);

// Structure to keep track of elevator status for scheduling decisions
struct ElevatorData {
    int id;
    int currentFloor;
    bool isFaulted;
};

// List of all elevators in the system
static std::vector<ElevatorData> elevInfo;

/**
 * @brief Choose the best elevator for a request using simple distance-based scheduling.
 */
int pickElevator(const FloorRequest &r){
    static int lastChosen = -1;
    
    int best = -1;
    int bestDist = 999999;

    // Loop through elevators in a round-robin fashion
    for (size_t offset = 1; offset <= elevInfo.size(); ++offset) {
        int i = (lastChosen + offset) % elevInfo.size();
        if (elevInfo[i].isFaulted) continue;

        int dist = std::abs(elevInfo[i].currentFloor - r.floor);
        if (dist < bestDist) {
            bestDist = dist;
            best = i;
        }
    }

    if (best != -1)
        lastChosen = best;

    return best;
}


int main(int argc,char* argv[]){
    if(argc < 4){
        // Removed std::cout
        return 1;
    }
    // Parsing ports from command that initializes scheduler process
    int schedPort = std::stoi(argv[1]); // Port this scheduler listens on
    int floorPort = std::stoi(argv[2]); // Port to acknowledge back to Floor
    int nElev     = std::stoi(argv[3]); // Number of elevators

    // Initialize elevator information
    elevInfo.resize(nElev);
    for(int i=0; i<nElev; ++i){
        elevInfo[i].id          = i;
        elevInfo[i].currentFloor= 1;
        elevInfo[i].isFaulted   = false;
    }
    
    // Create sockets to receive from floor and elevators
    int floorSock    = createBoundSocket(schedPort);     // listens from Floor
    int elevatorSock = createBoundSocket(schedPort+100); // completions/status from Elevator
    int sendSock     = socket(AF_INET, SOCK_DGRAM, 0);

    // =====================================
    // Floor -> Scheduler -> ELevator thread
    // =====================================
    std::thread floorThread([&]{
        while(!stopAll){
            std::string ip; int p;
            auto data = udpRecvString(floorSock, ip, p);
            if(data.empty()){
                simulateSleepMs(100);
                continue;
            }
            auto fr = deserializeRequest(data);

            // pick elevator
            int best = pickElevator(fr);
            fr.assignedElevator = best;

            // If no elevators are available
            if(best < 0){
                // no elevator => discard
                std::string ack = "ACK FROM SCHED";
                udpSendString(sendSock, ack, "127.0.0.1", floorPort);
                continue;
            }
        
            // Normal case: send to elevator
            int ePort = 5000 + best;
            auto msg  = serializeRequest(fr);
            udpSendString(sendSock, msg, "127.0.0.1", ePort);

            udpSendString(sendSock, msg, "127.0.0.1", 6001);
        }
    });

    // ===========================
    // Elevator -> Scheduler -> Floor thread
    // ===========================
    std::thread elevatorThread([&]{
        while(!stopAll){
            std::string ip; int p;
            auto data = udpRecvString(elevatorSock, ip, p);
            if(data.empty()){
                simulateSleepMs(100);
                continue;
            }

            // Elevator status update message
            if(data.rfind("STATUS|", 0)==0){
                std::istringstream iss(data);
                std::string token;
                
                 // Parse message in format STATUS|ElevID=x|Floor=y|Fault=z..
                getline(iss, token, '|'); // "STATUS"
                getline(iss, token, '|'); // "ElevID=x"
                int eqPos = token.find('=');
                int eID = std::stoi(token.substr(eqPos+1));

                getline(iss, token, '|'); // "Floor=y"
                eqPos = token.find('=');
                int floor = std::stoi(token.substr(eqPos+1));

                getline(iss, token, '|'); // "Fault=z"
                eqPos = token.find('=');
                bool fault = (std::stoi(token.substr(eqPos+1))==1);

                elevInfo[eID].currentFloor= floor;
                elevInfo[eID].isFaulted   = fault;
                continue;
            }
            // else, treat it as a completion message
            auto fr = deserializeRequest(data);
            
            // Send ACK to floor so it knows the request completed
            std::string ack = "ACK FROM SCHED";
            udpSendString(sendSock, ack, "127.0.0.1", floorPort);
        }
    });

    // Keep scheduler running
    while(true){
        simulateSleepMs(500);
    }

    // Cleanup logic 
    stopAll=true;
    floorThread.join();
    elevatorThread.join();
    close(floorSock);
    close(elevatorSock);
    close(sendSock);
    return 0;
}
