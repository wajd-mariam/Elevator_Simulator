#include "Common.h"
#include <cmath>
#include <iostream>

int main(int argc,char* argv[]){
    if(argc<5)return 1;
    int myPort=std::atoi(argv[1]);int myID=std::atoi(argv[2]);std::string schedIP=argv[3];int schedPort=std::atoi(argv[4]);
    int sock=createBoundSocket(myPort);
    int currentFloor=1;bool doorsOpen=true;
    std::cout<<"[Elevator "<<myID<<"] Started on port "<<myPort<<"\n";
    while(true){
        std::string ip;int p;auto d=udpRecvString(sock,ip,p);
        if(d.empty()){simulateSleepMs(100);continue;}
        auto fr=deserializeRequest(d);
        if(doorsOpen){doorsOpen=false;simulateSleepMs(300);}
        int pd=std::abs(fr.floor-currentFloor);if(pd>0){simulateSleepMs(pd*500);currentFloor=fr.floor;}
        doorsOpen=true;simulateSleepMs(300);doorsOpen=false;simulateSleepMs(300);
        int dd=std::abs(fr.destination-currentFloor);if(dd>0){simulateSleepMs(dd*500);currentFloor=fr.destination;}
        doorsOpen=true;simulateSleepMs(300);
        ElevatorComplete ec{myID,fr};
        udpSendString(sock,serializeElevatorComplete(ec),schedIP,schedPort);
        std::cout<<"[Elevator "<<myID<<"] Floor "<<currentFloor<<" done\n";
    }
    close(sock);return 0;
}
