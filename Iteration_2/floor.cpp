#include "floor.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include "globals.h"

/**
 * @brief Constructs a Floor object and initializes its default data.
 * 
 * This constructor initializes a Floor object with a given floor number and input file name.
 *
 * @param n The floor number associated with this instance.
 * @param file The name of the input file containing floor requests.
 */
Floor::Floor(int n, const std::string& file) : floorNumber(n), filename(file) {
    this->readInputFile(filename); // Read requests during initialization
}


/**
 * @brief Reads input data from 'filename' parameter and packages each line into a 'FloorRequest' object, and 
 * adds each one to "vector<FloorRequest> requests" vector array.
 *
 * @param std::string& The name of the input file.
 */
void Floor::readInputFile(const std::string& filename) {
    // Opening a file
    std::ifstream file(filename);

    // If file does not exist -> ERROR
    if (!file) {
        std::cerr << "[Floor] Error: Could not open input file!" << std::endl;
        return;
    }

    std::string line;
    // Reading file line by line and storing input in "FloorRequest" struct
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        FloorRequest request = parseRequest(line);
        this->requests.push_back(request);
        pendingRequests++; // Incrementing requests
    }
    file.close();
}


/**
 * @brief Stores a line of input into a FloorRequest object
 * 
 * This function takes a single line of input from the input file, extracts the
 * timestamp, floor number, direction, and destination, and returns a 
 * FloorRequest object containing this data.
 * 
 * @return A FloorRequest object containing all data of a floor request
 */
FloorRequest Floor::parseRequest(const std::string& line) {
    std::istringstream iss(line);
    std::string timeStamp, direction;
    int floorNum, destination;

    iss >> timeStamp >> floorNum >> direction >> destination;
    return FloorRequest(timeStamp, floorNum, direction, destination);
}


/**
 * @brief Sends Floor request to Scheduler for processing
 * 
 * This method adds 'FloorRequest' objects (stored in 'vector<FloorRequest> requests') to "floorToScheduler" shared queue
 * and sends them one by one to the Scheduler subsystem. After sending a request, it notifies the Scheduler and 
 * waits for an aknowledgement before sending the next object.
 * 
 * Workflow:
 * 1. Lock the `floorToScheduler` queue and push the request.
 * 2. Notify the Scheduler that a new request is available.
 * 3. Wait for an acknowledgment from the Scheduler (`cvSchedulerToFloor`).
 * 4. Once acknowledged, remove the request from the `schedulerToFloor` queue.
 * 5. Print confirmation before proceeding to the next request.
 */
void Floor::sendRequestsToScheduler() {
    for (const FloorRequest &req : requests) {
        std::cout << "[Floor] Sending request to Scheduler: "
                  << req.timeStamp << " Floor " << req.floor << " "
                  << req.direction << " -> " << req.destination << std::endl;

        // 1) Push to floorToScheduler
        {
            std::lock_guard<std::mutex> lk(mtxFloorToScheduler);
            floorToScheduler.push(req);
        }
        // 2) Notify scheduler that we have a new request
        cvFloorToScheduler.notify_all();

        // 3) Wait for ack from Scheduler->Floor
        {
            std::unique_lock<std::mutex> lk(mtxSchedulerToFloor);
            cvSchedulerToFloor.wait(lk, [] {
                return !schedulerToFloor.empty() || stopThreads;
            });
            if (stopThreads && schedulerToFloor.empty()) {
                break;
            }
            FloorRequest doneReq = schedulerToFloor.front();
            schedulerToFloor.pop();
            std::cout << "[Floor] Acknowledgment received for request: Floor "
                      << doneReq.floor << " -> " << doneReq.destination << "\n\n";
        }
    }

    // Done with requests
    std::cout << "[Floor] Done sending all requests. Setting stopThreads = true.\n";
    stopThreads = true;

    // Unblock any waiting threads
    cvFloorToScheduler.notify_all();
    cvSchedulerToElevator.notify_all();
    cvElevatorToScheduler.notify_all();
    cvSchedulerToFloor.notify_all();
}
