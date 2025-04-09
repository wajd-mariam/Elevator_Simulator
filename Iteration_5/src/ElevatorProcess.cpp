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

// Variables used by each elevator
static bool hardFault = false;
static bool doorsOpen = true;
static int  currentFloor = 1;
static ElevatorState eState = ElevatorState::WAITING;
static std::atomic<bool> stopAll(false);
static std::string direction = "IDLE";
static std::string currentFaultType = "noFault";
static std::chrono::steady_clock::time_point lastFaultTime;

/**
 * @brief Send an up-to-date "STATUS" message to the Scheduler. 
 * Format: STATUS|ElevID=x|Floor=y|Fault=z|Direction=dir|State=state|FaultType=type
 */
static void sendElevatorStatus(int sock, int elevatorID, const std::string &schedIP, int schedPort, const std::string& direction, const std::string& state, const std::string &faultType) {
    std::ostringstream oss;
    oss << "STATUS|ElevID=" << elevatorID
        << "|Floor=" << currentFloor
        << "|Fault=" << (hardFault ? 1 : 0)
        << "|Direction=" << direction
        << "|State=" << state
        << "|FaultType=" << faultType;

    std::string msg = oss.str();
    //std::cout << "[DEBUG] Elevator " << elevatorID << " sending status: " << msg << std::endl;
    udpSendString(sock, oss.str(), schedIP, schedPort + 100);
    udpSendString(sock, oss.str(), "127.0.0.1", 6000);
}


/**
 * Converts the enum ElevatorState to a readable string.
 */
std::string elevatorStateToString(ElevatorState state) {
    switch (state) {
        case ElevatorState::WAITING: return "WAITING";
        case ElevatorState::MOVING: return "MOVING";
        case ElevatorState::DOORS_OPEN: return "DOORS OPEN";
        case ElevatorState::STOPPED: return "STOPPED";
        default: return "UNKNOWN";
    }
}

// Main function for every ElevatorProcess 
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

    // Create socket to receive and send messages
    int sock     = createBoundSocket(myPort);
    if(sock < 0){
        std::cerr<<"[Elevator "<<elevatorID<<"] Could not bind on port "<<myPort<<"\n";
        return 1;
    }
    int sendSock = socket(AF_INET, SOCK_DGRAM, 0);  // For completions + status

    // Thread to handle requests from Scheduler
    std::thread listener([&]{
        while(!stopAll){
            std::string ip; 
            int port;
            auto data = udpRecvString(sock, ip, port);
            if(data.empty()){
                simulateSleepMs(50);
                continue;
            }

            // Deserialize incoming FloorRequest from Scheduler
            FloorRequest fr = deserializeRequest(data);

            // Setting direction after deserialzing FloorReuqest fr
            std::string direction;
            if (fr.destination > currentFloor)
                direction = "UP";
            else if (fr.destination < currentFloor)
                direction = "DOWN";
            else
                direction = "IDLE";

            // Handle stuck elevator fault from FloorRequest fr
            hardFault = fr.hasFault;
            currentFaultType = fr.faultType;
            if (fr.hasFault) {
                lastFaultTime = std::chrono::steady_clock::now();  // mark when fault started
            }

            // Mark elevator as moving
            eState = ElevatorState::MOVING;
            // Send updated status so scheduler sees new state + floor
            sendElevatorStatus(sendSock, elevatorID, schedIP, schedPort, direction, elevatorStateToString(eState), fr.faultType);

            // Simulate movement from currentFloor -> pickup
            int dist = std::abs(fr.floor - currentFloor);
            simulateSleepMs(dist * 1500);
            currentFloor= fr.floor;

            // Doors open
            doorsOpen=true; simulateSleepMs(300);
            // Doors close
            doorsOpen=false;simulateSleepMs(300);

            // Move from pickup -> destination
            dist= std::abs(fr.destination - currentFloor);
            simulateSleepMs(dist*1500);
            currentFloor= fr.destination;
            doorsOpen=true; simulateSleepMs(300);

            // Send completion message to scheduler => schedPort+100
            fr.floor       = currentFloor;
            fr.destination = currentFloor;
            fr.timeStamp += "(ElevID="+std::to_string(elevatorID)+")";
            auto msg = serializeRequest(fr);
            udpSendString(sendSock, msg, schedIP, schedPort + 100);

            // Mark elevator as WAITING
            eState= ElevatorState::WAITING;
            // Send updated status
            sendElevatorStatus(sendSock, elevatorID, schedIP, schedPort, direction, elevatorStateToString(eState), fr.faultType);
        }
    });
    listener.detach(); // Run listener thread in background

    // Main loop: check if fault should be cleared. Keep running until user kills it
    while(true){
        simulateSleepMs(500);

        // Automatically clear hard faults after 5 seconds
        if (hardFault) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastFaultTime).count();
            if (elapsed > 5) {
                // Clear fault
                hardFault = false;
                currentFaultType = "noFault";
                sendElevatorStatus(sock, elevatorID, schedIP, schedPort, direction, elevatorStateToString(eState), currentFaultType);
            }
        }
    }
    
    // Clean up
    stopAll=true;
    close(sock);
    close(sendSock);
    return 0;
}
