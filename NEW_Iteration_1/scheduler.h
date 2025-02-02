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
    /**
     * @brief Continuously processes requests from the Floor subsystem and forwards them to the Elevator subsystem.
     * 
     * Workflow:
     * 1. Waits for new requests from the Floor subsystem (stored in `floorToScheduler` queue).
     * 2. Forwards requests to the Elevator subsystem (stored in `schedulerToElevator` queue).
     * 3. Waits for the Elevator to complete the request (`elevatorToScheduler` queue).
     * 4. Notifies the Floor once the request is completed.
     * 5. Terminates when all requests have been processed using global variables ('pendingRequests' & 'stopThreads')
     */
    void processFloorRequests();
};

#endif // SCHEDULER_H