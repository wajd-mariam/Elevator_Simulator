#include "elevator.h"
#include "scheduler.h"
#include <atomic>

// Global counter of pending requests:
extern std::atomic<int> pendingRequests;
extern std::atomic<bool> stopThreads;

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
        schedulerToElevator.pop();
        lock.unlock();

        if (request.floor == 0 && request.destination == 0 && request.direction.empty()) {
            std::cerr << "[ERROR] Elevator received an invalid request!" << std::endl;
            continue;
        }

        std::cout << "[Elevator] Moving from Floor " << request.floor
                  << " to Destination " << request.destination
                  << " (Direction: " << request.direction << ")" << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(2));

        //  Notify Scheduler that the Elevator has arrived
        {
            std::lock_guard<std::mutex> lock(mtxElevatorToScheduler);
            elevatorToScheduler.push(request);
        }
        cvElevatorToScheduler.notify_one();
    }
}
