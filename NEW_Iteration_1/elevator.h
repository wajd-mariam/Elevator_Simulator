#ifndef ELEVATOR_H
#define ELEVATOR_H

#include "floor.h"
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <random>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <list>
#include <deque>
#include "FloorRequest.h"


extern std::queue<FloorRequest> schedulerToElevator;
extern std::mutex mtxSchedulerToElevator;
extern std::condition_variable cvSchedulerToElevator;

extern std::queue<FloorRequest> elevatorToScheduler;
extern std::mutex mtxElevatorToScheduler;
extern std::condition_variable cvElevatorToScheduler;

class Elevator {
private:
    bool elevatorMoving; //says when elevator
                        //indicates when a new task can be inputted
                        //can be inputted when not moving essentially (will be patched late4r)
    bool doorsOpen; // for later use
    int floorAt; // indicates the floor number the elevator is located on
                // used to tell if its going in the proper direction 
    int floorGoingTo; // loaded in by scheduler

public:
    //Elevator(): elevatorMoving(false), doorsOpen(false), floorAt(0), floorGoingTo(0), mtx(), waitE() {}
    void processRequests();
};

#endif // ELEVATOR_H