#include "Common.h"

std::vector<ElevatorStatus> globalElevatorStatus;
std::mutex globalElevatorMutex;

std::string serializeRequest(const FloorRequest &fr){
    std::string faultBit = (fr.hasFault ? "1" : "0");
    return fr.timeStamp + "|"
         + std::to_string(fr.floor) + "|"
         + fr.direction + "|"
         + std::to_string(fr.destination) + "|"
         + faultBit + "|"
         + fr.faultType + "|"
         + std::to_string(fr.passengers);
}

FloorRequest deserializeRequest(const std::string &s){
    auto p1 = s.find("|");
    auto p2 = s.find("|", p1 + 1);
    auto p3 = s.find("|", p2 + 1);
    auto p4 = s.find("|", p3 + 1);
    auto p5 = s.find("|", p4 + 1);
    auto p6 = s.find("|", p5 + 1);
    if(p1==std::string::npos || p2==std::string::npos || p3==std::string::npos ||
       p4==std::string::npos || p5==std::string::npos || p6==std::string::npos)
    {
        throw std::runtime_error("Malformed request: " + s);
    }
    FloorRequest fr;
    fr.timeStamp   = s.substr(0, p1);
    fr.floor       = std::stoi(s.substr(p1 + 1, p2 - (p1 + 1)));
    fr.direction   = s.substr(p2 + 1, p3 - (p2 + 1));
    fr.destination = std::stoi(s.substr(p3 + 1, p4 - (p3 + 1)));
    fr.hasFault    = (s.substr(p4 + 1, p5 - (p4 + 1)) == "1");
    fr.faultType   = s.substr(p5 + 1, p6 - (p5 + 1));
    fr.passengers  = std::stoi(s.substr(p6 + 1));
    return fr;
}

int createBoundSocket(int port){
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0){
        std::cerr << "socket() failed\n";
        return -1;
    }
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(port);

    if(bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0){
        std::cerr << "bind failed on port " << port << "\n";
        ::close(sock);
        return -1;
    }
    return sock;
}

void udpSendString(int sock,const std::string &msg,const std::string &ip,int port){
    sockaddr_in dest;
    memset(&dest,0,sizeof(dest));
    dest.sin_family=AF_INET;
    dest.sin_port=htons(port);
    inet_pton(AF_INET,ip.c_str(),&dest.sin_addr);
    sendto(sock, msg.c_str(), msg.size(), 0, (struct sockaddr*)&dest, sizeof(dest));
}

std::string udpRecvString(int sock,std::string &rip,int &rport){
    char buf[1024];
    sockaddr_in src; 
    socklen_t srclen=sizeof(src);
    int len = recvfrom(sock, buf, sizeof(buf)-1, 0, (struct sockaddr*)&src, &srclen);
    if(len < 0) return "";
    buf[len] = '\0';
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &src.sin_addr, ip, sizeof(ip));
    rip  = ip;
    rport= ntohs(src.sin_port);
    return std::string(buf);
}

void simulateSleepMs(int ms){
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
