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

// Global shared elevator status:
struct ElevatorStatus {
  int id;
  int currentFloor;
  bool doorsOpen;
  bool isFaulted;
  std::string state;
};

extern std::vector<ElevatorStatus> globalElevatorStatus;
extern std::mutex globalElevatorMutex;

// A single floor request
struct FloorRequest {
    std::string timeStamp;
    int floor;
    std::string direction;
    int destination;
    bool hasFault;
    std::string faultType;
    int passengers;

    FloorRequest()
      : floor(0),
        destination(0),
        hasFault(false),
        faultType("noFault"),
        passengers(1){}

    FloorRequest(const std::string &t,int f,
                 const std::string &dir,int d,
                 bool hf=false,const std::string &ft="noFault",
                 int p=1)
      : timeStamp(t),floor(f),direction(dir),
        destination(d),hasFault(hf),faultType(ft),passengers(p){}
};

// Store 7 fields as: "time|floor|direction|destination|1/0|faultType|passengers"
std::string serializeRequest(const FloorRequest &fr);
FloorRequest deserializeRequest(const std::string &s);

int createBoundSocket(int port);
void udpSendString(int sock,const std::string &msg,const std::string &destIP,int destPort);
std::string udpRecvString(int sock,std::string &sourceIP,int &sourcePort);

void simulateSleepMs(int ms);

#endif
