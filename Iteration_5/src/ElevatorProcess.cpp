#include "Common.h"

enum class ElevatorState {
    WAITING,
    RECEIVING,
    MOVING,
    SENDING_FEEDBACK,
    STOPPED
};

static const int ELEVATOR_CAPACITY = 4;

static bool hardFault      = false;
static bool doorStuckFault = false;
static bool triedFixDoor   = false;

static std::atomic<bool> stopAll(false);
static std::queue<FloorRequest> reqQueue;
static std::mutex mtxReq;
static std::condition_variable cvReq;

static int elevatorID   = 0;
static int listenSock   = -1;
static int sendSock     = -1;
static std::string schedIP;
static int schedPort    = 0;
static int currentFloor = 1;
static bool doorsOpen   = true;
static ElevatorState eState = ElevatorState::WAITING;

static const int moveSpeedMs   = 500;
static const int TIMEOUT_THRESHOLD = 15;

// "FAULT|ElevID=x|desc"
static void sendFault(const std::string &desc){
    std::string fmsg="FAULT|ElevID="+std::to_string(elevatorID)+"|"+desc;
    udpSendString(sendSock, fmsg, schedIP, (schedPort + 100));
}

static void checkDoorStuck(){
    if(doorStuckFault && !triedFixDoor){
        //std::cout<<"Door stuck\n";
        doorStuckFault = false;
        triedFixDoor   = true;
        //std::cout<<"[Elevator "<<elevatorID<<"] Freed stuck door\n";
        return;
    }
    if(doorStuckFault && triedFixDoor){
        //std::cout<<"[Elevator "<<elevatorID<<"] Door stuck again => HARD FAULT\n";
        //std::cout<<"Hard fault\n";
        hardFault = true;
        eState    = ElevatorState::STOPPED;
        sendFault("doorStuckPermanent");
    }
}

static void movementTimer(int floors, int expectedMs){
    auto start = std::chrono::steady_clock::now();
    int margin = 2000;
    while(!hardFault && !stopAll){
        simulateSleepMs(50);
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
        if(elapsed > (expectedMs + margin)){
            //std::cout<<"[Elevator "<<elevatorID<<"] Timer => HARD FAULT\n";
            //std::cout<<"Hard fault\n";
            hardFault = true;
            eState    = ElevatorState::STOPPED;
            sendFault("stuckBetweenFloors");
            return;
        }
    }
}

static void moveFloors(int from, int to){
    int floors = std::abs(to - from);
    if(floors <= 0) return;

    int expectedTime = floors * moveSpeedMs;
    std::thread timerThread;
    bool useTimer = (floors >= TIMEOUT_THRESHOLD);

    if(useTimer){
        timerThread = std::thread(movementTimer, floors, expectedTime);
    }

    //std::cout << "[Elevator " << elevatorID << "] Moving from "
              //<< from << " to " << to << "\n";

    simulateSleepMs(expectedTime);

    if(!hardFault){
        currentFloor = to;
    }

    if(useTimer){
        timerThread.detach();
    }
}

static void doMovement(const FloorRequest &r){
    // Check for immediate stuckElevator => Hard fault
    if(r.hasFault && r.faultType == "stuckElevator"){
        //std::cout<<"Hard fault\n";
        hardFault = true;
        eState    = ElevatorState::STOPPED;
        sendFault("stuckElevator");
        return;
    }

    // Check doorStuck => set doorStuckFault => next door action triggers it
    if(r.hasFault && r.faultType=="doorStuck"){
        doorStuckFault = true;
    }

    // capacity check
    if(r.passengers > ELEVATOR_CAPACITY){
        //std::cout<<"Full\n";
        //std::cout<<"[Elevator "<<elevatorID<<"] cannot take "
                // <<r.passengers<<"\n";
        return;
    }

    // close door if open
    if(doorsOpen){
        // std::cout<<"[Elevator "<<elevatorID<<"] Doors closing.\n";
        simulateSleepMs(300);
        checkDoorStuck();
        if(hardFault) return;
        doorsOpen = false;
    }

    // move to pickup floor
    if(currentFloor != r.floor){
        moveFloors(currentFloor, r.floor);
        if(hardFault) return;
    }

    // open door
    // std::cout<<"[Elevator "<<elevatorID<<"] Doors opening.\n";
    doorsOpen = true;
    simulateSleepMs(300);
    checkDoorStuck();
    if(hardFault) return;

    // close door
    // std::cout<<"[Elevator "<<elevatorID<<"] Doors closing.\n";
    doorsOpen = false;
    simulateSleepMs(300);
    checkDoorStuck();
    if(hardFault) return;

    // move to destination
    if(currentFloor != r.destination){
        moveFloors(currentFloor, r.destination);
        if(hardFault) return;
    }

    // open at destination
    //std::cout<<"[Elevator "<<elevatorID<<"] Doors opening at floor "
            // <<currentFloor<<"\n";
    doorsOpen = true;
    simulateSleepMs(300);
    checkDoorStuck();
}

static void listenerThread(){
    while(!stopAll){
        std::string ip; 
        int p;
        auto data = udpRecvString(listenSock, ip, p);
        if(data.empty()){
            simulateSleepMs(50);
            continue;
        }
        FloorRequest fr = deserializeRequest(data);
        {
            std::lock_guard<std::mutex> lk(mtxReq);
            reqQueue.push(fr);
        }
        cvReq.notify_all();
    }
}

