#ifndef FLOORREQUEST_H
#define FLOORREQUEST_H

#include <string>

struct FloorRequest {
    std::string timeStamp; // Time of request
    int floor;             // Source floor
    std::string direction; // "Up" or "Down"
    int destination;       // Target floor

    // Constructor
    FloorRequest(std::string t, int f, std::string d, int dest)
        : timeStamp(t), floor(f), direction(d), destination(dest) {}
};

#endif // FLOORREQUEST_H
