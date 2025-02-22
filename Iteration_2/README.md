# Elevator Control System - Iteration 1 - SYSC 3303 - B2 - Group 1 - Winter 2025


Authors:
Ahmed Ali 101181126,
Azan Huggins Goolamallee 101260082,
Albert Robu 101275700,
Wajd Mariam 101225633,
Samuel Nieuwenhuis 101225633

## Overview

This project is meant to create and begin testing a program specifically for elevator simulation. This is done by using 3 subsystems, the floor that people press the button to request an elevator, the elevator itself, and the scheduler which is used to mediate between the two

Date: February 22nd, 2025

## Setup:
1) Ensure having C++11 compiler and pthread library installed
2) sudo apt install make libgtest-dev -y

## Usage
1) Extract SYSC_3303_B2_G1_Iteration2.zip file
2) Open a new terminal window and copy the commands below:
- "make clean"
- "make"
- "./ElevatorSystem"
- "./runTests"

## Components
### Floor Subsystem
- floor.cpp: Holds class and functions dedicated to the floor subclass
- floor.h: Holds all definitions of functions and class of floor subclass
- FloorRequests.h: Holds FloorRequests structure

### Scheduler Subsystem
- scheduler.cpp: Holds class and functions dedicated to the scheduler subclass
- scheduler.h: Holds all definitions of functions and class of scheduler subclass

### Elevator Subsystem
- elevator.cpp: Holds class and functions dedicated to the elevator subclass
- elevator.h: Holds all definitions of fuctions and class of elevator subclass

### Global Variables
- globals.cpp: initializes all global variables
- globals.h: declares all global variables

### Testing
iteration2_tests.cpp: test suite

### main
- main.cpp: program where all threads communicate

## Responsibilities
- Ahmed Ali -> Testing, general code modification and debugging
- Wajd Mariam-> Diagrams, README.md, general code modification and debugging.
- Albert Robu-> Testing
- Azan Huggins Goolamallee-> Floor.cpp
- Sam Nieuwenhuis -> Scheduler.cpp
