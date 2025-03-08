#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>
#include <iostream>
#include <chrono>
#include <thread>
#include <stdexcept>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <cmath>
#include <fstream>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>

// A single floor request
struct FloorRequest {
    std::string timeStamp;
    int floor;
    std::string direction;
    int destination;
    FloorRequest():floor(0),destination(0){}
    FloorRequest(const std::string &t,int f,const std::string &dir,int d)
       :timeStamp(t),floor(f),direction(dir),destination(d){}
};

std::string serializeRequest(const FloorRequest &fr);
FloorRequest deserializeRequest(const std::string &s);

int createBoundSocket(int port);
void udpSendString(int sock,const std::string &msg,const std::string &destIP,int destPort);
std::string udpRecvString(int sock,std::string &sourceIP,int &sourcePort);

void simulateSleepMs(int ms);

#endif
