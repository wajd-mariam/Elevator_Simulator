#ifndef FLOORREQUEST_H
#define FLOORREQUEST_H

#include <string>

/**
 * @brief Represents a floor request made by a user.
 * 
 * This struct stores information about a request for an elevator, including:
 * 1. The time the request was made.
 * 2. The source floor where the request was made.
 * 3. The direction of travel ("Up" or "Down").
 * 4. The destination floor where the user wants to go.
 */
struct FloorRequest {
    std::string timeStamp; // Time of request
    int floor;             // Source floor
    std::string direction; // "Up" or "Down"
    int destination;       // Target floor

    /**
     * @brief Constructs a FloorRequest object with given parameters.
     * 
     * @param t Time of request (as a string).
     * @param f Source floor number.
     * @param d Direction of travel ("Up" or "Down").
     * @param dest Destination floor number.
     */
    FloorRequest(std::string t, int f, std::string d, int dest)
        : timeStamp(t), floor(f), direction(d), destination(dest) {}
};

#endif // FLOORREQUEST_H
