#include <gtest/gtest.h>
#include <fstream>
#include <thread>
#include <chrono>
#include "scheduler.h"
#include "elevator.h"
#include "floor.h"
#include "globals.h"

static void clearGlobalQueues() {
    stopThreads = false;
    pendingRequests = 0;
    while (!floorToScheduler.empty())      floorToScheduler.pop();
    while (!schedulerToElevator.empty())   schedulerToElevator.pop();
    while (!elevatorToScheduler.empty())   elevatorToScheduler.pop();
    while (!schedulerToFloor.empty())      schedulerToFloor.pop();
}

bool openFileForTest(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "[Test] Error: Could not open input file: " << filename << std::endl;
        return false;
    }
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
    }
    file.close();
    return true;
}

// Elevator Tests

TEST(ElevatorTest, InitialState) {
    Elevator e;
    EXPECT_EQ(e.getCurrentState(), ElevatorState::WAIT_FOR_SCHEDULER);
    EXPECT_TRUE(e.getDoorsOpen());
    EXPECT_EQ(e.getCurrentFloor(), 1);
}

TEST(ElevatorTest, SingleRequestEndFloor) {
    clearGlobalQueues();
    FloorRequest req("10:00", 2, "Up", 5);
    {
        std::lock_guard<std::mutex> lk(mtxSchedulerToElevator);
        schedulerToElevator.push(req);
    }
    cvSchedulerToElevator.notify_all();

    Elevator elevator;
    std::thread elevThread(&Elevator::processRequests, &elevator);

    std::this_thread::sleep_for(std::chrono::milliseconds(2500));

    stopThreads = true;
    cvSchedulerToElevator.notify_all();
    elevThread.join();

    EXPECT_EQ(elevator.getCurrentFloor(), 5);
    EXPECT_EQ(elevatorToScheduler.size(), (size_t)1);
    clearGlobalQueues();
}

TEST(ElevatorTest, MultipleAndEdgeRequests) {
    clearGlobalQueues();
    FloorRequest req1("11:00", 3, "Up", 6);
    FloorRequest req2("11:05", 7, "Down", 2);
    FloorRequest req3("11:06", 5, "Up", 5);
    {
        std::lock_guard<std::mutex> lk(mtxSchedulerToElevator);
        schedulerToElevator.push(req1);
        schedulerToElevator.push(req2);
        schedulerToElevator.push(req3);
    }
    cvSchedulerToElevator.notify_all();

    Elevator e;
    std::thread t(&Elevator::processRequests, &e);

    std::this_thread::sleep_for(std::chrono::seconds(6));
    stopThreads = true;
    cvSchedulerToElevator.notify_all();
    t.join();

    EXPECT_EQ(e.getCurrentFloor(), 5);
    EXPECT_FALSE(elevatorToScheduler.empty());
    clearGlobalQueues();
}

// Scheduler Tests

TEST(SchedulerTest, InitialState) {
    Scheduler s;
    EXPECT_EQ(s.getCurrentState(), SchedulerState::WAIT_FOR_REQUEST);
}

TEST(SchedulerTest, MultipleRequests) {
    clearGlobalQueues();
    pendingRequests = 3;
    FloorRequest r1("08:00", 4, "Down", 1);
    FloorRequest r2("08:02", 2, "Up", 7);
    FloorRequest r3("08:04", 3, "Down", 1);
    {
        std::lock_guard<std::mutex> lk(mtxFloorToScheduler);
        floorToScheduler.push(r1);
        floorToScheduler.push(r2);
        floorToScheduler.push(r3);
    }
    cvFloorToScheduler.notify_all();

    Scheduler s;
    std::thread schedTh(&Scheduler::processFloorRequests, &s);

    std::thread fakeElevator([] {
        while (!stopThreads) {
            std::unique_lock<std::mutex> lk(mtxSchedulerToElevator);
            if (!schedulerToElevator.empty()) {
                FloorRequest req = schedulerToElevator.front();
                schedulerToElevator.pop();
                lk.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(400));
                {
                    std::lock_guard<std::mutex> lk2(mtxElevatorToScheduler);
                    elevatorToScheduler.push(req);
                }
                cvElevatorToScheduler.notify_all();
            } else {
                cvSchedulerToElevator.wait_for(lk, std::chrono::milliseconds(100));
            }
            if (stopThreads) break;
        }
    });

    std::this_thread::sleep_for(std::chrono::seconds(3));
    EXPECT_TRUE(stopThreads);
    schedTh.join();
    fakeElevator.join();
    EXPECT_EQ(schedulerToFloor.size(), (size_t)3);
    clearGlobalQueues();
}

