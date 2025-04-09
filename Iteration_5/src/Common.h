#ifndef COMMON_H
#define COMMON_H

#include <string>

// Represents a user request from a floor
struct FloorRequest {
    std::string timeStamp;
    int floor;
    std::string direction;  // "UP"/"DOWN"
    int destination;
    bool hasFault = false;          // if a fault is injected
    std::string faultType;  // "doorStuck", "stuckElevator", etc.
    int passengers = 0;         // passenger count
    int requestID = -1;

    FloorRequest() = default;
    
    // Constructor for FloorRequest object
    FloorRequest(const std::string& ts, int fl, const std::string& dir, int dest,
            bool fault, const std::string& fType, int pax)
    : timeStamp(ts), floor(fl), direction(dir), destination(dest),
    hasFault(fault), faultType(fType), passengers(pax) {}
};

// Basic elevator states
enum class ElevatorState {
    WAITING,
    RECEIVING,
    MOVING,
    SENDING_FEEDBACK,
    DOORS_OPEN,
    STOPPED
};

struct ElevatorStatus {
    int id;
    int currentFloor;
    bool doorsOpen;
    bool isFaulted;
    std::string state; // "WAITING", "MOVING", etc.
    std::string faultType = "noFault";
    std::string direction;
    int currentRequestID = -1;
};

// Socket helpers
int  createBoundSocket(int port);
void udpSendString(int sock, const std::string &msg, const std::string &ip, int port);
std::string udpRecvString(int sock, std::string &senderIP, int &senderPort);

// Serializers for requests / statuses
std::string serializeRequest(const FloorRequest &fr);
FloorRequest deserializeRequest(const std::string &s);

std::string serializeElevatorStatus(const ElevatorStatus &st);
ElevatorStatus deserializeElevatorStatus(const std::string &s);

// Simulates a blocking sleep in ms
void simulateSleepMs(int ms);

#endif
