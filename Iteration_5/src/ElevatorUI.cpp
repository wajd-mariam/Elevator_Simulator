// ElevatorUI.cpp
#include <ncurses.h>
#include <vector>
#include <string>
#include <thread>
#include <unistd.h> // for sleep()
#include <cstdlib>  // for system()
#include "Common.h"

// Launch backend processes
void launchProcesses() {
    std::cout << "Getting here." << std::endl;
    std::thread([] { system("./SchedulerProcess 4001 4000 3"); }).detach();
    std::thread([] { system("./ElevatorProcess 5000 0 127.0.0.1 4001"); }).detach();
    std::thread([] { system("./ElevatorProcess 5001 1 127.0.0.1 4001"); }).detach();
    std::thread([] { system("./ElevatorProcess 5002 2 127.0.0.1 4001"); }).detach();

    std::cout << "Running processes" << std::endl;
    sleep(3); // Ensure others are ready

    std::thread([] { system("./FloorProcess 4000 127.0.0.1 4001 input.txt"); }).detach();
    std::cout << "Starting floor process" << std::endl;
}

// UI display
void displayElevators(const std::vector<ElevatorStatus>& elevators) {
    clear();

    for (size_t i = 0; i < elevators.size(); ++i) {
        int col = i * 25;
        mvprintw(0, col, "+-----------------------+");
        mvprintw(1, col, "|   Elevator %d         |", elevators[i].id);
        mvprintw(2, col, "+-----------------------+");
        mvprintw(3, col, "| Floor:     %-3d       |", elevators[i].currentFloor);
        mvprintw(4, col, "| Door:   %-8s     |", elevators[i].doorsOpen ? "OPEN" : "CLOSED");
        mvprintw(5, col, "| Status: %-8s    |", elevators[i].isFaulted ? "FAULT" : "OK");
        mvprintw(6, col, "+-----------------------+");
    }

    refresh();
}

int main() {
    //clear();

    launchProcesses();  // Start all backend processes

    /**
    initscr();            // Start ncurses mode
    noecho();             // Don't echo keypresses
    cbreak();             // Disable line buffering
    curs_set(0);          // Hide cursor
     */
    while (true) {
        std::vector<ElevatorStatus> copy;
        {
            std::lock_guard<std::mutex> lock(globalElevatorMutex);
            copy = globalElevatorStatus;
        }

        std::cout << "[UI DEBUG] Snapshot size: " << copy.size() << std::endl;
        for (const auto& e : copy) {
            std::cout << "[UI DEBUG] Elevator " << e.id << " Floor: " << e.currentFloor
                    << " Door: " << (e.doorsOpen ? "Open" : "Closed")
                    << " Fault: " << (e.isFaulted ? "YES" : "NO")
                    << " State: " << e.state << std::endl;
        }
        //displayElevators(copy);
        sleep(1);
    }

    endwin();
    return 0;
}
