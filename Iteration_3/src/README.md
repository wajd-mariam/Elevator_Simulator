# Elevator Control System - Iteration 3 - SYSC 3303 - B2 - Group 1 - Winter 2025

Authors:
Ahmed Ali 101181126,
Azan Huggins Goolamallee 101260082,
Albert Robu 101275700,
Wajd Mariam 101225633,
Samuel Nieuwenhuis 101225633

## Overview

This project is meant to create and begin testing a program specifically for elevator simulation. This is done by using 3 subsystems, the floor that people press the button to request an elevator, the elevator itself, and the scheduler which is used to mediate between the two

Date: March 8th, 2025

## Setup:
1) Ensure having C++11 compiler and pthread library installed
2) sudo apt install make libgtest-dev -y

## Usage
1) Extract SYSC_3303_B2_G1_Iteration3.zip file
2) Navigate to the "src" directory
3) Open a terminal and enter:
- "make clean"
- "make"
4) Open a 4 other new terminal windows and copy and execute the commands below in order:
- ./SchedulerProcess 4001 4000 3
- ./ElevatorProcess 5000 0 127.0.0.1 4001
- ./ElevatorProcess 5001 1 127.0.0.1 4001
- ./ElevatorProcess 5002 2 127.0.0.1 4001
- ./FloorProcess 4000 127.0.0.1 4001 input.txt
5) To run tests, enter the following command in a new terminal:
- "./iteration3_tests"

## Components
### Floor Subsystem
- FloorProcess.cpp:  Handles reading requests from an input file and sending them to the scheduler.
- Common.h / Common.cpp: Contains shared structures and utility functions, including FloorRequest serialization.

### Scheduler Subsystem
- SchedulerProcess.cpp: Acts as the central scheduler, receiving floor requests and assigning elevators based on proximity.

### Elevator Subsystem
- ElevatorProcess.cpp: Simulates an elevator, moving between floors and sending completion messages back to the scheduler.

### Global Variables
- Common.h / Common.cpp: Defines shared structures and functions, such as request serialization, UDP communication, and sleep simulation.

### Testing
iteration3_tests.cpp: test suite


The system operates using UDP communication for message passing between processes.

Elevators prioritize requests based on proximity and availability.

The project includes robust error handling for malformed requests and missing input files.

## Responsibilities
- Ahmed Ali -> Testing, SchedulerProcess.cpp, general code modification and debugging
- Wajd Mariam-> Diagrams, ElevatorProcess.cpp, general code modification and debugging.
- Albert Robu-> Testing
- Azan Huggins Goolamallee-> README.md and general code modification and debugging
- Sam Nieuwenhuis -> FloorProcess.cpp Diagrams


