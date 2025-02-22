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

enum class ElevatorState{
    WAIT_FOR_SCHEDULER,
    RECEIVE_INSTRUCTIONS,
    UNPACK_INSTRUCTIONS,
    CLOSE_DOORS,
    MOVING_TO_FLOOR,
    SEND_FEEDBACK_TO_SCHEDULER,
    OPEN_DOORS
};

class Elevator {
private:
    bool elevatorMoving; //says when elevator
                        //indicates when a new task can be inputted
                        //can be inputted when not moving essentially (will be patched late4r)
    int floorAt; // indicates the floor number the elevator is located on
                // used to tell if its going in the proper direction 
    int floorGoingTo; // loaded in by scheduler

    //Added for testing
    ElevatorState currentState{ElevatorState::WAIT_FOR_SCHEDULER};
    bool doorsOpen{true};
    int currentFloor{1};

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

    // Testing methods - added specifically for testing state transitions
    ElevatorState getCurrentState() const { return currentState; }
    bool getDoorsOpen() const { return doorsOpen; }
    int getCurrentFloor() const { return currentFloor; }
};

#endif // ELEVATOR_H