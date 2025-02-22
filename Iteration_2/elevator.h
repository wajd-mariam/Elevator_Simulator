#ifndef ELEVATOR_H
#define ELEVATOR_H

#include <cmath>
#include <condition_variable>
#include "FloorRequest.h"

enum class ElevatorState {
    WAIT_FOR_SCHEDULER         = 0,
    RECEIVE_INSTRUCTIONS       = 1,
    CLOSE_DOORS                = 3,
    MOVING_TO_FLOOR            = 4,
    SEND_FEEDBACK_TO_SCHEDULER = 5,
    OPEN_DOORS                 = 6,
    STOPPED                    = 7
};

class Elevator {
private:
    ElevatorState currentState {ElevatorState::WAIT_FOR_SCHEDULER};
    int currentFloor {1};
    bool doorsOpen {true};
    int floorMovementSpeed {500};


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
