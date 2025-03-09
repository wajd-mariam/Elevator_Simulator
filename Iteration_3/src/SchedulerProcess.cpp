/* SchedulerProcess.cpp - Manages elevator scheduling and request handling */
#include "Common.h"

// Enum representing different scheduler states
enum class SchedulerState {
    WAIT_FOR_REQUEST,
    PROCESS_REQUEST,
    SELECT_ELEVATOR,
    SEND_TO_ELEVATOR,
    WAIT_FOR_ELEVATOR,
    NOTIFY_FLOOR,
    TERMINATE
};

// Structure to hold elevator status information
struct ElevatorStatus {
    int id;
    int currentFloor;
};

// Global flags and data structures for synchronization
static std::atomic<bool> stopAll(false);
static std::queue<FloorRequest> floorRequests;
static std::mutex mtxFloor;
static std::condition_variable cvFloor;

static std::queue<FloorRequest> elevatorCompletions;
static std::mutex mtxElev;
static std::condition_variable cvElev;

static std::vector<ElevatorStatus> elevInfo;

static int floorSock=-1;
static int elevatorSock=-1;
static int sendSock=-1;
static int floorPort=0;
static int schedPort=0;

// Thread to listen for floor requests
void floorListenerThread(){
    while(!stopAll){
        std::string ip; int p;
        auto data=udpRecvString(floorSock,ip,p);
        if(data.empty()){simulateSleepMs(100);continue;}
        auto fr=deserializeRequest(data);
        {
            std::lock_guard<std::mutex> lk(mtxFloor);
            floorRequests.push(fr);
        }
        cvFloor.notify_all();
    }
}

// Thread to listen for elevator completions
void elevatorListenerThread(){
    while(!stopAll){
        std::string ip; int p;
        auto data=udpRecvString(elevatorSock,ip,p);
        if(data.empty()){simulateSleepMs(100);continue;}
        auto comp=deserializeRequest(data);
        {
            std::lock_guard<std::mutex> lk(mtxElev);
            elevatorCompletions.push(comp);
        }
        cvElev.notify_all();
    }
}

// Function to pick the best elevator for a request
int pickBestElevator(const FloorRequest &r){
    int best=-1; 
    int bestDist=99999;
    for(auto &e: elevInfo){
        int dist=std::abs(e.currentFloor - r.floor);
        if(dist<bestDist){
            bestDist=dist; best=e.id;
        }
    }
    if(best<0 && !elevInfo.empty()) best=elevInfo[0].id;
    return best;
}

// Usage: ./SchedulerProcess <schedulerPort> <floorPort> <numElevators>
// Main function - sets up scheduler and handles requests
int main(int argc,char* argv[]){
    if(argc<4){
        std::cerr<<"Usage: "<<argv[0]<<" <schedulerPort> <floorPort> <numElevators>\n";
        return 1;
    }
    schedPort   = std::stoi(argv[1]);
    floorPort   = std::stoi(argv[2]);
    int nElev   = std::stoi(argv[3]);

    // create elevator statuses
    for(int i=0;i<nElev;i++){
        ElevatorStatus es; es.id=i; es.currentFloor=1;
        elevInfo.push_back(es);
    }

    // Create sockets for communication
    floorSock=createBoundSocket(schedPort);
    if(floorSock<0)return 1;
    // elevatorSock binds on schedPort+100
    elevatorSock=createBoundSocket(schedPort+100);
    if(elevatorSock<0)return 1;
    sendSock=socket(AF_INET,SOCK_DGRAM,0);

    // Start listener threads
    std::thread flTh(floorListenerThread);
    std::thread elTh(elevatorListenerThread);

    SchedulerState st=SchedulerState::WAIT_FOR_REQUEST;
    FloorRequest current;
    bool running=true;
    while(running){
        switch(st){
        case SchedulerState::WAIT_FOR_REQUEST:{
            std::unique_lock<std::mutex> lk(mtxFloor);
            cvFloor.wait(lk,[&]{return !floorRequests.empty()||stopAll;});
            if(stopAll && floorRequests.empty()){st=SchedulerState::TERMINATE;break;}
            current=floorRequests.front();
            floorRequests.pop();
            lk.unlock();
            std::cout<<"[Scheduler] Received request floor="
                     <<current.floor<<"->"<<current.destination<<"\n";
            st=SchedulerState::PROCESS_REQUEST;
            break;
        }
        case SchedulerState::PROCESS_REQUEST:{
            st=SchedulerState::SELECT_ELEVATOR;
            break;
        }
        case SchedulerState::SELECT_ELEVATOR:{
            int best=pickBestElevator(current);
            std::cout<<"[Scheduler] Chose elevator "<<best<<"\n";
            // store eID in timeStamp or direction, etc.
            current.timeStamp+="(EID="+std::to_string(best)+")";
            st=SchedulerState::SEND_TO_ELEVATOR;
            break;
        }
        case SchedulerState::SEND_TO_ELEVATOR:{
            // parse out eID from current
            int eID=0;
            auto pos=current.timeStamp.rfind("(EID=");
            if(pos!=std::string::npos){
                eID=std::stoi(current.timeStamp.substr(pos+5));
            }
            int elevatorPort=5000+ eID;
            auto msg=serializeRequest(current);
            udpSendString(sendSock,msg,"127.0.0.1",elevatorPort);
            std::cout<<"[Scheduler] Sent request to Elevator "<<eID
                     <<" on port "<<elevatorPort<<"\n";
            st=SchedulerState::WAIT_FOR_ELEVATOR;
            break;
        }
        case SchedulerState::WAIT_FOR_ELEVATOR:{
            FloorRequest comp;
            {
                std::unique_lock<std::mutex> lk(mtxElev);
                cvElev.wait(lk,[&]{return !elevatorCompletions.empty()||stopAll;});
                if(stopAll && elevatorCompletions.empty()){
                    st=SchedulerState::TERMINATE;break;
                }
                comp=elevatorCompletions.front();
                elevatorCompletions.pop();
            }
            std::cout<<"[Scheduler] Elevator done floor="
                     <<comp.floor<<"->"<<comp.destination<<"\n";
            // parse elevator ID from comp.timeStamp e.g.  ...(ElevID=1)
            int eID=0;
            auto pos=comp.timeStamp.rfind("(ElevID=");
            if(pos!=std::string::npos){
                eID=std::stoi(comp.timeStamp.substr(pos+8));
            }
            // update that elevator's floor
            elevInfo[eID].currentFloor=comp.destination;

            current=comp; 
            st=SchedulerState::NOTIFY_FLOOR;
            break;
        }
        case SchedulerState::NOTIFY_FLOOR:{
            // send ack
            auto ackMsg=serializeRequest(current);
            udpSendString(sendSock,ackMsg,"127.0.0.1",floorPort);
            std::cout<<"[Scheduler] Acked to Floor on port "<<floorPort<<"\n";
            st=SchedulerState::WAIT_FOR_REQUEST;
            break;
        }
        case SchedulerState::TERMINATE:{
            running=false;
            break;
        }
        }
    }

    stopAll=true;
    flTh.join();
    elTh.join();
    close(floorSock);
    close(elevatorSock);
    close(sendSock);
    std::cout<<"[Scheduler] Exiting...\n";
    return 0;
}
