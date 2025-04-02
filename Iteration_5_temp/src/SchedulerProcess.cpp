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

static std::atomic<bool> stopAll(false);

struct ElevatorData {
    int id;
    int currentFloor;
    bool isFaulted;
};

static std::vector<ElevatorData> elevInfo;

int pickElevator(const FloorRequest &r){
    static int lastChosen = -1;
    
    int best=-1; 
    int bestDist=999999;
    for(size_t i=0; i<elevInfo.size(); ++i){
        if(elevInfo[i].isFaulted) continue;
        int dist = std::abs(elevInfo[i].currentFloor - r.floor);
        if(dist < bestDist){
            bestDist = dist;
            best = (int)i;
        } else if (dist == bestDist) {
            // Tie-breaker: pick next after lastChosen
            if ((int)i > lastChosen) {
                best = i;
            }
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
    int schedPort = std::stoi(argv[1]);
    int floorPort = std::stoi(argv[2]);
    int nElev     = std::stoi(argv[3]);

    elevInfo.resize(nElev);
    for(int i=0; i<nElev; ++i){
        elevInfo[i].id          = i;
        elevInfo[i].currentFloor= 1;
        elevInfo[i].isFaulted   = false;
    }

    int floorSock    = createBoundSocket(schedPort);     // listens from Floor
    int elevatorSock = createBoundSocket(schedPort+100); // completions/status from Elevator
    int sendSock     = socket(AF_INET, SOCK_DGRAM, 0);

    // Floor -> Scheduler
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
            if(best < 0){
                // no elevator => discard
                std::string ack = "ACK FROM SCHED";
                udpSendString(sendSock, ack, "127.0.0.1", floorPort);
                continue;
            }
            int ePort = 5000 + best;
            auto msg  = serializeRequest(fr);
            udpSendString(sendSock, msg, "127.0.0.1", ePort);
        }
    });

    // Elevator -> Scheduler
    std::thread elevatorThread([&]{
        while(!stopAll){
            std::string ip; int p;
            auto data = udpRecvString(elevatorSock, ip, p);
            if(data.empty()){
                simulateSleepMs(100);
                continue;
            }
            // check if it's a STATUS or completion
            if(data.rfind("STATUS|", 0)==0){
                std::istringstream iss(data);
                std::string token;
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
            // else normal completion
            auto fr = deserializeRequest(data);
            // ack floor
            std::string ack = "ACK FROM SCHED";
            udpSendString(sendSock, ack, "127.0.0.1", floorPort);
        }
    });

    while(true){
        simulateSleepMs(500);
    }
    stopAll=true;
    floorThread.join();
    elevatorThread.join();
    close(floorSock);
    close(elevatorSock);
    close(sendSock);
    return 0;
}
