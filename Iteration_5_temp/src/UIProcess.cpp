#include <ncurses.h>
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <unistd.h>         // for sleep()
#include <cstdlib>          // for system()
#include <sstream>
#include "Common.h"         // includes ElevatorStatus, udpRecvString, createBoundSocket, etc.

std::vector<ElevatorStatus> liveStatus(3);  // for 3 elevators
std::mutex mtx;

std::vector<FloorRequest> liveRequests;
std::mutex mtxRequests;

// Deserialize STATUS string into ElevatorStatus
ElevatorStatus deserializeStatus(const std::string& msg) {
    ElevatorStatus es;

    std::istringstream iss(msg);
    std::string token;

    while (std::getline(iss, token, '|')) {
        if (token.find("ElevID=") == 0)
            es.id = std::stoi(token.substr(7));
        else if (token.find("Floor=") == 0)
            es.currentFloor = std::stoi(token.substr(6));
        else if (token.find("Fault=") == 0)
            es.isFaulted = std::stoi(token.substr(6)) != 0;
        else if (token.find("Door=") == 0)
            es.doorsOpen = std::stoi(token.substr(5)) != 0;
        else if (token.find("Direction=") == 0)
            es.direction = token.substr(10);  // UP, DOWN, IDLE
        else if (token.find("State=") == 0)
            es.state = token.substr(6);       // MOVING, WAITING, etc.
    }
    return es;
}

// Background thread to receive UDP status messages from elevators
void uiListenerThread() {
    int sock = createBoundSocket(6000);  // UI listens on port 6000

    while (true) {
        std::string ip; int port;
        auto msg = udpRecvString(sock, ip, port);
        if (!msg.empty() && msg.find("STATUS") == 0) {
            ElevatorStatus status = deserializeStatus(msg);
            std::lock_guard<std::mutex> lock(mtx);
            if (status.id >= 0 && status.id < (int)liveStatus.size()) {
                liveStatus[status.id] = status;
            }
        }
    }
}

void requestListenerThread() {
    int sock = createBoundSocket(6001);  // requests arrive at port 6001
    if (sock < 0) return;

    while (true) {
        std::string ip;
        int port;
        auto msg = udpRecvString(sock, ip, port);

        if (!msg.empty()) {
            try {
                FloorRequest fr = deserializeRequest(msg);
                std::lock_guard<std::mutex> lock(mtxRequests);
                liveRequests.push_back(fr);
            } catch (...) {
                // malformed input, skip
            }
        }
    }
}


// Draw the current UI using ncurses
void displayUI() {
    while (true) {
        clear();
        int row = 1;

        mvprintw(row++, 2, "+================ Elevator Status ================+");
        mvprintw(row++, 2, "| Elevator | Floor | Direction |     Status       |");
        mvprintw(row++, 2, "+----------+-------+-----------+------------------+");

        for (const auto& e : liveStatus) {
            mvprintw(row++, 2, "|    %-5d |  %-4d |  %-8s |  %-16s |",
                     e.id, e.currentFloor, 
                     e.direction.c_str(),
                     e.isFaulted ? "FAULT" : e.state.c_str());
        }

        mvprintw(row++, 2, "+=================================================+");

        // Display Floor Request Queue
        row++;
        mvprintw(row++, 2, "+================== Request Queue ======================+");

        {
            std::lock_guard<std::mutex> lock(mtxRequests);  // Protect access to liveRequests
            if (liveRequests.empty()) {
                mvprintw(row++, 2, "| (No pending requests)                                 |");
            } else {
                int count = 1;
                for (const auto& r : liveRequests) {
                    mvprintw(row++, 2, "| Req %-2d: Floor %-2d -> %-2d | Passengers: %-2d        |", 
                             count++, r.floor, r.destination, r.passengers);
                }
            }   
        }

        mvprintw(row++, 2, "+=======================================================+");
        mvprintw(row + 2, 2, "Press 'q' to quit.");

        refresh();

        timeout(100);  // non-blocking key wait
        int ch = getch();
        if (ch == 'q' || ch == 'Q') break;

        sleep(1);
    }
}

// Launch elevator + scheduler + floor processes
void launchProcesses() {
    std::thread([] { system("./SchedulerProcess 4001 4000 3"); }).detach();
    std::thread([] { system("./ElevatorProcess 5000 0 127.0.0.1 4001"); }).detach();
    std::thread([] { system("./ElevatorProcess 5001 1 127.0.0.1 4001"); }).detach();
    std::thread([] { system("./ElevatorProcess 5002 2 127.0.0.1 4001"); }).detach();

    sleep(2); // delay to ensure scheduler is up
    std::thread([] { system("./FloorProcess 4000 127.0.0.1 4001 input.txt"); }).detach();
}

int main() {
    launchProcesses();

    initscr(); noecho(); cbreak(); curs_set(0);

    std::thread elevatorListener(uiListenerThread);       // Receives STATUS updates from elevators
    std::thread requestListener(requestListenerThread);   // Receives REQUEST updates from FloorProcess
    displayUI();  // this blocks forever

    elevatorListener.join();
    requestListener.join();

    endwin();// Restore terminal state
    return 0;
}
