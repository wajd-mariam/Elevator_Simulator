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
            std::string t, dir, ft;
            bool hf;
            int f, d, pass;
            iss >> t >> f >> dir >> d >> std::boolalpha >> hf >> ft >> pass;
            FloorRequest fr(t, f, dir, d, hf, ft, pass);
            requests.push_back(fr);
        }
    }
    in.close();

    std::cout << "[Floor] Loaded " << requests.size()
              << " requests from " << file << "\n";

    // send each request => wait ack
    for(auto &r: requests){
        auto msg = serializeRequest(r);
        std::cout << "[Floor] Sending request " << msg
                  << " to " << schedIP << ":" << schedPort << "\n";
        udpSendString(sock, msg, schedIP, schedPort);

        std::string rip; 
        int rp;
        auto ack = udpRecvString(sock, rip, rp);
        if(!ack.empty()){
            auto ackReq = deserializeRequest(ack);
            std::cout << "[Floor] Acknowledgment Floor "
                      << ackReq.floor << "->" << ackReq.destination << "\n\n";
        }
    }

    std::cout<<"[Floor] Done sending all. Listening...\n";
    while(true){
        std::string rip;
        int rp;
        auto data = udpRecvString(sock, rip, rp);
        if(!data.empty()){
            auto rr = deserializeRequest(data);
            std::cout<<"[Floor] Late ack: Floor "
                     << rr.floor <<"->"<< rr.destination <<"\n";
        }
        simulateSleepMs(500);
    }

    close(sock);
    return 0;
}
