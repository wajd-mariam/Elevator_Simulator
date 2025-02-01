#include "scheduler.h"
#include "floor.h"
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono> 

// Global counter of pending requests:
extern std::atomic<int> pendingRequests;
extern std::atomic<bool> stopThreads; 

std::queue<FloorRequest> schedulerToElevator;
std::mutex mtxSchedulerToElevator;
std::condition_variable cvSchedulerToElevator;

std::queue<FloorRequest> elevatorToScheduler;
std::mutex mtxElevatorToScheduler;
std::condition_variable cvElevatorToScheduler;

std::queue<FloorRequest> schedulerToFloor;
std::mutex mtxSchedulerToFloor;
std::condition_variable cvSchedulerToFloor;

void Scheduler::processFloorRequests() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtxFloorToScheduler);
        // Wait for a request from the Floor Subsystem
        cvFloorToScheduler.wait(lock, [] { return !floorToScheduler.empty() || !elevatorToScheduler.empty() || pendingRequests > 0 || stopThreads; });   

        // Handle Elevator Arrivals FIRST (before sending new requests from schedular to elevator)
        /**if (!elevatorToScheduler.empty()) {
            FloorRequest arrivedAtFloor = elevatorToScheduler.front();
            elevatorToScheduler.pop();
            lock.unlock();

            std::cout << "[Scheduler] Elevator arrived at Floor " << arrivedAtFloor.destination << std::endl;
            
            pendingRequests--;
            std::cout << "DEBUG: pending request value:" << pendingRequests << std::endl;
            if (pendingRequests == 0) {
                stopThreads = true;
                std::cout << "[Scheduler] All requests processed. Terminating..." << std::endl;
                cvSchedulerToElevator.notify_all();  // Wake up elevator to notify of system shut down
                cvFloorToScheduler.notify_all();
                cvElevatorToScheduler.notify_all();
                return;
            }
            continue; 

            // Notify Floor:
            {
                std::lock_guard<std::mutex> lock(mtxSchedulerToFloor);
                schedulerToFloor.push(arrivedAtFloor);
            }
            cvSchedulerToFloor.notify_one();

            
        }*/

        // Check new requests from floor:
        if (!floorToScheduler.empty()) {
            std::cout << "[Scheduler] Processing request..." << std::endl;
            // Retrieve the request from the queue
            FloorRequest request = floorToScheduler.front();
            floorToScheduler.pop();  // Pop first element from "floorToScheduler" queue

            lock.unlock();

            std::cout << "[Scheduler] Processing request: "
                    << "Time: " << request.timeStamp
                    << ", Floor: " << request.floor
                    << ", Direction: " << request.direction
                    << ", Destination: " << request.destination << std::endl;
        
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            // To ensure correct data transfer and avoid data corruption:
            FloorRequest newRequest(request.timeStamp, request.floor, request.direction, request.destination);

            // Forward request to Elevator
            {
                std::lock_guard<std::mutex> lock(mtxSchedulerToElevator);
                schedulerToElevator.push(newRequest);
            }
            cvSchedulerToElevator.notify_one();
            std::cout << "[Scheduler] Sent request to Elevator queue" << std::endl;
        
            // Wait for Elevator confirmation before processing the next request
            std::unique_lock<std::mutex> lockElevator(mtxElevatorToScheduler);
            cvElevatorToScheduler.wait(lockElevator, [] { return !elevatorToScheduler.empty(); });

            FloorRequest completedRequest = elevatorToScheduler.front();
            elevatorToScheduler.pop();
            std::cout << "[Scheduler] Elevator completed request: Floor " << completedRequest.floor
                      << " -> Destination " << completedRequest.destination << std::endl;
            
            {
                std::lock_guard<std::mutex> lock(mtxSchedulerToFloor);
                schedulerToFloor.push(completedRequest);
            }
            cvSchedulerToFloor.notify_one();  // Wake up Floor

            pendingRequests--;
            if (pendingRequests == 0) {
                stopThreads = true;
                std::cout << "[Scheduler] All requests processed. Terminating..." << std::endl;
                cvSchedulerToElevator.notify_all();  // Wake up elevator to notify of system shut down
                cvFloorToScheduler.notify_all();
                cvElevatorToScheduler.notify_all();
                return;
            }
        }
    }
};