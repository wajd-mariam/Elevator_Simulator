#include <gtest/gtest.h>
#include "Common.h"

// ---------------------------------------------------------------------
// Basic tests
// ---------------------------------------------------------------------

TEST(Iteration3_Serialization, FloorRequest_SerializeDeserialize){
    FloorRequest f("12:34",2,"Up",5);
    std::string s=serializeRequest(f);
    FloorRequest r=deserializeRequest(s);
    EXPECT_EQ(r.timeStamp,"12:34");
    EXPECT_EQ(r.floor,2);
    EXPECT_EQ(r.direction,"Up");
    EXPECT_EQ(r.destination,5);
}

// Just a demonstration of a possible “ElevatorComplete” struct:
struct ElevatorComplete {
    int elevatorID;
    FloorRequest request;
};
std::string serializeElevatorComplete(const ElevatorComplete &ec){
    return std::to_string(ec.elevatorID)+"|"+serializeRequest(ec.request);
}
ElevatorComplete deserializeElevatorComplete(const std::string &s){
    auto p=s.find("|");
    ElevatorComplete ec;
    ec.elevatorID=std::stoi(s.substr(0,p));
    ec.request=deserializeRequest(s.substr(p+1));
    return ec;
}

TEST(Iteration3_Serialization, ElevatorComplete_SerializeDeserialize){
    ElevatorComplete ec{1,FloorRequest("06:12",3,"Down",20)};
    std::string s=serializeElevatorComplete(ec);
    ElevatorComplete x=deserializeElevatorComplete(s);
    EXPECT_EQ(x.elevatorID,1);
    EXPECT_EQ(x.request.timeStamp,"06:12");
    EXPECT_EQ(x.request.floor,3);
    EXPECT_EQ(x.request.direction,"Down");
    EXPECT_EQ(x.request.destination,20);
}

TEST(Iteration3_Serialization, MalformedStrings){
    std::string invalid1="abc";
    std::string invalid2="Time|NotANum|Up|4";
    EXPECT_THROW({deserializeRequest(invalid1);}, std::exception);
    EXPECT_THROW({deserializeRequest(invalid2);}, std::exception);
}

// Simple scheduling pick function
struct EStatus{int id;int floor;bool stopped;};
int pickElevator(const FloorRequest &req,const std::vector<EStatus> &sts){
    int best=-1;int d=9999;
    for(auto &e:sts){
        if(e.stopped) continue;
        int dist=std::abs(e.floor-req.floor);
        if(dist<d){d=dist;best=e.id;}
        else if(dist==d && e.id<best){
            best=e.id;
        }
    }
    if(best<0 && !sts.empty()) best=sts[0].id;
    return best;
}

TEST(Iteration3_SchedulerLogic, PickNearestElevator){
    std::vector<EStatus> v={{0,1,false},{1,10,false},{2,20,false}};
    FloorRequest req("xx",9,"Up",12);
    EXPECT_EQ(pickElevator(req,v),1);
}

TEST(Iteration3_SchedulerLogic, AllStoppedButOne){
    std::vector<EStatus> v={{0,5,false},{1,6,true},{2,7,true}};
    FloorRequest req("xx",20,"Down",1);
    EXPECT_EQ(pickElevator(req,v),0);
}

TEST(Iteration3_SchedulerLogic, TieBreak){
    std::vector<EStatus> v={{0,5,false},{1,5,false},{2,10,false}};
    FloorRequest req("xx",5,"Down",1);
    EXPECT_EQ(pickElevator(req,v),0);
}

// check input file
TEST(Iteration3_InputFile, HasEntries){
    std::ifstream in("input.txt");
    ASSERT_TRUE(in.good());
    int count=0;std::string line;
    while(std::getline(in,line)) if(!line.empty()) count++;
    in.close();
    EXPECT_GT(count,0);
}
TEST(Iteration3_InputFile, NonExistent){
    std::ifstream in("no_such_file.txt");
    EXPECT_FALSE(in.good());
}

int main(int argc,char** argv){
    ::testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
