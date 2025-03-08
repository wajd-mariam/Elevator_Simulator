CXX       = g++
CXXFLAGS  = -std=c++17 -Wall -Wextra -pthread
GTEST_LIB = -lgtest -lgtest_main -pthread

SRC_FLOOR     = FloorProcess.cpp
SRC_SCHEDULER = SchedulerProcess.cpp
SRC_ELEVATOR  = ElevatorProcess.cpp
SRC_TESTS     = iteration3_tests.cpp
# If you have a separate Common.cpp, list it here, e.g.:
# SRC_COMMON    = Common.cpp
# OBJ_COMMON    = Common.o

OBJ_FLOOR     = FloorProcess.o
OBJ_SCHEDULER = SchedulerProcess.o
OBJ_ELEVATOR  = ElevatorProcess.o
OBJ_TESTS     = iteration3_tests.o
# OBJ_COMMON    = Common.o

all: FloorProcess SchedulerProcess ElevatorProcess iteration3_tests

FloorProcess: $(OBJ_FLOOR)
	$(CXX) $(CXXFLAGS) -o FloorProcess $(OBJ_FLOOR)

SchedulerProcess: $(OBJ_SCHEDULER)
	$(CXX) $(CXXFLAGS) -o SchedulerProcess $(OBJ_SCHEDULER)

ElevatorProcess: $(OBJ_ELEVATOR)
	$(CXX) $(CXXFLAGS) -o ElevatorProcess $(OBJ_ELEVATOR)

iteration3_tests: $(OBJ_TESTS)
	$(CXX) $(CXXFLAGS) -o iteration3_tests $(OBJ_TESTS) $(GTEST_LIB)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

tests: iteration3_tests
	./iteration3_tests

clean:
	rm -f *.o FloorProcess SchedulerProcess ElevatorProcess iteration3_tests
