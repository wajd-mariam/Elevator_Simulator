#include "scheduler.h"

void Scheduler::processFloorRequests() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtxFloorToScheduler);

        // Wait for a request from the Floor Subsystem
        cv.wait(lock, [] { return !floorToScheduler.empty(); });    

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
    }
}