# Elevator Control System - Iteration 1

Authors:
Azan Huggins Goolamallee 101260082
Albert Robu 101275700
Wajd Mariam 101225633
Samuel Nieuwenhuis 101225633

## Overview

This project is meant to create and begin testing a program specifically for elevator simulation. This is done by using 3 subsystems, the floor that people press the button to request an elevator, the elevator itself, and the scheduler which is used to mediate between the two

Date: February 1st

## Requirements
*not sure what to put here

## Setup:
1) Ensure having C++11 compiler and pthread library installed

## Usage
1) Compile: "g++ -std=c++11 -o app main.cpp floor.cpp scheduler.cpp elevator.cpp -lpthread"
2)  Run: "./a"

## Components
### Elevator
- Elevator.cpp: Holds class and functions dedicated to the elevator subclass
- Elevator.h: Holds all definitions of fuctions and class of elevator subclass

### Floor
- floor.cpp: Holds class and functions dedicated to the floor subclass
- floor.h: Holds all definitions of functions and class of floor subclass
- FloorRequests.h:Holds FloorRequests structure

### scheduler
- scheduler.cpp: Holds class and functions dedicated to the scheduler subclass
- scheduler.h: Holds all definitions of functions and class of scheduler subclass

### test
- input.txt: Input file that holds the time, floor, direction, and destination of the people trying to use the elevator

### main
- main.cpp: program where all threads communicate

## Responsibilities
Wajd Mariam-> Floor.cpp, test files
Albert Robu-> Scheduler.cpp
Azan Huggins Goolamallee-> Elevator.cpp
Sam Nieuwenhuis -> UML diagrams
