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
std::queue<FloorRequest> schedulerQueue;
std::mutex mtx; 
std::condition_variable cv; 

class Floor {
private:
    int floorNumber;
    std::vector<FloorRequest> requests; // List of requests
    std::string filename;

public:
    // Constructor
    Floor(int n, const std::string& file) : floorNumber(n), filename(file) {
        readInputFile("input.txt"); // Read requests during initialization
    }

    // Read input data and store requests in a 'FloorRequest' struct:
    void readInputFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file) {
            std::cerr << "Error: Could not open input file!" << std::endl;
            return;
        }
    

        std::string line;
        while (std::getline(file, line)) {
             // DEBUG: Print each line being read from the file
            std::cout << "[DEBUG] Reading line: " << line << std::endl;
            FloorRequest req = parseRequest(line);
            requests.push_back(req);

            // DEBUG: Print parsed request details
            std::cout << "[DEBUG] Parsed Request -> Time: " << req.timeStamp
                  << ", Floor: " << req.floor
                  << ", Direction: " << req.direction
                  << ", Destination: " << req.destination;
        }
        file.close();
    }
    
    // Parses a request line into a FloorRequest object
    FloorRequest parseRequest(const std::string& line) {
        std::istringstream iss(line);
        std::string timeStamp, direction;
        int floorNum, destination, elevatorNum;

        iss >> timeStamp >> floorNum >> direction >> destination >> elevatorNum;

        return FloorRequest(timeStamp, floorNum, direction, destination, elevatorNum);
    }

    void printAllRequests() {
    std::cout << "[DEBUG] Printing all stored requests:" << std::endl;
    for (const FloorRequest& req : requests) {
        std::cout << "Time: " << req.timeStamp
                  << ", Floor: " << req.floor
                  << ", Direction: " << req.direction
                  << ", Destination: " << req.destination
                  << ", Elevator: " << req.elevatorNum << std::endl;
    }
}
};


int main() {
    std::string filename = "input.txt";

    //  Start Floor Subsystem and Scheduler threads
    Floor floorSystem(1, filename);
    floorSystem.printAllRequests();
    //std::thread floorThread(std::ref(floorSystem));
    //std::thread schedulerThreadInstance(schedulerThread);

    // Keep threads running
    //floorThread.join();
    //schedulerThreadInstance.join();

    return 0;
}