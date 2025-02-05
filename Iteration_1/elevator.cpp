#include "elevator.h"
#include "scheduler.h"
#include <atomic>

// Global variables used to control program terminiation:
extern std::atomic<int> pendingRequests;
extern std::atomic<bool> stopThreads;

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
        std::unique_lock<std::mutex> lock(mtxSchedulerToElevator);
        // Wait for a request from the Scheduler Subsystem
        cvSchedulerToElevator.wait(lock, [] { return !schedulerToElevator.empty() || stopThreads; });

        // Terminating if all requests have been processed:
        if (stopThreads = true && schedulerToElevator.empty()) {
            std::cout << "[Elevator] No more requests. Terminating..." << std::endl;
            return;
        }

        // Retrieve the request from the schedulerToElevator queue
        FloorRequest request = schedulerToElevator.front();
        schedulerToElevator.pop(); // Pop first element from "schedulerToElevator" queue
        lock.unlock();

        // Checking for invalid data stored in "FloorRequest" object:
        if (request.floor == 0 && request.destination == 0 && request.direction.empty()) {
            std::cerr << "[ERROR] Elevator received an invalid request!" << std::endl;
            continue;
        }

        std::cout << "[Elevator] Moving from Floor " << request.floor
                  << " to Destination " << request.destination
                  << " (Direction: " << request.direction << ")" << std::endl;

        // Simulating elevator movement for 2 seconds (this duration will change in upcoming iterations)
        std::this_thread::sleep_for(std::chrono::seconds(2));

        //  Notify Scheduler that the Elevator has arrived and completed the request
        {
            std::lock_guard<std::mutex> lock(mtxElevatorToScheduler);
            // Adding the completed request to the "elevatorToScheduler" shared queue for Scheduler to process it.
            elevatorToScheduler.push(request);
        }
        // Notifying Scheduler of adding completed request to the "elevatorToScheduler" shared queue
        cvElevatorToScheduler.notify_one();
    }
}
