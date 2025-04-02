#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>
#include <vector>
#include <queue>
#include <atomic>
#include <fstream>
#include <sstream>
#include <chrono>
#include <cstdlib>
#include <unistd.h>     // for sleep()
#include <ncurses.h>    // for terminal UI

// ------------------------------------------------------------------
// Data Structures
// ------------------------------------------------------------------
struct FloorRequest {
    std::string timeStamp;
    int floor;
    std::string direction;
    int destination;
    bool hasFault;
    std::string faultType;
    int passengers;
};

enum class ElevatorState { WAITING, RECEIVING, MOVING, SENDING_FEEDBACK, STOPPED };

struct ElevatorStatus {
    int id;
    int currentFloor;
    bool doorsOpen;
    bool isFaulted;
    std::string state;  // e.g., "WAITING", "MOVING"
};

// ------------------------------------------------------------------
// Global Data
// ------------------------------------------------------------------
static const int NUM_ELEVATORS = 3;

// Queues & synchronization
std::mutex g_mutex;
std::condition_variable g_cvFloorRequests;
std::condition_variable g_cvElevatorRequests[NUM_ELEVATORS];

std::queue<FloorRequest> floorToScheduler;
std::queue<FloorRequest> schedulerToElev[NUM_ELEVATORS];

// Elevator states array
ElevatorStatus g_elevators[NUM_ELEVATORS];

// For stop signals
std::atomic<bool> g_stopAll(false);

