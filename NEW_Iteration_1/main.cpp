#include <iostream>
#include <thread>
#include <string>
#include <atomic>
#include "floor.h"
#include "scheduler.h"
#include "elevator.h"

// Tracking number of pending requests:
std::atomic<int> pendingRequests(0);
std::atomic<bool> stopThreads(false);

int main() {
    std::string filename = "input.txt";

    // Create Scheduler, Floor, and Elevator objects:
    Floor floorSystem(1, filename);
    Scheduler scheduler;
    Elevator elevator;
    floorSystem.printAllRequests();

    // Create Scheduler, Floor, and Elevator threads:
    std::thread floorThread(&Floor::sendRequestsToScheduler, &floorSystem);
    std::thread schedulerThread(&Scheduler::processFloorRequests, &scheduler);
    std::thread elevatorThread(&Elevator::processRequests, &elevator);

    // Keep threads running
    floorThread.join();
    schedulerThread.join();
    elevatorThread.join();

    std::cout << "Program terminated successfully." << std::endl;
    return 0;
}