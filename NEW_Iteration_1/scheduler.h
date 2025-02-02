#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "FloorRequest.h"

// Queue for sending requests from Scheduler -> Elevator
extern std::queue<FloorRequest> schedulerToElevator;
extern std::mutex mtxSchedulerToElevator;
extern std::condition_variable cvSchedulerToElevator;

// Queue for sending requests from Elevator -> Scheduler
extern std::queue<FloorRequest> elevatorToScheduler;
extern std::mutex mtxElevatorToScheduler;
extern std::condition_variable cvElevatorToScheduler;

// Queue for sending requests from Scheduler -> Floor
extern std::queue<FloorRequest> schedulerToFloor;
extern std::mutex mtxSchedulerToFloor;
extern std::condition_variable cvSchedulerToFloor;

class Scheduler {
public:
    void processFloorRequests();
    void processElevatorArrivals();
};

#endif // SCHEDULER_H