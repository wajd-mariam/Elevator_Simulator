#include "Common.h"

enum class ElevatorState {
    WAIT_FOR_SCHEDULER,
    RECEIVE_INSTRUCTIONS,
    MOVING_TO_FLOOR,
    SEND_FEEDBACK,
    STOPPED
};

static std::atomic<bool> stopAll(false);
static std::queue<FloorRequest> elevatorQueue;
static std::mutex mtxElev;
static std::condition_variable cvElev;

static int elevatorID=0;
static int listenSock=-1;
static int sendSock=-1;
static int currentFloor=1;
static bool doorsOpen=true;
static ElevatorState eState=ElevatorState::WAIT_FOR_SCHEDULER;
static int movementSpeed=500;

static std::string schedIP;
static int schedPort=0; // Send completions to schedPort+100

void elevatorListenerThread(){
    while(!stopAll){
        std::string ip; int p;
        auto data=udpRecvString(listenSock,ip,p);
        if(data.empty()){simulateSleepMs(100);continue;}
        auto fr=deserializeRequest(data);
        {
            std::lock_guard<std::mutex> lk(mtxElev);
            elevatorQueue.push(fr);
        }
        cvElev.notify_all();
    }
}

static void doMovementLogic(const FloorRequest &req){
    if(doorsOpen){
        doorsOpen=false;
        std::cout<<"[Elevator "<<elevatorID<<"] Doors closing.\n";
        simulateSleepMs(300);
    }
    int dist=std::abs(currentFloor-req.floor);
    if(dist>0){
        std::cout<<"[Elevator "<<elevatorID<<"] Moving from "<<currentFloor<<" to "<<req.floor<<"\n";
        simulateSleepMs(dist*movementSpeed);
        currentFloor=req.floor;
    }
    std::cout<<"[Elevator "<<elevatorID<<"] Doors opening.\n";
    doorsOpen=true; simulateSleepMs(300);
    std::cout<<"[Elevator "<<elevatorID<<"] Doors closing.\n";
    doorsOpen=false;simulateSleepMs(300);

    dist=std::abs(currentFloor-req.destination);
    if(dist>0){
        std::cout<<"[Elevator "<<elevatorID<<"] Moving from "<<currentFloor
                 <<" to "<<req.destination<<"\n";
        simulateSleepMs(dist*movementSpeed);
        currentFloor=req.destination;
    }
    std::cout<<"[Elevator "<<elevatorID<<"] Doors opening at floor "<<currentFloor<<"\n";
    doorsOpen=true; simulateSleepMs(300);
}

void elevatorMainLoop(){
    bool running=true;
    FloorRequest currentReq;
    while(running){
        switch(eState){
        case ElevatorState::WAIT_FOR_SCHEDULER:{
            std::unique_lock<std::mutex> lk(mtxElev);
            cvElev.wait(lk,[&]{return !elevatorQueue.empty()||stopAll;});
            if(stopAll && elevatorQueue.empty()){eState=ElevatorState::STOPPED;break;}
            currentReq=elevatorQueue.front();
            elevatorQueue.pop();
            lk.unlock();
            std::cout<<"[Elevator "<<elevatorID<<"] Received request floor "
                     <<currentReq.floor<<"->"<<currentReq.destination<<"\n";
            eState=ElevatorState::RECEIVE_INSTRUCTIONS;
            break;
        }
        case ElevatorState::RECEIVE_INSTRUCTIONS:{
            eState=ElevatorState::MOVING_TO_FLOOR;
            break;
        }
        case ElevatorState::MOVING_TO_FLOOR:{
            doMovementLogic(currentReq);
            eState=ElevatorState::SEND_FEEDBACK;
            break;
        }
        case ElevatorState::SEND_FEEDBACK:{
            // Let's embed the elevatorID in the timeStamp so scheduler can parse it
            std::cout<<"[Elevator "<<elevatorID<<"] Completed request.\n";
            FloorRequest doneReq = currentReq;
            // put something like "(eID=1)" in doneReq.timeStamp if you want
            doneReq.timeStamp+="(ElevID="+std::to_string(elevatorID)+")";
            auto msg=serializeRequest(doneReq);
            // send to schedulerPort+100
            udpSendString(sendSock,msg,schedIP,schedPort+100);
            eState=ElevatorState::WAIT_FOR_SCHEDULER;
            break;
        }
        case ElevatorState::STOPPED:{
            running=false;
            break;
        }
        }
    }
}

int main(int argc,char* argv[]){
    // Usage: ./ElevatorProcess <listenPort> <elevatorID> <schedulerIP> <schedulerPort>
    if(argc<5){
        std::cerr<<"Usage: "<<argv[0]<<" <listenPort> <elevatorID> <schedulerIP> <schedulerPort>\n";
        return 1;
    }
    int myPort       = std::stoi(argv[1]);
    elevatorID       = std::stoi(argv[2]);
    schedIP          = argv[3];
    schedPort        = std::stoi(argv[4]);

    listenSock=createBoundSocket(myPort);
    if(listenSock<0)return 1;
    sendSock=socket(AF_INET,SOCK_DGRAM,0);

    std::thread thr(elevatorListenerThread);
    elevatorMainLoop();

    stopAll=true;
    thr.join();
    close(listenSock);
    close(sendSock);
    std::cout<<"[Elevator "<<elevatorID<<"] Exiting.\n";
    return 0;
}
