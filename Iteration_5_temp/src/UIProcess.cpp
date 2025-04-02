#include <ncurses.h>
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

// Deserialize STATUS string into ElevatorStatus
ElevatorStatus deserializeStatus(const std::string& msg) {
    ElevatorStatus es;

    size_t idPos = msg.find("ElevID=");
    size_t floorPos = msg.find("Floor=");
    size_t faultPos = msg.find("Fault=");

    if (idPos != std::string::npos)
        es.id = std::stoi(msg.substr(idPos + 7, msg.find('|', idPos) - (idPos + 7)));

    if (floorPos != std::string::npos)
        es.currentFloor = std::stoi(msg.substr(floorPos + 6, msg.find('|', floorPos) - (floorPos + 6)));

    if (faultPos != std::string::npos)
        es.isFaulted = std::stoi(msg.substr(faultPos + 6)) != 0;

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

// Draw the current UI using ncurses
void displayUI() {
    while (true) {
        clear();
        int row = 1;

        mvprintw(row++, 2, "+=============== Elevator Status ===============+");
        for (size_t i = 0; i < liveStatus.size(); ++i) {
            const ElevatorStatus& e = liveStatus[i];
            mvprintw(row++, 2, "| Elevator %-2d | Floor: %-3d | Status: %-12s |",
                     e.id, e.currentFloor, 
                     e.isFaulted ? "FAULT" : "OK");
        }
        mvprintw(row++, 2, "+===============================================+");

        // Dummy request queue section for future expansion (optional)
        row++;
        mvprintw(row++, 2, "+================== Request Queue ======================+");
        mvprintw(row++, 2, "| (example) Req 1: Floor 2 => 8 | Passengers: 3          |");
        mvprintw(row++, 2, "| (example) Req 2: Floor 6 => 1 | Fault: Door Stuck      |");
        mvprintw(row++, 2, "| (example) Req 3: Floor 4 => 9 | Passengers: 5 (Over)   |");
        mvprintw(row++, 2, "+=======================================================+");

        mvprintw(row + 2, 2, "Press 'q' to quit.");

        refresh();

        // Allow graceful quit with 'q'
        timeout(100);  // wait 100ms
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

    std::thread listener(uiListenerThread);
    displayUI();  // this blocks forever

    listener.join();
    endwin();
    return 0;
}
