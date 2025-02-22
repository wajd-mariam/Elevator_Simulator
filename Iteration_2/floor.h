#ifndef FLOOR_H
#define FLOOR_H

#include <string>
#include <vector>
#include "FloorRequest.h"


class Floor {
private:
    int floorNumber;
    // Vector array to store "FloorRequest" object
    std::vector<FloorRequest> requests;
    std::string filename; // "input.txt"

public:
    /**
     * @brief Constructs a Floor object and initializes its default data.
     * 
     * This constructor initializes a Floor object with a given floor number and input file name.
     *
     * @param n The floor number associated with this instance.
     * @param file The name of the input file containing floor requests.
     */
    Floor(int n, const std::string& file);

    /**
     * @brief Reads input data from 'filename' parameter and packages each line into a 'FloorRequest' object, and 
     * adds each one to "vector<FloorRequest> requests" vector array.
     *
     * @param std::string& The name of the input file.
     */
    void readInputFile(const std::string& filename);
    /**
     * @brief Stores a line of input into a FloorRequest object
     * 
     * This function takes a single line of input from the input file, extracts the
     * timestamp, floor number, direction, and destination, and returns a 
     * FloorRequest object containing this data.
     * 
     * @return A FloorRequest object containing all data of a floor request
     */
    FloorRequest parseRequest(const std::string& line);
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
    void sendRequestsToScheduler();
};

#endif // FLOOR_H