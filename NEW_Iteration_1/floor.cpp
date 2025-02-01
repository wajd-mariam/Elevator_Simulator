#include "floor.h"
#include "scheduler.h"
#include <atomic>

// Global counter of pending requests:
extern std::atomic<int> pendingRequests;

std::queue<FloorRequest> floorToScheduler;
std::mutex mtxFloorToScheduler; 
std::condition_variable cvFloorToScheduler;

// Constructor
Floor::Floor(int n, const std::string& file) : floorNumber(n), filename(file) {
    this->readInputFile("input.txt"); // Read requests during initialization
}


// Read input data and store requests in a 'FloorRequest' struct:
void Floor::readInputFile(const std::string& filename) {
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
        this->requests.push_back(req);
        
        pendingRequests++;
        std::cout << "DEBUG: pending request value:" << pendingRequests << std::endl;

        // DEBUG: Print parsed request details
        std::cout << "[DEBUG] Parsed Request -> Time: " << req.timeStamp
                  << ", Floor: " << req.floor
                  << ", Direction: " << req.direction
                  << ", Destination: " << req.destination;
    }
    file.close();
}


// Parses a request line into a FloorRequest object
FloorRequest Floor::parseRequest(const std::string& line) {
    std::istringstream iss(line);
    std::string timeStamp, direction;
    int floorNum, destination;

    iss >> timeStamp >> floorNum >> direction >> destination;
    return FloorRequest(timeStamp, floorNum, direction, destination);
}

void Floor::printAllRequests() {
    std::cout << "[DEBUG] Printing all stored requests:" << std::endl;
    for (const FloorRequest& req : requests) {
        std::cout << "Time: " << req.timeStamp
                << ", Floor: " << req.floor
                << ", Direction: " << req.direction
                << ", Destination: " << req.destination << std::endl;
    }
}

void Floor::sendRequestsToScheduler() {
    for (const FloorRequest& req : this->requests) {
        std::cout << "[Floor] Sending request to Scheduler:" << std::endl;
        {
            std::lock_guard<std::mutex> lock(mtxFloorToScheduler);
            floorToScheduler.push(req);
        }
        cvFloorToScheduler.notify_one(); // Notifying Scheduler thread

        // Wait for aknowledgment from Scheduler before sending the next request
        std::unique_lock<std::mutex> lock(mtxSchedulerToFloor);
        cvSchedulerToFloor.wait(lock, [] { return !schedulerToFloor.empty(); });

        FloorRequest processedRequest = schedulerToFloor.front();
        schedulerToFloor.pop();
        std::cout << "[Floor] Acknowledgment received for request: Floor " << processedRequest.floor
                  << " -> Destination " << processedRequest.destination << std::endl;
    }
}