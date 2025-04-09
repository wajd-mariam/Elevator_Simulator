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

// Simulates how an elevator would handle a given floor request,
// including movement, capacity checks, and fault detection.
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

// Verifies that a single doorStuck fault is handled as transient (not a hard fault)
TEST(Iteration5_Simulation, DoorStuckTransient)
{
    ElevatorSim e;
    FloorRequest req("xx", 3, "Up", 5, true, "doorStuck", 2);
    simulateElevatorRequest(e, req);
    EXPECT_FALSE(e.hardFault) << "First doorStuck should be transient => no permanent fault";
    EXPECT_EQ(e.currentFloor, 5);
    EXPECT_TRUE(e.doorStuckFixedOnce);
}

// Verifies that encountering doorStuck twice results in a hard fault
TEST(Iteration5_Simulation, DoorStuckPermanentSecondTime)
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

// Verifies that a stuckElevator fault causes immediate hard fault
TEST(Iteration5_Simulation, StuckElevatorImmediateFault)
{
    ElevatorSim e;

    // stuckElevator => immediate hard fault
    FloorRequest req("xx", 5, "Up", 15, true, "stuckElevator", 2);
    simulateElevatorRequest(e, req);

    EXPECT_TRUE(e.hardFault) << "stuckElevator => immediate fault";
    EXPECT_EQ(e.currentFloor, 1) 
        << "No movement because faulted immediately";
}

// Verifies that a long trip exceeding TIMEOUT threshold triggers fault
TEST(Iteration5_Simulation, TimerFaultLongDistance)
{
    ElevatorSim e;
    FloorRequest req("xx", 2, "Up", 20, false, "noFault", 3);

    simulateElevatorRequest(e, req);

    EXPECT_TRUE(e.hardFault) << "totalFloors=19 => Timer => Hard fault";
    EXPECT_EQ(e.currentFloor, 1) 
        << "No final movement since faulted mid-trip";
}

// Verifies that a valid short request without fault works properly
TEST(Iteration5_Simulation, NoFaultShortTrip)
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

// Verifies that requests exceeding elevator capacity are ignored (no movement or fault)
TEST(Iteration5_Simulation, OverCapacity)
{
    ElevatorSim e;
    // 5 passengers => capacity=4 => "Full" => no movement => no fault
    FloorRequest req("xx", 1, "Up", 10, false, "noFault", 5);
    simulateElevatorRequest(e, req);

    EXPECT_FALSE(e.hardFault);
    // Did not move => floor remains 1
    EXPECT_EQ(e.currentFloor, 1);
}


// Test: Request with zero passengers
TEST(Iteration5_Simulation, ZeroPassengers) {
    ElevatorSim e;
    FloorRequest req("xx", 2, "Down", 1, false, "noFault", 0);
    simulateElevatorRequest(e, req);
    EXPECT_EQ(e.currentFloor, 1);
    EXPECT_FALSE(e.hardFault);
}


// UI Test: Verifies that a valid STATUS message is correctly parsed into an ElevatorStatus object
TEST(Iteration5_Display, DeserializeElevatorStatusSimple) {
    std::string msg = "STATUS|ElevID=0|Floor=5|Fault=0|Direction=UP|State=MOVING|FaultType=noFault";
    ElevatorStatus es = deserializeElevatorStatus(msg);
    EXPECT_EQ(es.id, 0);
    EXPECT_EQ(es.currentFloor, 5);
    EXPECT_TRUE(es.doorsOpen);
    EXPECT_FALSE(es.isFaulted);
    EXPECT_EQ(es.state, "MOVING");
    EXPECT_EQ(es.direction, "UP");
    EXPECT_EQ(es.faultType, "noFault");
}

// UI Test: Verifies that out-of-order fields are parsed correctly
TEST(Iteration5_Display, DeserializeElevatorStatusOutOfOrderFields)
{
    std::string msg = "STATUS|Direction=UP|State=MOVING|Floor=9|ElevID=2|Fault=0|FaultType=doorStuck";
    ElevatorStatus es = deserializeElevatorStatus(msg);

    EXPECT_EQ(es.id, 2);
    EXPECT_EQ(es.currentFloor, 9);
    EXPECT_FALSE(es.isFaulted);
    EXPECT_EQ(es.state, "MOVING");
    EXPECT_EQ(es.direction, "UP");
    EXPECT_EQ(es.faultType, "doorStuck");
}

// UI Test: Verifies partial messages are handled gracefully (missing some fields)
TEST(Iteration5_Display, DeserializeElevatorStatusPartialMessage)
{
    std::string msg = "STATUS|ElevID=4|Floor=8|Fault=1";
    ElevatorStatus es = deserializeElevatorStatus(msg);

    EXPECT_EQ(es.id, 4);
    EXPECT_EQ(es.currentFloor, 8);
    EXPECT_TRUE(es.isFaulted);
    EXPECT_EQ(es.state, "");  // not included
    EXPECT_EQ(es.direction, "");  // not included
    EXPECT_EQ(es.faultType, "noFault");  // not included
}

// UI Test: Test to ensure correct deserialization of a moving elevator going DOWN with a doorStuck fault (soft fault). 
TEST(Iteration5_Display, DeserializeElevatorStatus_DoorStuckMovingDown)
{
    std::string input = "STATUS|ElevID=1|Floor=4|Fault=1|Direction=DOWN|State=MOVING|FaultType=doorStuck";
    ElevatorStatus es = deserializeElevatorStatus(input);

    EXPECT_EQ(es.id, 1);
    EXPECT_EQ(es.currentFloor, 4);
    EXPECT_TRUE(es.isFaulted);
    EXPECT_EQ(es.direction, "DOWN");
    EXPECT_EQ(es.state, "MOVING");
    EXPECT_EQ(es.faultType, "doorStuck");
}


int main(int argc, char** argv){
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
