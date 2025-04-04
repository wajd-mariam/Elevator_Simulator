CXX       = g++
CXXFLAGS  = -std=c++17 -Wall -Wextra -pthread
GTEST_LIB = -lgtest -lgtest_main -pthread -lncurses

SRC_FLOOR      = FloorProcess.cpp
SRC_SCHEDULER  = SchedulerProcess.cpp
SRC_ELEVATOR   = ElevatorProcess.cpp
SRC_COMMON     = Common.cpp
SRC_TESTS      = iteration5_tests.cpp
SRC_UI         = UIProcess.cpp

OBJ_FLOOR      = FloorProcess.o
OBJ_SCHEDULER  = SchedulerProcess.o
OBJ_ELEVATOR   = ElevatorProcess.o
OBJ_COMMON     = Common.o
OBJ_TESTS      = iteration5_tests.o
OBJ_UI         = UIProcess.o

all: FloorProcess SchedulerProcess ElevatorProcess ElevatorUI  iteration5_tests

FloorProcess: $(OBJ_FLOOR) $(OBJ_COMMON)
	$(CXX) $(CXXFLAGS) -o FloorProcess $(OBJ_FLOOR) $(OBJ_COMMON)

SchedulerProcess: $(OBJ_SCHEDULER) $(OBJ_COMMON)
	$(CXX) $(CXXFLAGS) -o SchedulerProcess $(OBJ_SCHEDULER) $(OBJ_COMMON)

ElevatorProcess: $(OBJ_ELEVATOR) $(OBJ_COMMON)
	$(CXX) $(CXXFLAGS) -o ElevatorProcess $(OBJ_ELEVATOR) $(OBJ_COMMON)

iteration5_tests: $(OBJ_TESTS) $(OBJ_COMMON)
	$(CXX) $(CXXFLAGS) -o iteration5_tests $(OBJ_TESTS) $(OBJ_COMMON) $(GTEST_LIB)

ElevatorUI: $(OBJ_UI) $(OBJ_COMMON)
	$(CXX) $(CXXFLAGS) -o ElevatorUI $(OBJ_UI) $(OBJ_COMMON) -lncurses

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

tests5: iteration5_tests
	./iteration5_tests

clean:
	rm -f *.o FloorProcess SchedulerProcess ElevatorProcess ElevatorUI iteration5_tests
