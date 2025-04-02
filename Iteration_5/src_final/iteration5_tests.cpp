#include <gtest/gtest.h>
#include "Common.h"

struct ElevatorSim {
    int currentFloor;
    bool hardFault;
    bool doorStuckFixedOnce;

    ElevatorSim() : currentFloor(1), hardFault(false), doorStuckFixedOnce(false) {}
};

static const int ELEVATOR_CAPACITY = 4;
static const int TIMEOUT_THRESHOLD = 15;
static const int moveSpeedMs       = 500;
static const int timerMarginMs     = 2000;

void simulateElevatorRequest(ElevatorSim &e, const FloorRequest &req)
{
    if(e.hardFault) {
        return;
    }

    // capacity check
    if(req.passengers > ELEVATOR_CAPACITY) {
        // "Full" => do not move, no fault
        return;
    }

    // immediate stuckElevator check
    if(req.hasFault && req.faultType=="stuckElevator") {
        // Hard fault
        e.hardFault = true;
        return;
    }

    // doorStuck check
    if(req.hasFault && req.faultType=="doorStuck") {
        if(!e.doorStuckFixedOnce) {
            // First time => fix it => no permanent fault
            e.doorStuckFixedOnce = true;
        } else {
            // Second time => permanent => hard fault
            e.hardFault = true;
            return;
        }
    }

    int distanceToPickup = std::abs(e.currentFloor - req.floor);
    int distanceToDest   = std::abs(req.floor - req.destination);
    int totalFloors      = distanceToPickup + distanceToDest;

    if(totalFloors >= TIMEOUT_THRESHOLD) {
        e.hardFault = true;
        return;
    }

    e.currentFloor = req.destination;
}

TEST(Iteration4_Simulation, DoorStuckTransient)
{
    ElevatorSim e;

    FloorRequest req("xx", 3, "Up", 5, true, "doorStuck", 2);

    simulateElevatorRequest(e, req);

    EXPECT_FALSE(e.hardFault) << "First doorStuck should be transient => no permanent fault";
    EXPECT_EQ(e.currentFloor, 5);
    EXPECT_TRUE(e.doorStuckFixedOnce);
}

TEST(Iteration4_Simulation, DoorStuckPermanentSecondTime)
{
    ElevatorSim e;

    FloorRequest r1("xx", 2, "Down", 1, true, "doorStuck", 2);
    simulateElevatorRequest(e, r1);
    EXPECT_FALSE(e.hardFault);
    EXPECT_EQ(e.currentFloor, 1);
    EXPECT_TRUE(e.doorStuckFixedOnce);

    FloorRequest r2("xx", 3, "Up", 6, true, "doorStuck", 2);
    simulateElevatorRequest(e, r2);

    EXPECT_TRUE(e.hardFault)
        << "Second doorStuck => permanent => ElevatorSim hardFault should be true";
    EXPECT_EQ(e.currentFloor, 1);
}

TEST(Iteration4_Simulation, StuckElevatorImmediateFault)
{
    ElevatorSim e;

    // stuckElevator => immediate hard fault
    FloorRequest req("xx", 5, "Up", 15, true, "stuckElevator", 2);
    simulateElevatorRequest(e, req);

    EXPECT_TRUE(e.hardFault) << "stuckElevator => immediate fault";
    EXPECT_EQ(e.currentFloor, 1) 
        << "No movement because faulted immediately";
}

TEST(Iteration4_Simulation, TimerFaultLongDistance)
{
    ElevatorSim e;
    FloorRequest req("xx", 2, "Up", 20, false, "noFault", 3);

    simulateElevatorRequest(e, req);

    EXPECT_TRUE(e.hardFault) << "totalFloors=19 => Timer => Hard fault";
    EXPECT_EQ(e.currentFloor, 1) 
        << "No final movement since faulted mid-trip";
}

TEST(Iteration4_Simulation, NoFaultShortTrip)
{
    ElevatorSim e; // floor=1
    FloorRequest req("xx", 2, "Up", 3, false, "noFault", 2);
    simulateElevatorRequest(e, req);

    // No fault
    EXPECT_FALSE(e.hardFault);
    // Moved from 1->2->3 => final floor=3
    EXPECT_EQ(e.currentFloor, 3);
    // Not doorStuck
    EXPECT_FALSE(e.doorStuckFixedOnce);
}

TEST(Iteration4_Simulation, OverCapacity)
{
    ElevatorSim e;
    // 5 passengers => capacity=4 => "Full" => no movement => no fault
    FloorRequest req("xx", 1, "Up", 10, false, "noFault", 5);
    simulateElevatorRequest(e, req);

    EXPECT_FALSE(e.hardFault);
    // Did not move => floor remains 1
    EXPECT_EQ(e.currentFloor, 1);
}

int main(int argc, char** argv){
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
