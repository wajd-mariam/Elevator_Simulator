#include "scheduler.h"
#include "floor.h"
#include <iostream>
#include <thread>
#include <chrono> 

std::queue<FloorRequest> schedulerToElevator;
std::mutex mtxSchedulerToElevator;
std::condition_variable cvSchedulerToElevator;

std::queue<FloorRequest> elevatorToScheduler;
std::mutex mtxElevatorToScheduler;
std::condition_variable cvElevatorToScheduler;

void Scheduler::processFloorRequests() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtxFloorToScheduler);

        // Wait for a request from the Floor Subsystem
        cvFloorToScheduler.wait(lock, [] { return !floorToScheduler.empty(); });    

        std::cout << "RECIEVING DATA FROM FLOOR" << std::endl;
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

        // Forward request to Elevator
        {
            std::lock_guard<std::mutex> lock(mtxSchedulerToElevator);
            schedulerToElevator.push(request);
        }
        cvSchedulerToElevator.notify_one();
        std::cout << "[Scheduler] Sent request to Elevator queue" << std::endl;
    }
}