#include "scheduler.h"
#include "floor.h"
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono> 

// Global variables used to control program terminiation:
extern std::atomic<int> pendingRequests;
extern std::atomic<bool> stopThreads; 

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
    while (true) {
        // Acquiring lock to access critical section ('floorToScheduler' queue) after Elevator proccessed a request:
        std::unique_lock<std::mutex> lock(mtxFloorToScheduler);
        // Wait until a new request to be added from the Floor thread
        cvFloorToScheduler.wait(lock, [] { return !floorToScheduler.empty(); }); 

        // Acquiring lock to access critical section after Floor added a new request:
        std::cout << "[Scheduler] Received request..." << std::endl;
        // Retrieve the request from the `floorToScheduler` queue:
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
        // Creating a new FloorRequest object 
        FloorRequest newRequest(request.timeStamp, request.floor, request.direction, request.destination);

        // Forward request to Elevator using "schedulerToElevator" shared queue
        {
            std::lock_guard<std::mutex> lock(mtxSchedulerToElevator);
            schedulerToElevator.push(newRequest);
        }
        // Notifying Elevator of new forwarded requests from Scheduler
        cvSchedulerToElevator.notify_one();
        std::cout << "[Scheduler] Sent request to Elevator queue" << std::endl;
        
        // Acquiring lock to access critical section ('elevatorToScheduler' queue) after Elevator proccessed a request:
        std::unique_lock<std::mutex> lockElevator(mtxElevatorToScheduler);
        // Wait for Elevator confirmation before processing the next request
        cvElevatorToScheduler.wait(lockElevator, [] { return !elevatorToScheduler.empty(); });

        // Retrieve the completed request from elevator and popping it from 'elevatorToScheduler' queue:
        FloorRequest completedRequest = elevatorToScheduler.front();
        elevatorToScheduler.pop();
        std::cout << "[Scheduler] Elevator completed request: Floor " << completedRequest.floor
                << " -> Destination " << completedRequest.destination << std::endl;
        
        // Forwarding completed request by Elevator to Floor subsystem
        {
            // Acquiring lock to access critical section ('schedulerToFloor' queue):
            std::lock_guard<std::mutex> lock(mtxSchedulerToFloor);
            // Adding the completed request to the queue
            schedulerToFloor.push(completedRequest);
        }

        // Wake up Floor thread to notify of Elevator completing a request
        cvSchedulerToFloor.notify_one();  
        pendingRequests--; // Decerement Global variable pendingRequests after processing a request by Elevator subsystem

        // If all requests from "input.txt" were processed -> terminate system
        if (pendingRequests == 0) {
            stopThreads = true;
            std::cout << "[Scheduler] All requests processed. Terminating..." << std::endl;
            // Wake up all condition variables to notify of system termination
            cvSchedulerToElevator.notify_all();  
            cvFloorToScheduler.notify_all();
            cvElevatorToScheduler.notify_all();
            return; // Terminiating Scheduler thread when all requests were processed
        }
    }
};