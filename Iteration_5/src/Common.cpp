#include "Common.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <chrono>
#include <thread>

int createBoundSocket(int port){
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock<0){
        std::cerr<<"[Common] socket() failed\n";
        return -1;
    }
    sockaddr_in addr; 
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if(bind(sock,(struct sockaddr*)&addr,sizeof(addr))<0){
        std::cerr<<"[Common] bind() failed on port "<<port<<"\n";
        close(sock);
        return -1;
    }
    return sock;
}

void udpSendString(int sock, const std::string &msg, const std::string &ip, int port){
    sockaddr_in dest;
    memset(&dest,0,sizeof(dest));
    dest.sin_family=AF_INET;
    dest.sin_port=htons(port);
    inet_pton(AF_INET, ip.c_str(), &dest.sin_addr);
    sendto(sock, msg.c_str(), msg.size(), 0, (struct sockaddr*)&dest, sizeof(dest));
}

std::string udpRecvString(int sock, std::string &senderIP, int &senderPort){
    char buf[1024];
    sockaddr_in src; 
    socklen_t srclen=sizeof(src);
    int len=recvfrom(sock, buf, sizeof(buf)-1, 0, (struct sockaddr*)&src, &srclen);
    if(len<0) return "";
    buf[len]='\0';
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &src.sin_addr, ip, sizeof(ip));
    senderIP=ip;
    senderPort=ntohs(src.sin_port);
    return std::string(buf);
}

// FloorRequest <-> string
std::string serializeRequest(const FloorRequest &fr){
    std::ostringstream oss;
    oss << fr.timeStamp << " " << fr.floor << " " << fr.direction << " "
        << fr.destination << " "  << fr.faultType << " " << fr.passengers << " " << fr.requestID;
    return oss.str();
}

FloorRequest deserializeRequest(const std::string &s){
    FloorRequest fr;
    std::istringstream iss(s);
    iss >> fr.timeStamp >> fr.floor >> fr.direction
        >> fr.destination >> fr.faultType >> fr.passengers >> fr.requestID; 
    return fr;
}

// ElevatorStatus <-> string
std::string serializeElevatorStatus(const ElevatorStatus &st){
    // id floor doorsOpen isFaulted state
    std::ostringstream oss;
    oss << st.id <<" "<< st.currentFloor <<" "
        << (st.doorsOpen?"1":"0") <<" "
        << (st.isFaulted?"1":"0") <<" "
        << st.direction << " "
        << st.state << " " 
        << st.faultType;
    return oss.str();
}

ElevatorStatus deserializeElevatorStatus(const std::string &s){
    //ElevatorStatus es;
    /**
    std::istringstream iss(s);
    int doorFlag, faultFlag;
    iss >> es.id >> es.currentFloor >> doorFlag >> faultFlag >> es.state >> es.direction >> es.currentRequestID;
    es.doorsOpen = (doorFlag==1);
    es.isFaulted = (faultFlag==1);
    return es;  */

    ElevatorStatus es;
    std::istringstream iss(s);
    std::string token;
    
    while (std::getline(iss, token, '|')) {
        if (token.find("ElevID=") == 0)
            es.id = std::stoi(token.substr(7));
        else if (token.find("Floor=") == 0)
            es.currentFloor = std::stoi(token.substr(6));
        else if (token.find("Fault=") == 0)
            es.isFaulted = std::stoi(token.substr(6)) != 0;
        else if (token.find("Direction=") == 0)
            es.direction = token.substr(10);
        else if (token.find("State=") == 0)
            es.state = token.substr(6);
        else if (token.find("FaultType=") == 0)
            es.faultType = token.substr(10);  //  parses "doorStuck", "noFault", etc.
    }

    return es;
}

void simulateSleepMs(int ms){
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
