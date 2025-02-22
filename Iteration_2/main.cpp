#include <iostream>
#include <thread>
#include <string>
#include <atomic>
#include "floor.h"
#include "scheduler.h"
#include "elevator.h"
#include "globals.h"

// Tracking number of pending requests:
std::atomic<int> pendingRequests(0);
// Boolean variable used to terminate threads
std::atomic<bool> stopThreads(false);

int main() {
    std::string filename = "input.txt";

    // Create Scheduler, Floor, and Elevator objects:
    Floor floorSystem(1, filename);
    Scheduler scheduler;
    Elevator elevator;

    // Create Floor, Scheduler, and Elevator threads:
    std::thread floorThread(&Floor::sendRequestsToScheduler, &floorSystem);
    std::thread schedulerThread(&Scheduler::processFloorRequests, &scheduler);
    std::thread elevatorThread(&Elevator::processRequests, &elevator);

    // Ensure all threads terminate before exiting program
    floorThread.join();
    schedulerThread.join();
    elevatorThread.join();

    // This statement runs after processing all requests:
    std::cout << "Program terminated successfully." << std::endl;
    return 0;
}