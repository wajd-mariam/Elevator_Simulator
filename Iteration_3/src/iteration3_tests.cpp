#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include "Common.h"

TEST(Iteration3_Serialization, FloorRequest_SerializeDeserialize){
    FloorRequest f("12:34",2,"Up",5);
    std::string s=serializeRequest(f);
    FloorRequest r=deserializeRequest(s);
    EXPECT_EQ(r.timeStamp,"12:34");
    EXPECT_EQ(r.floor,2);
    EXPECT_EQ(r.direction,"Up");
    EXPECT_EQ(r.destination,5);
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
    // Missing delimiters or invalid numeric fields
    std::string invalid1 = "abc"; // no '|'
    std::string invalid2 = "Time|NotANum|Up|4";
    EXPECT_THROW({deserializeRequest(invalid1);}, std::exception);
    EXPECT_THROW({deserializeRequest(invalid2);}, std::exception);
}

// Test a scheduling function
struct EStatus{int id;int floor;bool stopped;};
int pickElevator(const FloorRequest &req,const std::vector<EStatus> &sts){
    int best=-1;int d=9999;
    for(auto &e:sts){
        if(e.stopped) continue;
        int dist=abs(e.floor-req.floor);
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

// Tie-break test
TEST(Iteration3_SchedulerLogic, TieBreak){
    std::vector<EStatus> v={{0,5,false},{1,5,false},{2,10,false}};
    FloorRequest req("xx",5,"Down",1); // distance=0 for elev0,1
    EXPECT_EQ(pickElevator(req,v),0);  // smaller ID
}

// Input file tests
TEST(Iteration3_InputFile, HasEntries){
    std::ifstream in("input.txt");
    ASSERT_TRUE(in.good());
    int count=0;std::string line;
    while(std::getline(in,line)) if(!line.empty()) count++;
    in.close();
    EXPECT_GT(count,0);
}

// Non-existent file
TEST(Iteration3_InputFile, NonExistent){
    std::ifstream in("no_such_file.txt");
    EXPECT_FALSE(in.good());
}

int main(int argc,char** argv){
    testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
