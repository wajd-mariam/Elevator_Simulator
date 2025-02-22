#include "globals.h"

// Define global variables
std::atomic<int> pendingRequests(0);
std::atomic<bool> stopThreads(false);

// floor->scheduler
std::queue<FloorRequest> floorToScheduler;
std::mutex mtxFloorToScheduler;
std::condition_variable cvFloorToScheduler;

// scheduler->elevator
std::queue<FloorRequest> schedulerToElevator;
std::mutex mtxSchedulerToElevator;
std::condition_variable cvSchedulerToElevator;

// elevator->scheduler
std::queue<FloorRequest> elevatorToScheduler;
std::mutex mtxElevatorToScheduler;
std::condition_variable cvElevatorToScheduler;

// scheduler->floor
std::queue<FloorRequest> schedulerToFloor;
std::mutex mtxSchedulerToFloor;
std::condition_variable cvSchedulerToFloor;
