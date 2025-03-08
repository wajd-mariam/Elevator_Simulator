#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

struct FloorRequest {
    std::string timeStamp;
    int floor;
    std::string direction;
    int destination;
    FloorRequest() : floor(0), destination(0) {}
    FloorRequest(const std::string &t,int f,const std::string &dir,int d)
        : timeStamp(t),floor(f),direction(dir),destination(d) {}
};

inline std::string serializeRequest(const FloorRequest &fr) {
    return fr.timeStamp + "|" + std::to_string(fr.floor) + "|" + fr.direction + "|" + std::to_string(fr.destination);
}
inline FloorRequest deserializeRequest(const std::string &s) {
    auto p1=s.find("|"); auto p2=s.find("|",p1+1); auto p3=s.find("|",p2+1);
    if(p1==std::string::npos || p2==std::string::npos || p3==std::string::npos)
        throw std::runtime_error("Malformed request");
    FloorRequest fr;
    fr.timeStamp=s.substr(0,p1);
    fr.floor=std::stoi(s.substr(p1+1,p2-(p1+1)));
    fr.direction=s.substr(p2+1,p3-(p2+1));
    fr.destination=std::stoi(s.substr(p3+1));
    return fr;
}

struct ElevatorComplete {
    int elevatorID;
    FloorRequest request;
};

inline std::string serializeElevatorComplete(const ElevatorComplete &ec) {
    return std::to_string(ec.elevatorID)+"|"+serializeRequest(ec.request);
}
inline ElevatorComplete deserializeElevatorComplete(const std::string &s) {
    ElevatorComplete ec;
    auto p=s.find("|");
    ec.elevatorID=std::stoi(s.substr(0,p));
    ec.request=deserializeRequest(s.substr(p+1));
    return ec;
}

inline int createBoundSocket(int port) {
    int sock=socket(AF_INET,SOCK_DGRAM,0);
    sockaddr_in addr;memset(&addr,0,sizeof(addr));
    addr.sin_family=AF_INET;addr.sin_addr.s_addr=INADDR_ANY;addr.sin_port=htons(port);
    bind(sock,(struct sockaddr*)&addr,sizeof(addr));
    return sock;
}
inline void udpSendString(int sock,const std::string &msg,const std::string &ip,int port) {
    sockaddr_in dest;memset(&dest,0,sizeof(dest));
    dest.sin_family=AF_INET;dest.sin_port=htons(port);inet_pton(AF_INET,ip.c_str(),&dest.sin_addr);
    sendto(sock,msg.c_str(),msg.size(),0,(struct sockaddr*)&dest,sizeof(dest));
}
inline std::string udpRecvString(int sock,std::string &rip,int &rport) {
    char buf[1024];sockaddr_in src;socklen_t slen=sizeof(src);
    int len=recvfrom(sock,buf,sizeof(buf)-1,0,(struct sockaddr*)&src,&slen);
    if(len<0)return "";
    buf[len]='\0';
    char ip[INET_ADDRSTRLEN];inet_ntop(AF_INET,&src.sin_addr,ip,sizeof(ip));
    rip=ip;rport=ntohs(src.sin_port);
    return std::string(buf);
}
inline void simulateSleepMs(int ms){std::this_thread::sleep_for(std::chrono::milliseconds(ms));}

#endif
