#include "Common.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <vector>
#include <unistd.h>

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
    if(sock<0) return 1;

    // Read requests
    std::ifstream in(file);
    if(!in){
        std::cerr<<"[Floor] Could not open "<<file<<"\n";
        return 1;
    }
    std::vector<FloorRequest> requests;
    {
        std::string line;
        while(std::getline(in, line)){
            if(line.empty()) continue;
            FloorRequest fr;
            std::istringstream iss(line);
            iss >> fr.timeStamp >> fr.floor >> fr.direction
                >> fr.destination >> std::boolalpha
                >> fr.hasFault >> fr.faultType >> fr.passengers;
            
                // Checking for malfromed lines
            if (iss.fail()) {
                std::cerr << "Malformed line: " << line << "\n";
                continue;
            }

            requests.push_back(fr);
        }
    }
    in.close();

    int requestCounter = 0;
    // Send each request to Scheduler
    for(auto &r: requests){
        r.requestID = requestCounter++; // Assinging unique reqeuest ID
        std::string msg = serializeRequest(r);
        udpSendString(sock, msg, schedIP, schedPort);
        udpSendString(sock, msg, "127.0.0.1", 6001);  // Send copy to UI
        //std::cout<<"[Floor] Sent request floor="<<r.floor
        //        <<" -> "<<r.destination<<"\n";
        simulateSleepMs(500);
    }

    // We could listen for acks here, but for now just idle
    while(true){
        std::string ip; 
        int rp;
        auto data = udpRecvString(sock, ip, rp);
        if(!data.empty()){
            //std::cout<<"[Floor] Acknowledgment: "<<data<<"\n";
        }
        simulateSleepMs(500);
    }
    close(sock);
    return 0;
}
