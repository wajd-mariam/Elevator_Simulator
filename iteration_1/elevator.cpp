// What does the elevator need to do?
// needs to go up and down floors
// needs to have buttons that tell the system which floors to go to
// needs a door and motor to let people in/get people out (static variable)
#include "elevator.h"

using namespace std;

const int maxFloor = 20;

struct FloorRequest {
    int floor;
    string direction;
    int destination;
};

class Scheduler
{
    private:
        std::mutex mtx;
        std::condition_variable wait;
     
        // The data stored in a vector
        std::deque<std::vector<FloorRequest>> floorRequests;
        string elevatorStatus;


    public:
        // Constructor
        Scheduler() : mtx(), wait() {}


        // Method for floor (Wajd) to invoke
        void receiveFromFloor(const std::vector<FloorRequest>& request)
        {
            std::unique_lock<std::mutex> lock(mtx);
            floorRequests.push_back(request);
            wait.notify_all();    
        }


        std::vector<FloorRequest> getRequest ()
        {
            std::unique_lock<std::mutex> lock(mtx);
            // As the elevator (Azan) keep making requests, put
            // elevator in waiting zone till new data received from Floor
            while(floorRequests.empty()) {
                wait.wait(lock);  // Wait for request to arrive
            }
        }


        // Elevator (Azan), use this method to update
        // current status, atm use stuff like job completed
        void receiveFromElevator(int status)
        {
            std::unique_lock<std::mutex> lock(mtx);
            elevatorStatus.push_back(status);
        }


        // Floor (Wajd), use this method for elevator status
        int getElevatorStatus() {
            std::unique_lock<std::mutex> lock(mtx);
        } 

       FloorRequest getFloor(){
        
       }

       void wait(){

       } 
};


class OperateElevator{
    private:
        bool elevatorMoving; //says when elevator
                            //indicates when a new task can be inputted
                            //can be inputted when not moving essentially (will be patched late4r)
        bool doorsOpen; // for later use
        int floorAt; // indicates the floor number the elevator is located on
                    // used to tell if its going in the proper direction 
        int floorGoingTo; // loaded in by scheduler
        deque<vector<FloorRequest>> requests;
        mutex mtx;
        condition_variable waitE;

    public:
    OperateElevator(): elevatorMoving(false), doorsOpen(false), floorAt(0), floorGoingTo(0), mtx(), waitE() {}

    void loadCommand(vector<FloorRequest> request){ // called by scheduler to load in the place for elevator to go next
        requests.push_back(request); 
        waitE.notify_all();
    }
    void goToFloor(){
        // using the time found in learning phase, we use it to calculate the amount of time
        elevatorMoving = true; // make sure doors are closed and elevator is moving
        if(doorsOpen == true){
            closeDoors();
        }
        
        cout << "Doors are closing and the elevator is moving" << endl; // indicator on how far we made it
        //This is where we access the scheduler to bring us to a floor
        FloorRequest& req = requests[0].front();
        floorGoingTo = req.destination;
        

       // if(floorAt != floorGoingTo){ // this is meant for later when the elevator is more useful
            //using the times we found, we can make the elevator go to the floor
       // }

        // should also make a function that stops the elevator at floors if a person pushes a button later

        //from here, we reset the elevator
        openDoors();
        floorAt = floorGoingTo;
        requests.pop_front();
       // floorGoingTo = 0; // sends elevator back to waiting
    }

    void openDoors(){ // elevator 
        // should open doors using static variable calculated previously
        this_thread::sleep_for(chrono::milliseconds(100));
        return;
    }
    void closeDoors(){ // elevator
        //should close doors using static variable calculated previously
        this_thread::sleep_for(chrono::milliseconds(100));
    }

    void waitElevator(){ // elevator waits for further instruction
        unique_lock<mutex> lock(mtx);
        //while (floorGoingTo = 0){
        //     this_thread::sleep_for(chrono::milliseconds(100)); // waits until scheduler puts in a new floor for it to go to
        //}

        while(requests.empty()){
            //waitE.wait(lock); since im having a hard time making the mutex lock work, im just going to install a secondary "wait" system
             this_thread::sleep_for(chrono::milliseconds(100));
             

        }
        return;
    }
};

void elevatorThread(OperateElevator &shaft){
    cout << "Elevator Initialized" << endl;
    //while(true){
       // shaft.goToFloor();
        //shaft.waitElevator();
   // }
}

void floorThread(OperateElevator &shaft){

}

void schedulerThread(Scheduler &control){
   // while(true){

    //}   
}

int main(){
    class OperateElevator shaft;

    thread elevator1(elevatorThread, ref(shaft));
   // thread scheduler(schedulerThread, control);
   // thread floors(floorThread, shaft);
    
    elevator1.join();
    //scheduler.join();
    //floors.join();*/
}