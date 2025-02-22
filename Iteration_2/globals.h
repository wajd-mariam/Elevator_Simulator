#ifndef GLOBALS_H
#define GLOBALS_H

#include <atomic>
#include <iostream>
#include <thread>
#include <vector>
#include <sstream>
#include <queue>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include "FloorRequest.h"

// Declare global variables (they will be defined in one .cpp file)
extern std::atomic<int> pendingRequests;
extern std::atomic<bool> stopThreads;

// Shared queues for inter-thread communication
extern std::queue<FloorRequest> schedulerToElevator;
extern std::mutex mtxSchedulerToElevator;
extern std::condition_variable cvSchedulerToElevator;

extern std::queue<FloorRequest> elevatorToScheduler;
extern std::mutex mtxElevatorToScheduler;
extern std::condition_variable cvElevatorToScheduler;

extern std::queue<FloorRequest> schedulerToFloor;
extern std::mutex mtxSchedulerToFloor;
extern std::condition_variable cvSchedulerToFloor;

extern std::queue<FloorRequest> floorToScheduler;
extern std::mutex mtxFloorToScheduler;
extern std::condition_variable cvFloorToScheduler;

#endif // GLOBALS_H