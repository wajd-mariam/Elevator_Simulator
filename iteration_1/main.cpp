#include <string>
#include "floor.h"
#include "scheduler.h"
#include "elevator.h"

int main() {
    std::string filename = "input.txt";

    // Create Scheduler and Floor subsystems:
    Scheduler scheduler;
    Floor floorSystem(1, filename);
    floorSystem.printAllRequests();

    // Create threads for Floor and Scheduler:
    std::thread floorThread(&Floor::sendRequestsToScheduler, &floorSystem);
    std::thread schedulerThread(&Scheduler::processFloorRequests, &scheduler);
    //std::thread schedulerThreadInstance(schedulerThread);

    // Keep threads running
    floorThread.join();
    schedulerThread.join();


    class OperateElevator shaft;

    thread elevator1(elevatorThread, ref(shaft));
   // thread scheduler(schedulerThread, control);
   // thread floors(floorThread, shaft);
    
    elevator1.join();
    //scheduler.join();
    //floors.join();*/

    return 0;
}