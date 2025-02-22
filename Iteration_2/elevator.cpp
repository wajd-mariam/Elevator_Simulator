#include "elevator.h"
#include "globals.h"
#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>

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
void Elevator::processRequests() {
    while (true) {
        std::unique_lock<std::mutex> lk(mtxSchedulerToElevator);

        switch (currentState) {
        case ElevatorState::WAIT_FOR_SCHEDULER: {
            // Wait for a request or stop
            cvSchedulerToElevator.wait(lk, [] {
                return !schedulerToElevator.empty() || stopThreads;
            });
            if (stopThreads && schedulerToElevator.empty()) {
                std::cout << "[Elevator] No more requests. Terminating...\n";
                return;
            }
            currentState = ElevatorState::RECEIVE_INSTRUCTIONS;
            break;
        }

        case ElevatorState::RECEIVE_INSTRUCTIONS: {
            // Grab the next request
            FloorRequest req = schedulerToElevator.front();
            schedulerToElevator.pop();
            std::cout << "[Elevator] Received Request -> Floor: " << req.floor
                      << ", Destination: " << req.destination
                      << ", Direction: " << req.direction << std::endl;

            // If doors are open, we'll close them
            if (doorsOpen) {
                currentState = ElevatorState::CLOSE_DOORS;
            }
            // Unlock to simulate movement
            lk.unlock();

            // 1) Close doors if open
            if (doorsOpen) {
                std::cout << "[Elevator] Doors closing...\n";
                doorsOpen = false;
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
            }

            // 2) Move from currentFloor to pickupFloor
            currentState = ElevatorState::MOVING_TO_FLOOR;
            int pickupDistance = std::abs(req.floor - currentFloor);
            if (pickupDistance > 0) {
                std::cout << "[Elevator] Moving from floor " << currentFloor
                          << " to floor " << req.floor << "...\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(pickupDistance * floorMovementSpeed));
                currentFloor = req.floor;
            }

            // 3) Open doors at pickup
            currentState = ElevatorState::OPEN_DOORS;
            std::cout << "[Elevator] Doors opening at floor " << currentFloor << "...\n";
            doorsOpen = true;
            std::this_thread::sleep_for(std::chrono::milliseconds(300));

            // 4) Close doors again
            currentState = ElevatorState::CLOSE_DOORS;
            std::cout << "[Elevator] Doors closing...\n";
            doorsOpen = false;
            std::this_thread::sleep_for(std::chrono::milliseconds(300));

            // 5) Move to destination
            currentState = ElevatorState::MOVING_TO_FLOOR;
            int destDistance = std::abs(req.destination - currentFloor);
            if (destDistance > 0) {
                std::cout << "[Elevator] Moving from floor " << currentFloor
                          << " to floor " << req.destination << "...\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(destDistance * floorMovementSpeed));
                currentFloor = req.destination;
            }

            // 6) Open doors at destination
            currentState = ElevatorState::OPEN_DOORS;
            std::cout << "[Elevator] Doors opening at floor " << currentFloor << "...\n";
            doorsOpen = true;
            std::this_thread::sleep_for(std::chrono::milliseconds(300));

            // Re-lock so we can update global queue
            lk.lock();

            // 7) SEND_FEEDBACK_TO_SCHEDULER
            currentState = ElevatorState::SEND_FEEDBACK_TO_SCHEDULER;
            elevatorToScheduler.push(req);
            cvElevatorToScheduler.notify_all();
            std::cout << "[Elevator] Dropped off passenger at floor " << req.destination << "\n";

            break;
        }

        case ElevatorState::MOVING_TO_FLOOR:
        case ElevatorState::CLOSE_DOORS:
        case ElevatorState::OPEN_DOORS:
            // Typically these are just transitions
            break;

        case ElevatorState::SEND_FEEDBACK_TO_SCHEDULER: {
            currentState = ElevatorState::WAIT_FOR_SCHEDULER;
            break;
        }

        case ElevatorState::STOPPED: {
            std::cout << "[Elevator] STOPPED state encountered.\n";
            break;
        }

        } // end switch
    } // end while
}
