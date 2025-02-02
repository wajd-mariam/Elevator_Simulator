#ifndef ELEVATOR_H
#define ELEVATOR_H

#include "floor.h"
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <random>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <list>
#include <deque>
#include "FloorRequest.h"

// Queue for sending requests from Scheduler -> Elevator
extern std::queue<FloorRequest> schedulerToElevator;
extern std::mutex mtxSchedulerToElevator;
extern std::condition_variable cvSchedulerToElevator;

// Queue for sending requests from Elevator -> Scheduler
extern std::queue<FloorRequest> elevatorToScheduler;
extern std::mutex mtxElevatorToScheduler;
extern std::condition_variable cvElevatorToScheduler;

class Elevator {
private:
    bool elevatorMoving; //says when elevator
                        //indicates when a new task can be inputted
                        //can be inputted when not moving essentially (will be patched late4r)
    bool doorsOpen; // for later use
    int floorAt; // indicates the floor number the elevator is located on
                // used to tell if its going in the proper direction 
    int floorGoingTo; // loaded in by scheduler

public:
    /**
     * @brief Continuously processes elevator requests from the Scheduler.
     *
     * This function continuously checks for new elevator requests
     * from the `schedulerToElevator` sharedqueue. It fetches request from Scheduler (real-time),
     *  processes them, and then notifies the Scheduler when a request is completed.
     *
     * Workflow:
     * 1. Wait for a request from the Scheduler (`cvSchedulerToElevator`).
     * 2. If all requests are processed (`stopThreads == true`), terminate.
     * 3. Retrieve and validate the request.
     * 4. Simulate moving the elevator to the requested floor.
     * 5. Notify the Scheduler that the request has been completed.
     */
    void processRequests();
};

#endif // ELEVATOR_H