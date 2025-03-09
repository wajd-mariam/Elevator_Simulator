/* FloorProcess.cpp - Handles sending floor requests to the scheduler */
#include "Common.h"

int main(int argc,char* argv[]){
    // Usage: ./FloorProcess <myPort> <schedulerIP> <schedulerPort> input.txt
    // Ensure correct usage format
    if(argc<5){
        std::cerr<<"Usage: "<<argv[0]<<" <myPort> <schedulerIP> <schedulerPort> <inputFile>\n";
        return 1;
    }
    int myPort=std::stoi(argv[1]); // Port this process listens on
    std::string schedIP=argv[2];   // IP address of the scheduler
    int schedPort=std::stoi(argv[3]); // Port the scheduler listens on
    std::string file=argv[4];      // Input file containing floor requests

    // Create a UDP socket bound to the specified port
    int sock=createBoundSocket(myPort);
    if(sock<0)return 1;

    // Open the input file containing floor requests
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

    // Send each request to the scheduler via UDP
    for(auto &r:requests){
        auto msg=serializeRequest(r);
        std::cout<<"[Floor] Sending request "<<msg<<" to "<<schedIP<<":"<<schedPort<<"\n";
        udpSendString(sock,msg,schedIP,schedPort);

        // Wait for acknowledgment from the scheduler
        std::string ip; int p;
        auto ack=udpRecvString(sock,ip,p);
        if(!ack.empty()){
            auto ackReq=deserializeRequest(ack);
            std::cout<<"[Floor] Acknowledgment Floor "
                     <<ackReq.floor<<"->"<<ackReq.destination<<"\n\n";
        }
    }
    std::cout<<"[Floor] Done sending requests. Listening for extras.\n";

    // Keep listening for late acknowledgments from the scheduler
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

    close(sock); // Close the socket before exiting
    return 0;
}
