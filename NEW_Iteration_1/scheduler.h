#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "FloorRequest.h"

extern std::queue<FloorRequest> schedulerToElevator;
extern std::mutex mtxSchedulerToElevator;
extern std::condition_variable cvSchedulerToElevator;

extern std::queue<FloorRequest> elevatorToScheduler;
extern std::mutex mtxElevatorToScheduler;
extern std::condition_variable cvElevatorToScheduler;

class Scheduler {
public:
    void processFloorRequests();
};

#endif // SCHEDULER_H