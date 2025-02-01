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

struct FloorRequest {
    std::string timeStamp; // Time of request
    int floor;
    std::string direction; // "Up" or "Down"
    int destination; // Target floor
    int elevatorNum; // Assigned elevator number

    // Constructor
    FloorRequest(std::string t, int f, std::string d, int dest, int e)
        : timeStamp(t), floor(f), direction(d), destination(dest), elevatorNum(e) {}
};

// Shared queue for communication with scheduler
extern std::queue<FloorRequest> schedulerQueue;
extern std::mutex mtx; 
extern std::condition_variable cv;

class Floor {
private:
    int floorNumber;
    std::vector<FloorRequest> requests;
    std::string filename;

public:
    Floor(int n, const std::string& file);
    void readInputFile(const std::string& filename);
    FloorRequest parseRequest(const std::string& line);
    void printAllRequests(); // DEBUGGING
    void sendRequestsToScheduler();
};

#endif // FLOOR_H