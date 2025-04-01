#include "Common.h"

int main(int argc,char* argv[]){
    // Parse arguments
    // Expected arguments: ./FloorProcess <myPort> <schedulerIP> <schedulerPort> input.txt
    if(argc<5){
        std::cerr<<"Usage: "<<argv[0]<<" <myPort> <schedulerIP> <schedulerPort> <inputFile>\n";
        return 1;
    }
    std::cout << "STarting floor process." <<std::endl;
    // Reading command line arguments
    int myPort=std::stoi(argv[1]);
    std::string schedIP=argv[2];
    int schedPort=std::stoi(argv[3]);
    std::string file=argv[4];

    // Creating and binding UPD socket
    int sock=createBoundSocket(myPort);
    if(sock<0)return 1;

    // Open input file
    std::ifstream in(file);
    if(!in){
        std::cerr<<"Could not open file "<<file<<"\n";
        return 1;
    }

    // Parsing each line from input.txt as FloorRequest struct:
    // Each line in input.txt is expected to contain:
    // <timeStamp> <floor> <direction> <destination> <hasFault> <faultType> <passengers>
    std::vector<FloorRequest> requests;
    {
        std::string line;
        while(std::getline(in,line)){
            if(line.empty()) continue;
            std::istringstream iss(line);
            std::string t, dir, ft; // time, direction, fault-type
            bool hf; // has-faults
            int f, d, pass; // floor, destination, passengers
            iss >> t >> f >> dir >> d >> std::boolalpha >> hf >> ft >> pass;
            FloorRequest fr(t, f, dir, d, hf, ft, pass);
            requests.push_back(fr);
        }
    }
    in.close();

    //std::cout << "[Floor] Loaded " << requests.size()
             // << " requests from " << file << "\n";

    // send each request to scheduler => wait ack
    for(auto &r: requests){
        auto msg = serializeRequest(r);
        //std::cout << "[Floor] Sending request " << msg
                 // << " to " << schedIP << ":" << schedPort << "\n";
        udpSendString(sock, msg, schedIP, schedPort);

        // Sending each serialzied FloorRequest via UDP to Scheduler
        std::string rip; 
        int rp;
        auto ack = udpRecvString(sock, rip, rp);
        if(!ack.empty()){
            auto ackReq = deserializeRequest(ack);
           // std::cout << "[Floor] Acknowledgment Floor "
                      //<< ackReq.floor << "->" << ackReq.destination << "\n\n";
        }
    }
    

    // Passive listening for late ACK's
    //std::cout<<"[Floor] Done sending all. Listening...\n";
    while(true){
        std::string rip;
        int rp;
        auto data = udpRecvString(sock, rip, rp);
        if(!data.empty()){
            auto rr = deserializeRequest(data);
            //std::cout<<"[Floor] Late ack: Floor "
                    // << rr.floor <<"->"<< rr.destination <<"\n";
        }
        simulateSleepMs(500);
    }

    // Cleanup
    close(sock);
    return 0;
}
