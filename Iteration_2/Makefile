# Simple Makefile for iteration 2

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread
GTEST_LIBS = -lgtest -lgtest_main -pthread

# Source files
SRCS = main.cpp globals.cpp floor.cpp scheduler.cpp elevator.cpp
OBJS = $(SRCS:.cpp=.o)

TEST_SRCS = iteration2_tests.cpp globals.cpp floor.cpp scheduler.cpp elevator.cpp
TEST_OBJS = $(TEST_SRCS:.cpp=.o)

all: ElevatorSystem runTests

ElevatorSystem: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

runTests: $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(TEST_OBJS) $(GTEST_LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f *.o ElevatorSystem runTests

test: runTests
	./runTests