// ------------------------------------------------------------------
// Helper: Sleep in ms
// ------------------------------------------------------------------
void simulateSleepMs(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

// ------------------------------------------------------------------
// Floor Thread: Reads requests from file, pushes to scheduler
// ------------------------------------------------------------------
void floorThreadFunc(const std::string &inputFile) {
    std::ifstream in(inputFile);
    if(!in) {
        std::cerr<<"[Floor] Could not open "<<inputFile<<"\n";
        return;
    }
    std::vector<FloorRequest> requests;
    {
        std::string line;
        while(std::getline(in, line)){
            if(line.empty()) continue;
            std::istringstream iss(line);
            FloorRequest fr;
            iss >> fr.timeStamp >> fr.floor >> fr.direction
                >> fr.destination >> std::boolalpha
                >> fr.hasFault >> fr.faultType >> fr.passengers;
            requests.push_back(fr);
        }
    }

    // Send to scheduler
    for(auto &r: requests) {
        {
            std::lock_guard<std::mutex> lock(g_mutex);
            floorToScheduler.push(r);
        }
        g_cvFloorRequests.notify_one();
        // Optional debug
        // std::cout<<"[Floor] Sent request "<<r.floor<<"->"<<r.destination<<"\n";
        simulateSleepMs(300);
    }
    std::cout<<"[Floor] Done sending requests.\n";
}

// ------------------------------------------------------------------
// Elevator logic (3 threads total), processes assigned requests
// ------------------------------------------------------------------
static const int ELEVATOR_CAPACITY = 4;
static const int MOVE_SPEED_MS = 500;

void elevatorThreadFunc(int id) {
    bool running = true;
    ElevatorState eState = ElevatorState::WAITING;
    bool hardFault  = false;
    bool doorsOpen  = true;
    int  currentFloor = 1;

    auto updateElevatorStatus = [&](const std::string &st){
        // Safely update global
        std::lock_guard<std::mutex> lk(g_mutex);
        g_elevators[id].id           = id;
        g_elevators[id].currentFloor = currentFloor;
        g_elevators[id].doorsOpen    = doorsOpen;
        g_elevators[id].isFaulted    = hardFault;
        g_elevators[id].state        = st;
    };

    auto doMovement = [&](const FloorRequest &fr){
        // Over capacity => ignore
        if(fr.passengers> ELEVATOR_CAPACITY){
            // std::cout<<"[Elev "<<id<<"] Over capacity.\n";
            return;
        }

        // close door
        if(doorsOpen){
            simulateSleepMs(300);
            doorsOpen=false;
            updateElevatorStatus("CLOSE_DOORS");
        }

        // move to pickup
        if(currentFloor!=fr.floor){
            int floors=std::abs(currentFloor - fr.floor);
            simulateSleepMs(floors * MOVE_SPEED_MS);
            currentFloor=fr.floor;
            updateElevatorStatus("MOVING_PICKUP");
        }

        // open
        doorsOpen=true;
        updateElevatorStatus("OPEN_PICKUP");
        simulateSleepMs(300);

        // close
        doorsOpen=false;
        updateElevatorStatus("CLOSE_PICKUP");
        simulateSleepMs(300);

        // move to dest
        if(currentFloor!=fr.destination){
            int floors=std::abs(fr.destination - currentFloor);
            simulateSleepMs(floors * MOVE_SPEED_MS);
            currentFloor=fr.destination;
            updateElevatorStatus("MOVING_DEST");
        }

        // open at dest
        doorsOpen=true;
        updateElevatorStatus("OPEN_DEST");
        simulateSleepMs(300);
    };

    while(running && !g_stopAll) {
        FloorRequest currentReq;
        switch(eState) {
            case ElevatorState::WAITING:{
                // wait for request from scheduler
                {
                    std::unique_lock<std::mutex> lk(g_mutex);
                    g_cvElevatorRequests[id].wait(lk, [&]{
                        return !schedulerToElev[id].empty() || g_stopAll;
                    });
                    if(g_stopAll) {
                        eState=ElevatorState::STOPPED;
                        break;
                    }
                    currentReq=schedulerToElev[id].front();
                    schedulerToElev[id].pop();
                }
                eState=ElevatorState::RECEIVING;
                updateElevatorStatus("WAITING->RECEIVING");
                break;
            }
            case ElevatorState::RECEIVING:{
                eState=ElevatorState::MOVING;
                updateElevatorStatus("RECEIVING->MOVING");
                break;
            }
            case ElevatorState::MOVING:{
                updateElevatorStatus("MOVING");
                doMovement(currentReq);
                eState=ElevatorState::SENDING_FEEDBACK;
                break;
            }
            case ElevatorState::SENDING_FEEDBACK:{
                // done
                updateElevatorStatus("SENDING_FEEDBACK");
                eState=ElevatorState::WAITING;
                break;
            }
            case ElevatorState::STOPPED:{
                updateElevatorStatus("STOPPED");
                running=false;
                break;
            }
        }
    }
    std::cout<<"[Elevator "<<id<<"] Exiting.\n";
}

// ------------------------------------------------------------------
// Scheduler logic
// Waits for floor requests, pick elevator, push to that elevator
// ------------------------------------------------------------------
int pickElevator(const FloorRequest &fr){
    int best=-1;
    int bestDist=999999;
    for(int i=0;i<NUM_ELEVATORS;++i){
        if(g_elevators[i].isFaulted) continue; // skip faulted
        int dist=std::abs(g_elevators[i].currentFloor - fr.floor);
        if(dist<bestDist){
            bestDist=dist; best=i;
        }
    }
    return best;
}

void schedulerThreadFunc(){
    bool running=true;
    while(running && !g_stopAll){
        FloorRequest req;
        {
            std::unique_lock<std::mutex> lk(g_mutex);
            g_cvFloorRequests.wait(lk,[&]{return !floorToScheduler.empty()||g_stopAll;});
            if(g_stopAll) break;

            req=floorToScheduler.front();
            floorToScheduler.pop();
        }
        // pick elevator
        int chosen=pickElevator(req);
        if(chosen<0){
            std::cout<<"[Scheduler] No elevator => discard\n";
            continue;
        }
        {
            std::lock_guard<std::mutex> lock(g_mutex);
            schedulerToElev[chosen].push(req);
        }
        g_cvElevatorRequests[chosen].notify_one();
    }
    std::cout<<"[Scheduler] Exiting.\n";
}

// ------------------------------------------------------------------
// UI logic with ncurses: display elevator states
// ------------------------------------------------------------------
void displayElevatorsNcurses(){
    clear();
    mvprintw(0, 0, "==== Elevator System ====");
    for(int i=0; i<NUM_ELEVATORS; ++i){
        int col = i*25;
        mvprintw(2, col, "+-----------------------+");
        mvprintw(3, col, "|  Elevator %d          |", g_elevators[i].id);
        mvprintw(4, col, "+-----------------------+");
        mvprintw(5, col, "| Floor:   %-3d         |", g_elevators[i].currentFloor);
        mvprintw(6, col, "| Door:  %-8s       |",
            g_elevators[i].doorsOpen? "OPEN":"CLOSED");
        mvprintw(7, col, "| Fault: %-8s      |",
            g_elevators[i].isFaulted? "YES":"NO");
        mvprintw(8, col, "| State: %-12s   |",
            g_elevators[i].state.c_str());
        mvprintw(9, col, "+-----------------------+");
    }
    refresh();
}

void uiThreadFuncNcurses(){
    // We can do a simple loop
    while(!g_stopAll){
        {
            std::lock_guard<std::mutex> lk(g_mutex);
            displayElevatorsNcurses(); // read from g_elevators
        }
        simulateSleepMs(1000);
    }
}

// ------------------------------------------------------------------
// Main => single process, multiple threads
// ------------------------------------------------------------------
int main(int argc,char *argv[]){
    if(argc<2){
        std::cerr<<"Usage: "<<argv[0]<<" <inputFile>\n";
        return 1;
    }

    // init elevator states
    for(int i=0;i<NUM_ELEVATORS;++i){
        g_elevators[i].id          = i;
        g_elevators[i].currentFloor=1;
        g_elevators[i].doorsOpen   =true;
        g_elevators[i].isFaulted   =false;
        g_elevators[i].state       ="WAITING";
    }

    // create threads
    std::thread floorTh(floorThreadFunc, argv[1]);
    std::thread schedTh(schedulerThreadFunc);

    std::vector<std::thread> elevatorThreads;
    for(int i=0;i<NUM_ELEVATORS;++i){
        elevatorThreads.emplace_back(elevatorThreadFunc, i);
    }

    // Setup ncurses
    initscr();
    noecho();
    cbreak();
    curs_set(0);

    std::thread uiTh(uiThreadFuncNcurses);

    // Wait for floor to finish
    floorTh.join();
    std::cout<<"[main] Floor done.\n";

    // we wait a bit for demonstration
    simulateSleepMs(5000);
    g_stopAll=true;

    // wake all
    for(int i=0;i<NUM_ELEVATORS;++i){
        g_cvElevatorRequests[i].notify_all();
    }
    g_cvFloorRequests.notify_all();

    schedTh.join();
    for(auto &th : elevatorThreads){
        th.join();
    }
    uiTh.join();

    endwin(); // close ncurses
    std::cout<<"[main] Program done.\n";
    return 0;
}
