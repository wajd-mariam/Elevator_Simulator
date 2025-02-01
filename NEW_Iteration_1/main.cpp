#include <string>
#include "floor.h"
#include "scheduler.h"
#include "elevator.h"

int main() {
    std::string filename = "input.txt";

    // Create Scheduler and Floor subsystems:
    Floor floorSystem(1, filename);
    Scheduler scheduler;
    Elevator elevator;
    floorSystem.printAllRequests();

    // Create threads for Floor and Scheduler:
    std::thread floorThread(&Floor::sendRequestsToScheduler, &floorSystem);
    std::thread schedulerThread(&Scheduler::processFloorRequests, &scheduler);
    std::thread elevatorThread(&Elevator::processRequests, &elevator);

    // Keep threads running
    floorThread.join();
    schedulerThread.join();
    elevatorThread.join();

    return 0;
}