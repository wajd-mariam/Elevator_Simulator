#include "Common.h"

enum class SchedulerState {
    WAIT_FOR_REQUEST,
    PROCESS_REQUEST,
    SELECT_ELEVATOR,
    SEND_TO_ELEVATOR,
    WAIT_FOR_ELEVATOR,
    NOTIFY_FLOOR,
    TERMINATE
};

struct ElevatorStatus {
    int id;
    int currentFloor;
    bool isFaulted;
};

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
static int numElevators=0;

static void printConsole(){
    std::cout<<"\n=== Elevator Console ===\n";
    for(auto &e: elevInfo){
        std::cout<<"Elevator "<<e.id<<": floor="<<e.currentFloor
                 <<" faulted="<<(e.isFaulted?"YES":"NO")<<"\n";
    }
    std::cout<<"=========================\n";
}

static int parseElevID(const std::string &ts){
    auto pos = ts.rfind("(ElevID=");
    if(pos==std::string::npos) return -1;
    return std::stoi(ts.substr(pos+8));
}

static void floorThread(){
    while(!stopAll){
        std::string ip;
        int p;
        auto data = udpRecvString(floorSock, ip, p);
        if(data.empty()){
            simulateSleepMs(50);
            continue;
        }
        FloorRequest fr = deserializeRequest(data);
        {
            std::lock_guard<std::mutex> lk(mtxFloor);
            floorRequests.push(fr);
        }
        cvFloor.notify_all();
    }
}

static void elevatorThread(){
    while(!stopAll){
        std::string ip; 
        int p;
        auto msg = udpRecvString(elevatorSock, ip, p);
        if(msg.empty()){
            simulateSleepMs(50);
            continue;
        }
        if(msg.rfind("FAULT|", 0) == 0){
            std::istringstream iss(msg);
            std::string token;
            getline(iss, token, '|');
            getline(iss, token, '|');
            int eqPos = token.find('=');
            int eID   = std::stoi(token.substr(eqPos+1));
            getline(iss, token, '|');
            std::cout<<"[Scheduler] Elevator "<<eID
                     <<" fault => "<<token<<"\n";
            elevInfo[eID].isFaulted = true;
            continue;
        }

        FloorRequest comp = deserializeRequest(msg);
        {
            std::lock_guard<std::mutex> lk(mtxElev);
            elevatorCompletions.push(comp);
        }
        cvElev.notify_all();
    }
}

static int pickElevator(const FloorRequest &fr){
    int best=-1;
    int bestDist=999999;
    for(auto &e: elevInfo){
        if(e.isFaulted) continue;
        int dist = std::abs(e.currentFloor - fr.floor);
        if(dist < bestDist){
            bestDist=dist;
            best = e.id;
        }
    }
    return best;
}

