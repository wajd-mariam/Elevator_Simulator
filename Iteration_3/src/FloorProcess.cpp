#include "Common.h"
#include <fstream>
#include <sstream>

int main(int argc,char* argv[]){
    if(argc<5)return 1;
    int myPort=std::atoi(argv[1]);std::string schedIP=argv[2];int schedPort=std::atoi(argv[3]);std::string fname=argv[4];
    int sock=createBoundSocket(myPort);
    std::ifstream in(fname);if(!in)return 1;
    std::string line;std::vector<FloorRequest> reqs;
    while(std::getline(in,line)){if(line.empty())continue;std::istringstream ss(line);std::string t,dir;int f,d;ss>>t>>f>>dir>>d;reqs.emplace_back(t,f,dir,d);}
    in.close();
    for(auto &r:reqs){
        std::string s=serializeRequest(r);
        std::cout<<"[Floor] Sending request "<<r.timeStamp<<" Floor="<<r.floor<<" "<<r.direction<<"->"<<r.destination<<"\n";
        udpSendString(sock,s,schedIP,schedPort);
        std::string ip;int p;
        auto ack=udpRecvString(sock,ip,p);
        if(!ack.empty()){
            auto done=deserializeRequest(ack);
            std::cout<<"[Floor] Acknowledgment Floor "<<done.floor<<"->"<<done.destination<<"\n\n";
        }
    }
    std::cout<<"[Floor] Done sending requests. Listening for extra.\n";
    while(true){
        std::string ip;int p;auto a=udpRecvString(sock,ip,p);if(!a.empty()){
            auto dr=deserializeRequest(a);
            std::cout<<"[Floor] Late Ack Floor "<<dr.floor<<"->"<<dr.destination<<"\n";
        }
        simulateSleepMs(500);
    }
    close(sock);return 0;
}
