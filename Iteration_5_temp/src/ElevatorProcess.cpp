#include "Common.h"
#include <iostream>
#include <thread>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sstream>

// We'll assume your existing logic uses these for state:
static bool hardFault = false;
static bool doorsOpen = true;
static int  currentFloor = 1;

static ElevatorState eState = ElevatorState::WAITING;

static std::atomic<bool> stopAll(false);

static std::string direction = "IDLE";

static int currentRequestID = -1;

/**
 * @brief Send an up-to-date "STATUS" message to the Scheduler. 
 * Format: STATUS|ElevID=x|Floor=y|Fault=z
 */
static void sendElevatorStatus(int sock, int elevatorID, const std::string &schedIP, int schedPort, const std::string& direction, const std::string& state, int requestID) {
    std::ostringstream oss;
    oss << "STATUS|ElevID=" << elevatorID
        << "|Floor=" << currentFloor
        << "|Fault=" << (hardFault ? 1 : 0)
        << "|Direction=" << direction
        << "|State=" << (hardFault ? "FAULT" : (eState == ElevatorState::MOVING ? "MOVING" : "WAITING"))
        << "|RequestID=" << currentRequestID;

    // For minimal changes, we reuse schedPort+100 for 'status' 
    // or you can use the same port the elevator uses for completions
    udpSendString(sock, oss.str(), schedIP, schedPort + 100);

    udpSendString(sock, oss.str(), "127.0.0.1", 6000);
}

std::string elevatorStateToString(ElevatorState state) {
    switch (state) {
        case ElevatorState::WAITING: return "WAITING";
        case ElevatorState::MOVING: return "MOVING";
        case ElevatorState::DOORS_OPEN: return "DOORS OPEN";
        case ElevatorState::STOPPED: return "STOPPED";
        default: return "UNKNOWN";
    }
}

int main(int argc, char* argv[]){
    // Usage: ./ElevatorProcess <listenPort> <elevatorID> <schedulerIP> <schedulerPort>
    if(argc < 5){
        std::cerr << "Usage: " << argv[0]
                  << " <listenPort> <elevatorID> <schedIP> <schedPort>\n";
        return 1;
    }

    int myPort     = std::stoi(argv[1]);
    int elevatorID = std::stoi(argv[2]);
    std::string schedIP = argv[3];
    int schedPort  = std::stoi(argv[4]);

    // 1) Create sockets
    int sock     = createBoundSocket(myPort);
    if(sock < 0){
        std::cerr<<"[Elevator "<<elevatorID<<"] Could not bind on port "<<myPort<<"\n";
        return 1;
    }
    int sendSock = socket(AF_INET, SOCK_DGRAM, 0);  // For completions + status

    // 2) Continuously receive requests from the Scheduler
    std::thread listener([&]{
        while(!stopAll){
            std::string ip; 
            int port;
            auto data = udpRecvString(sock, ip, port);
            if(data.empty()){
                simulateSleepMs(50);
                continue;
            }
            // It's presumably a normal floor request:
            FloorRequest fr = deserializeRequest(data);

            currentRequestID = fr.requestID;
            
            // Setting direction after deserialzing FloorReuqest fr
            std::string direction;
            if (fr.destination > currentFloor)
                direction = "UP";
            else if (fr.destination < currentFloor)
                direction = "DOWN";
            else
                direction = "IDLE";

            // Mark elevator as moving
            eState = ElevatorState::MOVING;
            // Send updated status so scheduler sees new state + floor
            sendElevatorStatus(sendSock, elevatorID, schedIP, schedPort, direction, elevatorStateToString(eState), fr.requestID);

            // Simulate movement from currentFloor -> pickup
            int dist = std::abs(fr.floor - currentFloor);
            simulateSleepMs(dist * 500);
            currentFloor= fr.floor;

            // Determining direction: UP or DOWN or IDLE
            if (fr.floor > currentFloor) direction = "UP";
            else if (fr.floor < currentFloor) direction = "DOWN";
            else direction = "IDLE";

            // Doors open
            doorsOpen=true; simulateSleepMs(300);
            // Doors close
            doorsOpen=false;simulateSleepMs(300);

            // Move from pickup -> destination
            dist= std::abs(fr.destination - currentFloor);
            simulateSleepMs(dist*500);
            currentFloor= fr.destination;
            doorsOpen=true; simulateSleepMs(300);

            //std::cout<<"[Elevator "<<elevatorID<<"] Completed request.\n";

            // Send completion message => schedPort+100
            fr.floor       = currentFloor;
            fr.destination = currentFloor;
            fr.timeStamp += "(ElevID="+std::to_string(elevatorID)+")";
            auto msg = serializeRequest(fr);
            udpSendString(sendSock, msg, schedIP, schedPort + 100);

            // Mark elevator as WAITING
            eState= ElevatorState::WAITING;
            // Send updated status
            sendElevatorStatus(sendSock, elevatorID, schedIP, schedPort, direction, elevatorStateToString(eState), fr.requestID);
        }
    });
    listener.detach();

    // Keep running until user kills it
    while(true){
        simulateSleepMs(500);
    }
    stopAll=true;

    close(sock);
    close(sendSock);
    return 0;
}
