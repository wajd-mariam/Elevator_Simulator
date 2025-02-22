#include <iostream>
#include <thread>
#include <chrono>
#include "elevator.h"
#include "scheduler.h"
#include "floor.h"
#include "globals.h"

#define CHECK(condition, message) \
    if (!(condition)) std::cerr << "[TEST FAILED] " << message << std::endl;

void test_elevator_initial_state() {
    std::cerr << "\n[TEST] Elevator Initial State" << std::endl;
    Elevator elevator;
    CHECK(elevator.getDoorsOpen() == true, "Elevator doors should start open.");
    CHECK(elevator.getCurrentState() == ElevatorState::WAIT_FOR_SCHEDULER, "Elevator should start in WAIT_FOR_SCHEDULER state.");
}

void test_elevator_state_transitions() {
    std::cerr << "\n[TEST] Elevator State Transitions" << std::endl;
    Elevator elevator;
    
    // Simulating state transition
    bool stateReached = false;
    for (int i = 0; i < 10; i++) { 
        if (elevator.getCurrentState() == ElevatorState::RECEIVE_INSTRUCTIONS) {
            stateReached = true;
            break;
        }
        //std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    CHECK(stateReached, "Elevator did not transition to RECEIVE_INSTRUCTIONS in time.");
}

void test_scheduler_initial_state() {
    std::cerr << "\n[TEST] Scheduler Initial State" << std::endl;
    Scheduler scheduler;
    CHECK(scheduler.getCurrentState() == SchedulerState::WAIT_FOR_REQUEST, "Scheduler should start in WAIT_FOR_REQUEST state.");
}

void test_system() {
    std::cerr << "\n[TEST] System Integration" << std::endl;
    Scheduler scheduler;
    Elevator elevator;
    Floor floorSystem(1, "input.txt");
    
    std::thread floorThread(&Floor::sendRequestsToScheduler, &floorSystem);
    std::thread schedulerThread(&Scheduler::processFloorRequests, &scheduler);
    std::thread elevatorThread(&Elevator::processRequests, &elevator);
    
    floorThread.join();
    schedulerThread.join();
    elevatorThread.join();
    
    std::cerr << "[TEST] System ran successfully." << std::endl;
}

int main() {
    std::cerr << "\nStarting Tests..." << std::endl;
    std::cerr << "===========================\n";

    test_elevator_initial_state();
    test_elevator_state_transitions();
    test_scheduler_initial_state();
    test_system();

    std::cerr << "\nAll tests completed!" << std::endl;
    std::cerr << "===========================\n";
    return 0;
}
