#include "scheduler.h"
#include "globals.h"
#include <thread>
#include <chrono>

//Added for testing
//std::queue<FloorRequest> floorToScheduler;
//std::mutex mtxFloorToScheduler;
//std::condition_variable cvFloorToScheduler;

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
void Scheduler::processFloorRequests() {
    FloorRequest request;
    while (true) {
        switch(currentState) {
        case SchedulerState::WAIT_FOR_REQUEST: {
            // Wait for floorToScheduler or stop
            std::unique_lock<std::mutex> lk(mtxFloorToScheduler);
            cvFloorToScheduler.wait(lk, [] {
                return !floorToScheduler.empty() || stopThreads;
            });
            if (stopThreads && floorToScheduler.empty()) {
                currentState = SchedulerState::TERMINATE;
                break;
            }
            request = floorToScheduler.front();
            floorToScheduler.pop();
            std::cout << "[Scheduler] Received request from Floor.\n";
            lk.unlock();
            currentState = SchedulerState::PROCESS_REQUEST;
            break;
        }

        case SchedulerState::PROCESS_REQUEST: {
            std::cout << "[Scheduler] Processing request: Time=" << request.timeStamp
                      << " Floor=" << request.floor << " Dir=" << request.direction
                      << " Dest=" << request.destination << std::endl;
            currentState = SchedulerState::SEND_TO_ELEVATOR;
            break;
        }

        case SchedulerState::SEND_TO_ELEVATOR: {
            {
                std::lock_guard<std::mutex> lk(mtxSchedulerToElevator);
                schedulerToElevator.push(request);
            }
            cvSchedulerToElevator.notify_all();
            std::cout << "[Scheduler] Sent request to Elevator queue.\n";
            currentState = SchedulerState::WAIT_FOR_ELEVATOR;
            break;
        }

        case SchedulerState::WAIT_FOR_ELEVATOR: {
            // Wait for elevator to complete
            std::unique_lock<std::mutex> lk(mtxElevatorToScheduler);
            cvElevatorToScheduler.wait(lk, [] {
                return !elevatorToScheduler.empty() || stopThreads;
            });
            if (stopThreads && elevatorToScheduler.empty()) {
                currentState = SchedulerState::TERMINATE;
                break;
            }
            FloorRequest completedReq = elevatorToScheduler.front();
            elevatorToScheduler.pop();
            std::cout << "[Scheduler] Elevator completed request: Floor "
                      << completedReq.floor << " -> " << completedReq.destination << std::endl;
            request = completedReq; // store for next step
            currentState = SchedulerState::NOTIFY_FLOOR;
            break;
        }

        case SchedulerState::NOTIFY_FLOOR: {
            {
                std::lock_guard<std::mutex> lk(mtxSchedulerToFloor);
                schedulerToFloor.push(request);
            }
            cvSchedulerToFloor.notify_all();
            pendingRequests--;
            if (pendingRequests == 0) {
                std::cout << "[Scheduler] All requests processed. Setting stopThreads = true.\n";
                stopThreads = true;
                cvFloorToScheduler.notify_all();
                cvSchedulerToElevator.notify_all();
                cvElevatorToScheduler.notify_all();
                cvSchedulerToFloor.notify_all();
                currentState = SchedulerState::TERMINATE;
            } else {
                currentState = SchedulerState::WAIT_FOR_REQUEST;
            }
            break;
        }

        case SchedulerState::TERMINATE: {
            std::cout << "[Scheduler] Terminating...\n";
            return;
        }
        }
    }
}
