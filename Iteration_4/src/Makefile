CXX       = g++
CXXFLAGS  = -std=c++17 -Wall -Wextra -pthread
GTEST_LIB = -lgtest -lgtest_main -pthread

SRC_FLOOR      = FloorProcess.cpp
SRC_SCHEDULER  = SchedulerProcess.cpp
SRC_ELEVATOR   = ElevatorProcess.cpp
SRC_COMMON     = Common.cpp
SRC_TESTS      = iteration4_tests.cpp

OBJ_FLOOR      = FloorProcess.o
OBJ_SCHEDULER  = SchedulerProcess.o
OBJ_ELEVATOR   = ElevatorProcess.o
OBJ_COMMON     = Common.o
OBJ_TESTS      = iteration4_tests.o

all: FloorProcess SchedulerProcess ElevatorProcess iteration4_tests

FloorProcess: $(OBJ_FLOOR) $(OBJ_COMMON)
	$(CXX) $(CXXFLAGS) -o FloorProcess $(OBJ_FLOOR) $(OBJ_COMMON)

SchedulerProcess: $(OBJ_SCHEDULER) $(OBJ_COMMON)
	$(CXX) $(CXXFLAGS) -o SchedulerProcess $(OBJ_SCHEDULER) $(OBJ_COMMON)

ElevatorProcess: $(OBJ_ELEVATOR) $(OBJ_COMMON)
	$(CXX) $(CXXFLAGS) -o ElevatorProcess $(OBJ_ELEVATOR) $(OBJ_COMMON)

iteration4_tests: $(OBJ_TESTS) $(OBJ_COMMON)
	$(CXX) $(CXXFLAGS) -o iteration4_tests $(OBJ_TESTS) $(OBJ_COMMON) $(GTEST_LIB)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

tests4: iteration4_tests
	./iteration4_tests

clean:
	rm -f *.o FloorProcess SchedulerProcess ElevatorProcess iteration4_tests
