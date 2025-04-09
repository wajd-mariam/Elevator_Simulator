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
std::mutex mtxDebug;

std::vector<FloorRequest> liveRequests;
std::mutex mtxRequests;

int statusCount = 0;

// Background thread to receive UDP status messages from elevators
void uiListenerThread() {
    int sock = createBoundSocket(6000);  // UI listens on port 6000

    while (true) {
        std::string ip; int port;
        auto msg = udpRecvString(sock, ip, port);
        if (!msg.empty()) {
            //std::cout << "[UI DEBUG] Received: " << msg << std::endl;
        }
        if (!msg.empty() && msg.find("STATUS") == 0) {
            ElevatorStatus status = deserializeElevatorStatus(msg);
            std::lock_guard<std::mutex> lock(mtx);
            if (status.id >= 0 && status.id < (int)liveStatus.size()) {
                statusCount += 1;
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
            FloorRequest fr = deserializeRequest(msg);

            // Debug output: print the deserialized request
            /**std::lock_guard<std::mutex> lock(mtxDebug);
            std::cout << "[UI DEBUG] Received Request: "
                      << "ReqID=" << fr.requestID
                      << ", Floor=" << fr.floor
                      << ", Dest=" << fr.destination
                      << ", Dir=" << fr.direction
                      << ", Passengers=" << fr.passengers
                      << ", Fault=" << fr.hasFault
                      << ", Type=" << fr.faultType
                      << std::endl;
            */    
            {
                std::lock_guard<std::mutex> lock(mtxRequests);
                liveRequests.push_back(fr);
            }
        }
        simulateSleepMs(100);
    }
}

// Helper function to classify fault type:
std::string classifyFault(const std::string& faultType) {
    if (faultType == "elevatorStuck" || faultType == "motorFailure")
        return "Hard";
    if (faultType == "doorStuck" || faultType == "lightMalfunction")
        return "Soft";
    return "None";
}

// Draw the current UI using ncurses
void displayUI() {
    while (true) {
        clear();
        int row = 1;

        mvprintw(row++, 2, "+================= Elevator Status ==========================+");
        mvprintw(row++, 2, "| Elevator | Floor | Dir   |   Status  |    Fault Type       |");
        mvprintw(row++, 2, "+----------+-------+-------+-----------+---------------------+");

        {
            std::lock_guard<std::mutex> lock(mtx);  // lock liveStatus
            for (const auto& e : liveStatus) {
                // Classifying and assigning fault types appropriately:
                std::string faultDisplay = "None";
                //if (e.isFaulted) {
                    // Classify the fault
                if (e.faultType == "elevatorStuck" || e.faultType == "motorFailure")
                    faultDisplay = "Hard: " + e.faultType;
                else if (e.faultType == "doorStuck" || e.faultType == "lightMalfunction")
                    faultDisplay = "Soft: " + e.faultType;
                else
                    faultDisplay = "Unknown: " + e.faultType;
                //}

                mvprintw(row++, 2, "|   %-6d | %-5d | %-5s | %-9s | %-19s |",
                         e.id,
                         e.currentFloor,
                         e.direction.c_str(),
                         e.isFaulted ? "FAULT" : e.state.c_str(),
                         faultDisplay.c_str());
            }
        }

        mvprintw(row++, 2, "+==============================================================+");

        // Display Floor Request Queue
        row++;
        mvprintw(row++, 2, "+==================== Request Queue ========================+");

        {
            std::lock_guard<std::mutex> lock(mtxRequests);  // lock liveRequests
            if (liveRequests.empty()) {
                mvprintw(row++, 2, "| (No pending requests)                                      |");
            } else {
                for (const auto& r : liveRequests) {
                    mvprintw(row++, 2, "| Req ID %-3d: Floor %-2d -> %-2d | Passengers: %-2d             |", 
                             r.requestID, r.floor, r.destination, r.passengers);
                }
            }
        }

        mvprintw(row++, 2, "+============================================================+");

        // Footer
        mvprintw(row + 1, 2, "Press 'q' to quit.");

        refresh();

        // Allow quitting with 'q'
        timeout(100);  // non-blocking input wait
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
    displayUI(); 
    
    endwin();// Restore terminal state
    requestListener.join();
    return 0;
}
