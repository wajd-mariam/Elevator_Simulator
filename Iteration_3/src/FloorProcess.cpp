#include "Common.h"

int main(int argc,char* argv[]){
    // Usage: ./FloorProcess <myPort> <schedulerIP> <schedulerPort> input.txt
    if(argc<5){
        std::cerr<<"Usage: "<<argv[0]<<" <myPort> <schedulerIP> <schedulerPort> <inputFile>\n";
        return 1;
    }
    int myPort=std::stoi(argv[1]);
    std::string schedIP=argv[2];
    int schedPort=std::stoi(argv[3]);
    std::string file=argv[4];

    int sock=createBoundSocket(myPort);
    if(sock<0)return 1;

    std::ifstream in(file);
    if(!in){
        std::cerr<<"Could not open file "<<file<<"\n";
        return 1;
    }
    std::vector<FloorRequest> requests;
    {
        std::string line;
        while(std::getline(in,line)){
            if(line.empty()) continue;
            std::istringstream iss(line);
            std::string t,dir; int f,d;
            iss>>t>>f>>dir>>d;
            requests.emplace_back(t,f,dir,d);
        }
    }
    in.close();
    std::cout<<"[Floor] Loaded "<<requests.size()<<" requests from "<<file<<"\n";

    for(auto &r:requests){
        auto msg=serializeRequest(r);
        std::cout<<"[Floor] Sending request "<<msg<<" to "<<schedIP<<":"<<schedPort<<"\n";
        udpSendString(sock,msg,schedIP,schedPort);

        std::string ip; int p;
        auto ack=udpRecvString(sock,ip,p);
        if(!ack.empty()){
            auto ackReq=deserializeRequest(ack);
            std::cout<<"[Floor] Acknowledgment Floor "
                     <<ackReq.floor<<"->"<<ackReq.destination<<"\n\n";
        }
    }
    std::cout<<"[Floor] Done sending requests. Listening for extras.\n";

    while(true){
        std::string ip; int p;
        auto s=udpRecvString(sock,ip,p);
        if(!s.empty()){
            auto fr=deserializeRequest(s);
            std::cout<<"[Floor] Late ack: Floor "
                     <<fr.floor<<"->"<<fr.destination<<"\n";
        }
        simulateSleepMs(500);
    }

    close(sock);
    return 0;
}