// Usage: ./SchedulerProcess <schedulerPort> <floorPort> <numElevators>
int main(int argc,char* argv[]){
    if(argc < 4){
        std::cerr<<"Usage: "<<argv[0]
                 <<" <schedulerPort> <floorPort> <numElevators>\n";
        return 1;
    }
    schedPort    = std::stoi(argv[1]);
    floorPort    = std::stoi(argv[2]);
    numElevators = std::stoi(argv[3]);

    for(int i=0; i<numElevators; i++){
        ElevatorStatus es;
        es.id           = i;
        es.currentFloor = 1;
        es.isFaulted    = false;
        elevInfo.push_back(es);
    }

    floorSock    = createBoundSocket(schedPort);
    if(floorSock < 0) return 1;
    elevatorSock = createBoundSocket(schedPort + 100);
    if(elevatorSock < 0) return 1;
    sendSock     = socket(AF_INET, SOCK_DGRAM, 0);

    std::thread fth(floorThread);
    std::thread eth(elevatorThread);

    bool running = true;
    SchedulerState st = SchedulerState::WAIT_FOR_REQUEST;
    FloorRequest current;

    while(running){
        switch(st){
        case SchedulerState::WAIT_FOR_REQUEST:{
            std::unique_lock<std::mutex> lk(mtxFloor);
            cvFloor.wait(lk,[&]{return !floorRequests.empty() || stopAll;});
            if(stopAll && floorRequests.empty()){
                st = SchedulerState::TERMINATE;
                break;
            }
            current = floorRequests.front();
            floorRequests.pop();
            lk.unlock();

            std::cout<<"[Scheduler] Received floor request: "
                     << current.floor <<"->"<< current.destination
                     <<" fault="<<(int)current.hasFault<<"("
                     << current.faultType <<") pass="
                     << current.passengers <<"\n";

            st = SchedulerState::PROCESS_REQUEST;
            break;
        }
        case SchedulerState::PROCESS_REQUEST:{
            st = SchedulerState::SELECT_ELEVATOR;
            break;
        }
        case SchedulerState::SELECT_ELEVATOR:{
            int chosen = pickElevator(current);
            if(chosen < 0){
                // no elevator => discard
                std::cout<<"[Scheduler] No available elevator => discard request "
                         << current.floor <<"->"<< current.destination <<"\n";
                // ack floor anyway
                st = SchedulerState::NOTIFY_FLOOR;
                break;
            }
            std::cout<<"[Scheduler] Chose elevator "<<chosen<<"\n";
            current.timeStamp += "(ElevID=" + std::to_string(chosen) + ")";
            st = SchedulerState::SEND_TO_ELEVATOR;
            break;
        }
        case SchedulerState::SEND_TO_ELEVATOR:{
            int eID  = parseElevID(current.timeStamp);
            int ePort= 5000 + eID;
            auto msg = serializeRequest(current);
            udpSendString(sendSock, msg, "127.0.0.1", ePort);
            std::cout<<"[Scheduler] Sent request to Elevator "
                     << eID <<" on port "<< ePort <<"\n";
            st = SchedulerState::WAIT_FOR_ELEVATOR;
            break;
        }
        case SchedulerState::WAIT_FOR_ELEVATOR:{
            int eID = parseElevID(current.timeStamp);
            bool done = false;
            while(!done && !stopAll){
                {
                    std::unique_lock<std::mutex> lk(mtxElev);
                    // poll every 100ms
                    cvElev.wait_for(lk, std::chrono::milliseconds(100),
                                    [&]{return !elevatorCompletions.empty() || stopAll;});
                }
                if(stopAll){
                    st = SchedulerState::TERMINATE;
                    break;
                }
                // if elevator eID is now faulted => discard
                if(elevInfo[eID].isFaulted){
                    std::cout<<"[Scheduler] Elevator "<<eID
                             <<" is now faulted => discard this request\n";
                    st   = SchedulerState::NOTIFY_FLOOR;
                    done = true;
                    break;
                }
                // check if normal completion from eID arrived
                bool foundIt=false;
                std::unique_lock<std::mutex> lk2(mtxElev);
                size_t N = elevatorCompletions.size();
                for(size_t i=0; i<N; i++){
                    FloorRequest comp = elevatorCompletions.front();
                    elevatorCompletions.pop();
                    int cID = parseElevID(comp.timeStamp);
                    if(cID == eID){
                        std::cout<<"[Scheduler] Elevator done: floor="
                                 << comp.floor <<"->"<< comp.destination <<"\n";
                        elevInfo[eID].currentFloor = comp.destination;
                        printConsole();
                        current = comp;
                        st      = SchedulerState::NOTIFY_FLOOR;
                        foundIt = true;
                        done    = true;
                    } else {
                        elevatorCompletions.push(comp);
                    }
                }
                if(foundIt) break;
            }
            break;
        }
        case SchedulerState::NOTIFY_FLOOR:{
            auto ack = serializeRequest(current);
            udpSendString(sendSock, ack, "127.0.0.1", floorPort);
            std::cout<<"[Scheduler] Ack to Floor\n";
            st = SchedulerState::WAIT_FOR_REQUEST;
            break;
        }
        case SchedulerState::TERMINATE:{
            running = false;
            break;
        }
        }
    }

    stopAll = true;
    fth.join();
    eth.join();
    close(floorSock);
    close(elevatorSock);
    close(sendSock);

    std::cout<<"[Scheduler] Exiting...\n";
    return 0;
}
