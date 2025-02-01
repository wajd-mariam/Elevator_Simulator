#ifndef FLOOR_H
#define FLOOR_H

#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <sstream>
#include <queue>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <random>
#include "FloorRequest.h"

// Shared queue for communication 
// Floor -> Scheduler
extern std::queue<FloorRequest> floorToScheduler;    
extern std::mutex mtxFloorToScheduler; 
extern std::condition_variable cvFloorToScheduler;

// Scheduler ->  Floor
extern std::queue<FloorRequest> schedulerToFloor;
extern std::mutex mtxSchedulerToFloor;
extern std::condition_variable cvSchedulerToFloor;

class Floor {
private:
    int floorNumber;
    std::vector<FloorRequest> requests;
    std::string filename;

public:
    // Constructor
    Floor(int n, const std::string& file);

    // Methods
    void readInputFile(const std::string& filename);
    FloorRequest parseRequest(const std::string& line);
    void printAllRequests(); // DEBUGGING
    void sendRequestsToScheduler();
    void listenForElevatorArrival();
};

#endif // FLOOR_H