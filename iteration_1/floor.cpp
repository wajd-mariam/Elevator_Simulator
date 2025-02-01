#include "floor.h"


std::queue<FloorRequest> schedulerQueue;
std::mutex mtx; 
std::condition_variable cv;

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
    int floorNum, destination, elevatorNum;

    iss >> timeStamp >> floorNum >> direction >> destination >> elevatorNum;

    return FloorRequest(timeStamp, floorNum, direction, destination, elevatorNum);
}

void Floor::printAllRequests() {
    std::cout << "[DEBUG] Printing all stored requests:" << std::endl;
    for (const FloorRequest& req : requests) {
        std::cout << "Time: " << req.timeStamp
                << ", Floor: " << req.floor
                << ", Direction: " << req.direction
                << ", Destination: " << req.destination
                << ", Elevator: " << req.elevatorNum << std::endl;
    }
}