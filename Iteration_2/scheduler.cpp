#include "scheduler.h"
#include "floor.h"
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono> 
#include "globals.h"

// Queue for sending requests from Scheduler to Elevator
std::queue<FloorRequest> schedulerToElevator;
std::mutex mtxSchedulerToElevator;
std::condition_variable cvSchedulerToElevator;

// Queue for sending requests from Elevator to Scheduler
std::queue<FloorRequest> elevatorToScheduler;
std::mutex mtxElevatorToScheduler;
std::condition_variable cvElevatorToScheduler;

// Queue for sending requests from Scheduler to Floor
std::queue<FloorRequest> schedulerToFloor;
std::mutex mtxSchedulerToFloor;
std::condition_variable cvSchedulerToFloor;

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
    SchedulerState currentState = SchedulerState::WAIT_FOR_REQUEST;
    FloorRequest request;
    FloorRequest completedRequest;

    while (true) {
        switch (currentState) {
            case SchedulerState::WAIT_FOR_REQUEST: {
                // Acquiring lock to access critical section ('floorToScheduler' queue) after Elevator proccessed a request:
                std::unique_lock<std::mutex> lock(mtxFloorToScheduler);
                // Wait until a new request to be added from the Floor thread
                cvFloorToScheduler.wait(lock, [] { return !floorToScheduler.empty(); }); 

                // Acquiring lock to access critical section after Floor added a new request:
                std::cout << "[Scheduler] Received request..." << std::endl;
                // Retrieve the request from the `floorToScheduler` queue:
                request = floorToScheduler.front();
                floorToScheduler.pop();  // Pop first element from "floorToScheduler" queue
                lock.unlock();

                currentState = SchedulerState::PROCESS_REQUEST;
                break;
            }

            case SchedulerState::PROCESS_REQUEST: {
                std::cout << "[Scheduler] Processing request: Time: " << request.timeStamp
                          << ", Floor: " << request.floor
                          << ", Direction: " << request.direction
                          << ", Destination: " << request.destination << std::endl;
                //std::this_thread::sleep_for(std::chrono::milliseconds(500));
                currentState = SchedulerState::SEND_TO_ELEVATOR;
                break;
            }

            case SchedulerState::SEND_TO_ELEVATOR: {
                {
                    std::lock_guard<std::mutex> lock(mtxSchedulerToElevator);
                    schedulerToElevator.push(request);
                }
                cvSchedulerToElevator.notify_one();
                std::cout << "[Scheduler] Sent request to Elevator queue" << std::endl;
                currentState = SchedulerState::WAIT_FOR_ELEVATOR;
                break;
            }

            case SchedulerState::WAIT_FOR_ELEVATOR: {
                std::unique_lock<std::mutex> lock(mtxElevatorToScheduler);
                cvElevatorToScheduler.wait(lock, [] { return !elevatorToScheduler.empty(); });
                completedRequest = elevatorToScheduler.front();
                elevatorToScheduler.pop();
                std::cout << "[Scheduler] Elevator completed request: Floor " << request.floor
                          << " -> Destination " << request.destination << std::endl;
                currentState = SchedulerState::NOTIFY_FLOOR;
                break;
            }

            case SchedulerState::NOTIFY_FLOOR: {
                {
                    std::lock_guard<std::mutex> lock(mtxSchedulerToFloor);
                    schedulerToFloor.push(request);
                }
                cvSchedulerToFloor.notify_one();
                pendingRequests--;
                if (pendingRequests == 0) {
                    stopThreads = true;
                    std::cout << "[Scheduler] All requests processed. Terminating..." << std::endl;
                    cvSchedulerToElevator.notify_all();
                    cvFloorToScheduler.notify_all();
                    cvElevatorToScheduler.notify_all();
                    currentState = SchedulerState::TERMINATE;
                } else {
                    currentState = SchedulerState::WAIT_FOR_REQUEST;
                }
                break;
            }

            case SchedulerState::TERMINATE: {
                return;
            }
        }
    }
};