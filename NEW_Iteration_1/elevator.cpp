// What does the elevator need to do?
// needs to go up and down floors
// needs to have buttons that tell the system which floors to go to
// needs a door and motor to let people in/get people out (static variable)
#include "elevator.h"
#include "scheduler.h"


void Elevator::processRequests() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtxSchedulerToElevator);
        // Wait for a request from the Scheduler Subsystem
        cvSchedulerToElevator.wait(lock, [] { return !schedulerToElevator.empty(); });

        // Retrieve the request from the queue
        FloorRequest request = schedulerToElevator.front();
        schedulerToElevator.pop();
        lock.unlock();

        // Debug: Print received request
        std::cout << "[Elevator] Received request: Move from Floor " << request.floor
                  << " to Destination " << request.destination
                  << " (Direction: " << request.direction << ")" << std::endl;

        // Ensure valid request
        if (request.floor == 0 && request.destination == 0 && request.direction.empty()) {
            std::cerr << "[ERROR] Elevator received an invalid request!" << std::endl;
            continue;
        }

        // Print retrieved request:
        std::cout << "[Elevator] Received request: Move from Floor " << request.floor
                  << " to Destination " << request.destination << " (Direction: "
                  << request.direction << ")" << std::endl;

        // Simulate elevator movement:
        std::cout << "[Elevator] Moving to Floor " << request.destination << "..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));  // Simulate travel time - 2 Seconds

        //  Notify Scheduler that the Elevator has arrived
        {
            std::lock_guard<std::mutex> lock(mtxElevatorToScheduler);
            elevatorToScheduler.push(request);
        }
        cvElevatorToScheduler.notify_one();

        std::cout << "[Elevator] Reached destination: " << request.destination << std::endl;
        
    }
}
