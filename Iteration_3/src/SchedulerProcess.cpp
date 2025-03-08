#include "Common.h"
#include <vector>
#include <cmath>
#include <iostream>

struct ElevatorStatus{int id;int floor;bool stopped;};

int main(int argc,char* argv[]){
    if(argc<4)return 1;
    int schedPort=std::atoi(argv[1]);int floorPort=std::atoi(argv[2]);int elevCount=std::atoi(argv[3]);
    std::vector<std::pair<std::string,int>> elevEP;
    for(int i=0;i<elevCount;i++){
        elevEP.push_back({"127.0.0.1",5000+i});
        std::cout<<"[Scheduler] Elevator "<<i<<" at 127.0.0.1:"<<(5000+i)<<"\n";
    }
    int sock=createBoundSocket(schedPort);
    std::vector<ElevatorStatus> est;est.reserve(elevCount);
    for(int i=0;i<elevCount;i++){ElevatorStatus s{i,1,false};est.push_back(s);}
    auto pick=[&](FloorRequest &r){int best=-1,d=99999;for(auto &st:est){if(st.stopped)continue;int dist=std::abs(st.floor-r.floor);if(dist<d){d=dist;best=st.id;}}return best<0?0:best;};
    while(true){
        std::string ip;int p;
        auto msg=udpRecvString(sock,ip,p);
        if(msg.empty()){simulateSleepMs(200);continue;}
        auto fr=deserializeRequest(msg);
        std::cout<<"[Scheduler] Request Floor="<<fr.floor<<"->"<<fr.destination<<"\n";
        int c=pick(fr);
        std::cout<<"[Scheduler] Using Elevator "<<c<<"\n";
        udpSendString(sock,serializeRequest(fr),elevEP[c].first,elevEP[c].second);
        bool done=false;
        while(!done){
            std::string eip;int ep;
            auto ed=udpRecvString(sock,eip,ep);
            if(ed.empty()){simulateSleepMs(100);continue;}
            auto ec=deserializeElevatorComplete(ed);
            if(ec.request.floor==fr.floor && ec.request.destination==fr.destination){
                std::cout<<"[Scheduler] Elevator "<<ec.elevatorID<<" done.\n";
                est[ec.elevatorID].floor=ec.request.destination;
                udpSendString(sock,serializeRequest(fr),ip,p);
                done=true;
            } else std::cout<<"[Scheduler] Got other elevator complete.\n";
        }
    }
    close(sock);return 0;
}
