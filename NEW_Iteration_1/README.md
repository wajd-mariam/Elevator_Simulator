# Elevator Control System - Iteration 1 - SYSC 3303 - B2 - Group 1 - Winter 2025


Authors:
Azan Huggins Goolamallee 101260082,
Albert Robu 101275700,
Wajd Mariam 101225633,
Samuel Nieuwenhuis 101225633

## Overview

This project is meant to create and begin testing a program specifically for elevator simulation. This is done by using 3 subsystems, the floor that people press the button to request an elevator, the elevator itself, and the scheduler which is used to mediate between the two

Date: February 1st

## Setup:
1) Ensure having C++11 compiler and pthread library installed

## Usage
1) Extract SYSC_3303_B2_G1_Iteration1.zip file
2) Open a new terminal window and copy the commands below:
- "g++ -std=c++11 -o app main.cpp floor.cpp scheduler.cpp elevator.cpp -lpthread"
- "./app"

## Components
### Elevator
- elevator.cpp: Holds class and functions dedicated to the elevator subclass
- elevator.h: Holds all definitions of fuctions and class of elevator subclass

### Floor
- floor.cpp: Holds class and functions dedicated to the floor subclass
- floor.h: Holds all definitions of functions and class of floor subclass
- FloorRequests.h: Holds FloorRequests structure

### scheduler
- scheduler.cpp: Holds class and functions dedicated to the scheduler subclass
- scheduler.h: Holds all definitions of functions and class of scheduler subclass

### test
- input.txt: Input file that holds the time, floor, direction, and destination of the people trying to use the elevator
  - The input.txt file contains four test cases.    

### main
- main.cpp: program where all threads communicate

## Responsibilities
Wajd Mariam-> Floor.cpp
Albert Robu-> Scheduler.cpp
Azan Huggins Goolamallee-> Elevator.cpp
Sam Nieuwenhuis -> UML diagrams
