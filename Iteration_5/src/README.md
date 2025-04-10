# Elevator Control System - Iteration 5 - SYSC 3303 - B2 - Group 1 - Winter 2025

Authors:
Ahmed Ali 101181126,
Azan Huggins Goolamallee 101260082,
Albert Robu 101275700,
Wajd Mariam 101225633,
Samuel Nieuwenhuis 101225633

## Overview

This project is meant to create and begin testing a program specifically for elevator simulation. This is done by using 3 subsystems, the floor that people press the button to request an elevator, the elevator itself, and the scheduler which is used to mediate between the two

Date: March 22nd, 2025

## Setup:
1) Ensure having C++11 compiler and pthread library installed
2) sudo apt install make libgtest-dev -y

## Usage
1) Extract SYSC_3303_B2_G1_Iteration5.zip file
2) Navigate to the "src" directory
3) Open a terminal in the "src" directory and enter:
- "make clean"
- "make"
4) To run the UI, enter the following command:
- "./ElevatorUI"
5) To run tests, enter the following command in a new terminal:
- "./iteration5_tests"

## Components

### Common 
- Common.h / Common.cpp: Contains shared structures and utility functions, including FloorRequest serialization.

### Floor Subsystem
- FloorProcess.cpp:  Handles reading requests from an input file and sending them to the scheduler.

### Scheduler Subsystem
- SchedulerProcess.cpp: Acts as the central scheduler, receiving floor requests and assigning elevators based on proximity.

### Elevator Subsystem
- ElevatorProcess.cpp: Simulates an elevator, moving between floors and sending completion messages back to the scheduler.

### Testing
iteration5_tests.cpp: test suite

The system operates using UDP communication for message passing between processes.

Elevators prioritize requests based on proximity and availability.

The project includes robust error handling for malformed requests and missing input files.

## Responsibilities
- Ahmed Ali -> Testing and general code modification and debugging
- Wajd Mariam-> UI development and general code modification and debugging.
- Albert Robu-> Testing
- Azan Huggins Goolamallee-> README.md and general code modification and debugging
- Sam Nieuwenhuis -> Timing Diagrams


