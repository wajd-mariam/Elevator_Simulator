#include "elevator.h"
#include "scheduler.h"
#include <atomic>

// Global variables used to control program terminiation:
extern std::atomic<int> pendingRequests;
extern std::atomic<bool> stopThreads;
  
enum class ElevatorState{
    WAIT_FOR_SCHEDULER,
    RECEIVE_INSTRUCTIONS,
    UNPACK_INSTRUCTIONS,
    CLOSE_DOORS,
    MOVING_TO_FLOOR,
    SEND_FEEDBACK_TO_SCHEDULER,
    OPEN_DOORS
};

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
    ElevatorState currentState = ElevatorState::WAIT_FOR_SCHEDULER; // start program at "Wait for scheduler"
    FloorRequest request("", 0, "", 0); // Initialize with default values
    while (true) {
        std::unique_lock<std::mutex> lock(mtxSchedulerToElevator);

        switch(currentState) {
            case ElevatorState::WAIT_FOR_SCHEDULER: {
                // Wait for a request from the Scheduler Subsystem
                cvSchedulerToElevator.wait(lock, [] { return !schedulerToElevator.empty() || stopThreads; });

                // Terminating if all requests have been processed:
                if (stopThreads = true && schedulerToElevator.empty()) {
                    std::cout << "[Elevator] No more requests. Terminating..." << std::endl;
                    return;
                }
                
                currentState = ElevatorState::RECEIVE_INSTRUCTIONS;
                break;
            }

            case ElevatorState::RECEIVE_INSTRUCTIONS: {
                // Retrieve the request from the schedulerToElevator queue
                request = schedulerToElevator.front();
                schedulerToElevator.pop(); // Pop first element from "schedulerToElevator" queue
                // Print out request details
                std::cout << "[Elevator] Received Request -> Floor: " << request.floor 
                          << ", Destination: " << request.destination 
                          << ", Direction: " << request.direction << std::endl;
                lock.unlock(); // Unlock the queue after fetching the request
                currentState = ElevatorState::UNPACK_INSTRUCTIONS;
                break;
            }

            case ElevatorState::UNPACK_INSTRUCTIONS: {
                if (request.floor == 0 && request.destination == 0 && request.direction.empty()) {
                    std::cerr << "[ERROR] Elevator received an invalid request!" << std::endl;
                    currentState = ElevatorState::WAIT_FOR_SCHEDULER; // Restart waiting state
                } else {
                    currentState = ElevatorState::CLOSE_DOORS;
                }
                break;
            }

            case ElevatorState::CLOSE_DOORS: { 
                std::cout << "[Elevator] Doors closing..." << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(4));
                currentState = ElevatorState::MOVING_TO_FLOOR;
                break;
            }

            case ElevatorState::MOVING_TO_FLOOR: {
                // Another time waster, will implement true random values later
                std::cout << "[Elevator] Moving from Floor " << request.floor
                << " to Destination " << request.destination
                << " (Direction: " << request.direction << ")" << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(6));
                currentState = ElevatorState::MOVING_TO_FLOOR;               
            }

            case ElevatorState::SEND_FEEDBACK_TO_SCHEDULER: {
                //  Notify Scheduler that the Elevator has arrived and completed the request
                {
                    std::lock_guard<std::mutex> lock(mtxElevatorToScheduler);
                    // Adding the completed request to the "elevatorToScheduler" shared queue for Scheduler to process it.
                    elevatorToScheduler.push(request);
                }
                cvElevatorToScheduler.notify_one(); // Notify Scheduler
                
                currentState = ElevatorState::OPEN_DOORS;
                break;
            }

            case ElevatorState::OPEN_DOORS: {
                std::cout << "[Elevator] Doors opening..." << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(4));
                currentState = ElevatorState::WAIT_FOR_SCHEDULER; // Reset state
                break;
            }

            default:
                std::cerr << "[ERROR] Invalid State Encountered!" << std::endl;
                currentState = ElevatorState::WAIT_FOR_SCHEDULER;
                break;
        }
    }
}
