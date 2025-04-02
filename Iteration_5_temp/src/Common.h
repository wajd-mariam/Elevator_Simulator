#ifndef COMMON_H
#define COMMON_H

#include <string>

// Represents a user request from a floor
struct FloorRequest {
    std::string timeStamp;
    int floor;
    std::string direction;  // "UP"/"DOWN"
    int destination;
    bool hasFault;          // if a fault is injected
    std::string faultType;  // "doorStuck", "stuckElevator", etc.
    int passengers;         // passenger count
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
    std::string direction;
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