static void elevatorMainLoop(){
    FloorRequest currentReq;
    bool running = true;
    while(running){
        switch(eState){
        case ElevatorState::WAITING:{
            std::unique_lock<std::mutex> lk(mtxReq);
            cvReq.wait(lk, [&]{return !reqQueue.empty() || stopAll;});
            if(stopAll && reqQueue.empty()){
                eState=ElevatorState::STOPPED;
                break;
            }
            currentReq = reqQueue.front();
            reqQueue.pop();
            lk.unlock();

            // Updating global shared data:
            globalElevatorStatus[elevatorID].state = "WAITING";
            globalElevatorStatus[elevatorID].currentFloor = currentFloor;
            globalElevatorStatus[elevatorID].doorsOpen = doorsOpen;
            globalElevatorStatus[elevatorID].isFaulted = hardFault;

            //std::cout<<"[Elevator "<<elevatorID<<"] Received request floor="
                     //<< currentReq.floor <<"->"<< currentReq.destination
                     //<<" fault="<<(currentReq.hasFault?1:0)<<"("
                     //<< currentReq.faultType <<") pass="
                     //<< currentReq.passengers <<"\n";

            eState = ElevatorState::RECEIVING;
            break;
        }
        case ElevatorState::RECEIVING:{
            eState = ElevatorState::MOVING;
            break;
        }
        case ElevatorState::MOVING:{
            // Update initial status before movement starts
            globalElevatorStatus[elevatorID].state = "MOVING";
            globalElevatorStatus[elevatorID].currentFloor = currentFloor;
            globalElevatorStatus[elevatorID].doorsOpen = doorsOpen;
            globalElevatorStatus[elevatorID].isFaulted = hardFault;
            
            // Do actual movement
            doMovement(currentReq);
            // After movement finishes, update status again
            globalElevatorStatus[elevatorID].currentFloor = currentFloor;
            globalElevatorStatus[elevatorID].doorsOpen = doorsOpen;
            globalElevatorStatus[elevatorID].isFaulted = hardFault;
            globalElevatorStatus[elevatorID].state = hardFault ? "FAULT" : "IDLE";

            // Determine next state
            if(!hardFault){
                eState = ElevatorState::SENDING_FEEDBACK;
            }else{
                eState = ElevatorState::STOPPED;
            }
            break;
        }
        case ElevatorState::SENDING_FEEDBACK:{
            FloorRequest done = currentReq;
            if(done.passengers > ELEVATOR_CAPACITY){
                done.floor       = currentFloor;
                done.destination = currentFloor;
            } else if(!hardFault){
                done.floor       = currentFloor;
                done.destination = currentFloor;
            }
            done.timeStamp += "(ElevID=" + std::to_string(elevatorID) + ")";
            //std::cout<<"[Elevator "<<elevatorID<<"] Completed request.\n";

            auto msg = serializeRequest(done);
            udpSendString(sendSock, msg, schedIP, schedPort + 100);

            // Update UI status
            globalElevatorStatus[elevatorID].currentFloor = currentFloor;
            globalElevatorStatus[elevatorID].doorsOpen = doorsOpen;
            globalElevatorStatus[elevatorID].isFaulted = hardFault;
            globalElevatorStatus[elevatorID].state = "WAITING";

            eState = ElevatorState::WAITING;
            break;
        }
        case ElevatorState::STOPPED:{
            //std::cout<<"[Elevator "<<elevatorID<<"] STOPPED.\n";
            // Update UI status
            globalElevatorStatus[elevatorID].state = "FAULT";
            globalElevatorStatus[elevatorID].isFaulted = true;
            globalElevatorStatus[elevatorID].doorsOpen = doorsOpen;
            globalElevatorStatus[elevatorID].currentFloor = currentFloor;

            running = false;
            break;
        }
        }
    }
}

int main(int argc,char* argv[]){
    if(argc < 5){
        std::cerr<<"Usage: "<<argv[0]
                 <<" <listenPort> <elevatorID> <schedulerIP> <schedulerPort>\n";
        return 1;
    }
    int myPort  = std::stoi(argv[1]);
    elevatorID  = std::stoi(argv[2]);
    schedIP     = argv[3];
    schedPort   = std::stoi(argv[4]);

    /// Initialize global elevator status
    if (globalElevatorStatus.size() <= elevatorID)
    globalElevatorStatus.resize(elevatorID + 1);

    globalElevatorStatus[elevatorID].id = elevatorID;
    globalElevatorStatus[elevatorID].currentFloor = currentFloor;
    globalElevatorStatus[elevatorID].doorsOpen = doorsOpen;
    globalElevatorStatus[elevatorID].isFaulted = hardFault;
    globalElevatorStatus[elevatorID].state = "WAITING"; 

    listenSock = createBoundSocket(myPort);
    if(listenSock < 0) return 1;
    sendSock   = socket(AF_INET, SOCK_DGRAM, 0);

    std::thread lt(listenerThread);
    elevatorMainLoop();

    stopAll = true;
    lt.join();
    close(listenSock);
    close(sendSock);

    //std::cout<<"[Elevator "<<elevatorID<<"] Exiting.\n";
    return 0;
}
