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
// With floor & Scheduler
extern std::queue<FloorRequest> floorToScheduler;    // Floor -> Scheduler
extern std::mutex mtxFloorToScheduler; 
extern std::condition_variable cvFloorToScheduler;

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
};

#endif // FLOOR_H