#include "Common.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <vector>
#include <unistd.h>

// Entry point for the Floor process
int main(int argc,char* argv[]){
    // Usage: ./FloorProcess <myPort> <schedulerIP> <schedulerPort> input.txt
    if(argc<5){
        std::cerr<<"Usage: "<<argv[0]<<" <myPort> <schedulerIP> <schedulerPort> <inputFile>\n";
        return 1;
    }

    // Parse command-line arguments
    int myPort=std::stoi(argv[1]);
    std::string schedIP=argv[2];
    int schedPort=std::stoi(argv[3]);
    std::string file=argv[4];

    // Create UDP socket bound to myPort
    int sock=createBoundSocket(myPort);
    if(sock<0) return 1;

    // Read floor requests from the input file
    std::ifstream in(file);
    if(!in){
        std::cerr<<"[Floor] Could not open "<<file<<"\n";
        return 1;
    }

    // Creating a vector array of FloorRequest objects
    std::vector<FloorRequest> requests;
    {
        std::string line;
        while(std::getline(in, line)){
            if(line.empty()) continue;
            FloorRequest fr;
            std::istringstream iss(line);

            // Input format: timestamp floor direction destination hasFault faultType passengers
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
    // Send each request to Scheduler process
    for(auto &r: requests){
        r.requestID = requestCounter++; // Assinging unique reqeuest ID
        std::string msg = serializeRequest(r);

        // Send request to the scheduler
        udpSendString(sock, msg, schedIP, schedPort);

        // Also send a copy to the UI for live tracking
        udpSendString(sock, msg, "127.0.0.1", 6001);  // Send copy to UI

        simulateSleepMs(500);
    }

    // Remain active to optionally receive acknowledgments
    while(true){
        std::string ip; 
        int rp;
        auto data = udpRecvString(sock, ip, rp);
        simulateSleepMs(500);
    }
    close(sock);
    return 0;
}
