#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "FloorRequest.h"

// Define the states of the Scheduler
enum class SchedulerState {
    WAIT_FOR_REQUEST = 0,
    PROCESS_REQUEST  = 1,
    SEND_TO_ELEVATOR = 2,
    WAIT_FOR_ELEVATOR= 3,
    NOTIFY_FLOOR     = 4,
    TERMINATE        = 5
};


//Added for testing
//extern std::queue<FloorRequest> floorToScheduler;
//extern std::mutex mtxFloorToScheduler;
//extern std::condition_variable cvFloorToScheduler;

class Scheduler {
private:
    //Added for testing
    SchedulerState currentState{SchedulerState::WAIT_FOR_REQUEST};

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

    //Added for testing
    SchedulerState getCurrentState() const { return currentState;}
};

#endif // SCHEDULER_H