TEST(SchedulerTest, InvalidRequest) {
    clearGlobalQueues();
    FloorRequest invalidReq("09:15", -1, "Up", 3);
    pendingRequests = 1;
    {
        std::lock_guard<std::mutex> lk(mtxFloorToScheduler);
        floorToScheduler.push(invalidReq);
    }
    cvFloorToScheduler.notify_all();

    Scheduler s;
    std::thread schedTh(&Scheduler::processFloorRequests, &s);

    std::thread fakeElev([] {
        while (!stopThreads) {
            std::unique_lock<std::mutex> lk(mtxSchedulerToElevator);
            if (!schedulerToElevator.empty()) {
                FloorRequest r = schedulerToElevator.front();
                schedulerToElevator.pop();
                lk.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                {
                    std::lock_guard<std::mutex> lk2(mtxElevatorToScheduler);
                    elevatorToScheduler.push(r);
                }
                cvElevatorToScheduler.notify_all();
            } else {
                cvSchedulerToElevator.wait_for(lk, std::chrono::milliseconds(100));
            }
            if (stopThreads) break;
        }
    });

    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_TRUE(stopThreads);
    schedTh.join();
    fakeElev.join();
    EXPECT_EQ(schedulerToFloor.size(), (size_t)1);
    clearGlobalQueues();
}

// Floor Tests

TEST(FloorTest, ParseRequest) {
    clearGlobalQueues();
    Floor f(1, "");
    FloorRequest req = f.parseRequest("14:05:15.0 2 Up 4");
    EXPECT_EQ(req.timeStamp, "14:05:15.0");
    EXPECT_EQ(req.floor, 2);
    EXPECT_EQ(req.direction, "Up");
    EXPECT_EQ(req.destination, 4);
}

TEST(FloorTest, ReadInputFile) {
    clearGlobalQueues();
    std::string testFile = "input.txt";
    std::ofstream testOutput(testFile);
    if (testOutput) {
        testOutput << "10:10 5 Down 1\n10:12 2 Up 3\n";
        testOutput.close();
    }
    Floor f(1, testFile);
    EXPECT_TRUE(openFileForTest(testFile));
}

TEST(FloorTest, ReadInputFile_NonExistent) {
    clearGlobalQueues();
    std::string missing = "non_existent_file.txt";
    Floor f(1, missing);
    EXPECT_FALSE(openFileForTest(missing));
}

// System / End-to-End Tests

TEST(SystemTest, FullFlowTwoRequests) {
    clearGlobalQueues();
    {
        std::lock_guard<std::mutex> lk(mtxFloorToScheduler);
        floorToScheduler.push(FloorRequest("10:10", 5, "Down", 1));
        floorToScheduler.push(FloorRequest("10:12", 2, "Up", 3));
    }
    pendingRequests = 2;

    Floor floorObj(1, "");
    Scheduler schedulerObj;
    Elevator elevatorObj;

    std::thread floorTh(&Floor::sendRequestsToScheduler, &floorObj);
    std::thread schedTh(&Scheduler::processFloorRequests, &schedulerObj);
    std::thread elevTh(&Elevator::processRequests, &elevatorObj);

    floorTh.join();
    schedTh.join();
    elevTh.join();

    EXPECT_TRUE(stopThreads);
    EXPECT_EQ(schedulerToFloor.size(), (size_t)0);
}

TEST(SystemTest, FullFlowNoMovement) {
    clearGlobalQueues();
    {
        std::lock_guard<std::mutex> lk(mtxFloorToScheduler);
        floorToScheduler.push(FloorRequest("10:20", 4, "Up", 4));
    }
    pendingRequests = 1;

    Floor floorObj(1, "");
    Scheduler schedObj;
    Elevator elevObj;

    std::thread floorTh(&Floor::sendRequestsToScheduler, &floorObj);
    std::thread schedTh(&Scheduler::processFloorRequests, &schedObj);
    std::thread elevTh(&Elevator::processRequests, &elevObj);

    floorTh.join();
    schedTh.join();
    elevTh.join();

    EXPECT_TRUE(stopThreads);
    EXPECT_EQ(elevObj.getCurrentFloor(), 4);
    clearGlobalQueues();
}

/**
 * New test that runs the full code with input.txt.
 * We'll create a few lines in "input.txt" and let Floor read from it normally.
 */
TEST(SystemTest, FullFlowFromFile) {
    clearGlobalQueues();
    std::string testFile = "input.txt";
    {
        std::ofstream testOutput(testFile);
        if (testOutput) {
            testOutput << "14:01:01.0 1 Up 3\n"
                       << "14:02:02.0 3 Down 1\n";
        }
    }

    // 2 lines => pendingRequests after readInputFile will be 2
    Floor floorObj(1, testFile);
    Scheduler schedulerObj;
    Elevator elevatorObj;

    std::thread floorTh(&Floor::sendRequestsToScheduler, &floorObj);
    std::thread schedTh(&Scheduler::processFloorRequests, &schedulerObj);
    std::thread elevTh(&Elevator::processRequests, &elevatorObj);

    floorTh.join();
    schedTh.join();
    elevTh.join();

    EXPECT_TRUE(stopThreads);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
