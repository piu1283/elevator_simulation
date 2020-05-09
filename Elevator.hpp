//
// Created by ChenChen on 5/4/20.
//

#ifndef ELEVATOR_SIMU_ELEVATOR_HPP
#define ELEVATOR_SIMU_ELEVATOR_HPP

#include "Person.hpp"
#include "TowerContext.hpp"
#include "Statistician.hpp"
#include <condition_variable>
#include <vector>
#include <thread>
#include <chrono>
#include <algorithm>
#include <queue>
#include <mutex>

using namespace ::std;

class Elevator {
private:
    int id;
    int currentFloor;
    int status;// 1 go up, -1 go down, 0 still
    vector<queue<Person>> floorDrop;
    int destination;
    int oppositeDestination;
    int maxNumPeople;
    int timeMove; // the time that ele needs to move to next floor, in milliseconds
    int currentNumPeople;
    mutex destinationMutex;
    condition_variable cv;
    mutex sleepMutex;

    void setDestination(int floor);

public:

    pair<int, int> getCurrentDesAndOpDes() const;

    explicit Elevator(int idx);

    Elevator(int idx, int mnp, int floorNum);

    Elevator &operator=(const Elevator &ele);

    Elevator (const Elevator &ele);

    int getStatus() const;

    bool isStill() const;

    bool isGoUp() const;

    bool isGoDown() const;

    bool isFull() const;

    void setPickUpTask(int floorNum);

    void LoadCurrent(queue<Person> load);

    queue<Person> getOffCurrent();

    /**
     * move ele
     * @return if moved, return true, else return false;
     */
    bool move();

    int getCurrentFloorLabel() const;

    vector<int> getManInEleTarget() const;

    int getCurrentPeople() const;

    int getCurrentFloor() const;

    void wakeUp();

    [[noreturn]] void runElevator(TowerContext &context, Statistician &statistician);

};

Elevator::Elevator(const Elevator &ele) {
    id = ele.id;
    currentFloor = ele.currentFloor;
    status = ele.status;
    floorDrop = ele.floorDrop;
    destination = ele.destination;
    oppositeDestination = ele.oppositeDestination;
    maxNumPeople = ele.maxNumPeople;
    timeMove = ele.timeMove;
    currentNumPeople = ele.currentNumPeople;
}

Elevator &Elevator::operator=(const Elevator &ele) {
    id = ele.id;
    currentFloor = ele.currentFloor;
    status = ele.status;
    floorDrop = ele.floorDrop;
    destination = ele.destination;
    oppositeDestination = ele.oppositeDestination;
    maxNumPeople = ele.maxNumPeople;
    timeMove = ele.timeMove;
    currentNumPeople = ele.currentNumPeople;
    return *this;
}

Elevator::Elevator(int idx) : id(idx){
    currentFloor = 0;
    status = 0;
    floorDrop = vector<queue<Person>>(20);
    destination = -1;
    oppositeDestination = -1;
    maxNumPeople = 9;
    currentNumPeople = 0;
    timeMove = 1000;
}

Elevator::Elevator(int idx, int mnp, int floorNum) : maxNumPeople(mnp), id(idx) {
    floorDrop = vector<queue<Person>>(floorNum);
    status = 0;
    destination = -1;
    oppositeDestination = -1;
    currentNumPeople = 0;
    currentFloor = 0;
    timeMove = 1000;
}

int Elevator::getStatus() const {
    return status;
}

void Elevator::setPickUpTask(int floorNum) {
    setDestination(floorNum);
}

bool Elevator::move() {
    unique_lock<mutex>t (destinationMutex);
    // simu stop
    if (currentNumPeople == 0 && (destination == currentFloor || destination < 0) && oppositeDestination < 0) {
        status = 0;
        destination = -1;
        oppositeDestination = -1;
        return false;
    }
    // simu start
    if (status == 0) {
        if (currentFloor > destination) {
            status = -1;
        } else {
            status = 1;
        }
    } else { // turn other direction
        if (destination < 0) {
            destination = oppositeDestination;
            oppositeDestination = -1;
            status *= -1;
        }
    }

    // check boundary
    int res = currentFloor;
    if((currentFloor == 0 && status < 0) || (currentFloor == floorDrop.size() - 1 && status > 0)){
        status *= -1;
    }
    res += status;
    t.unlock();
    // simu move time
    this_thread::sleep_for(chrono::milliseconds(timeMove));
    // set current floor
    currentFloor = res;
    return true;
}

vector<int> Elevator::getManInEleTarget() const {
    vector<int> res;
    for (int i = 0; i < floorDrop.size(); i++) {
        if (!floorDrop[i].empty()) {
            res.push_back(i + 1);
        }
    }
    return res;
}

int Elevator::getCurrentPeople() const {
    return currentNumPeople;
}

int Elevator::getCurrentFloor() const {
    return currentFloor;
}

/**
 * load or drop people in current floor.
 * @param load people waiting in current floor
 * @return the prople who get off the ele
 */
void Elevator::LoadCurrent(queue<Person> load) {
    // load people
    if (!load.empty()) {
        while (currentNumPeople < maxNumPeople && !load.empty()) {
            currentNumPeople++;
            Person tmpP = load.front();
            load.pop();
            floorDrop[tmpP.getTargetFloor()].push(tmpP);
            setDestination(tmpP.getTargetFloor());
        }
        this_thread::sleep_for(chrono::milliseconds(1000));
    }
}

int Elevator::getCurrentFloorLabel() const {
    return currentFloor + 1;
}

void Elevator::setDestination(int floor) {
    unique_lock<mutex> dt(destinationMutex);
    if (status == 0) {
        destination = floor;
    } else if (status > 0) {
        if (floor > currentFloor) {
            destination = destination > floor ? destination : floor;
        } else {
            oppositeDestination = oppositeDestination < floor ? oppositeDestination : floor;
        }
    } else {// status < 0
        if (floor < currentFloor) {
            destination = destination < floor ? destination : floor;
        } else {
            oppositeDestination = oppositeDestination > floor ? oppositeDestination : floor;
        }
    }
    dt.unlock();
}

bool Elevator::isStill() const {
    return status == 0;
}

bool Elevator::isGoUp() const {
    return status > 0;
}

bool Elevator::isGoDown() const {
    return status < 0;
}

bool Elevator::isFull() const {
    return currentNumPeople >= maxNumPeople;
}

void Elevator::wakeUp() {
    cv.notify_one();
}

[[noreturn]] void Elevator::runElevator(TowerContext &context, Statistician &statistician) {
    while (true) {
        // when in new floor, first get people off
        queue<Person> gops = getOffCurrent();
        // calculate turn around time
        statistician.putPeople(gops);
        // then load people
        int numPick = maxNumPeople - currentNumPeople;
        bool finalPickDirectionUp = true;
        // check pick direction
        if(status == 0){
            // if ele is still
            if(destination > currentFloor) {
                finalPickDirectionUp = true;
            }else if(destination < currentFloor){
                finalPickDirectionUp = false;
            }else{ // situation: when ele is still and the scheduler assign current floor to it
                // when in 0 or max floor, you only have one direction to go
                if(currentFloor == 0){
                    finalPickDirectionUp = true;
                }else if(currentFloor == floorDrop.size() - 1){
                    finalPickDirectionUp = false;
                }else{
                    // otherwise
                    finalPickDirectionUp = context.needGoUp(currentFloor, id);
                }
            }
        }else{
            // if not still
            if(currentFloor == 0){
                finalPickDirectionUp = true;
            }else if(currentFloor == floorDrop.size() - 1){
                finalPickDirectionUp = false;
            }else{
                finalPickDirectionUp = isGoUp();
            }
        }
        // if there are people in this floor at current direction
        if (context.hasRemain(currentFloor, finalPickDirectionUp)) {
            // have room for more
            if (numPick > 0) {
                queue<Person> p;
                p = context.getInEle(numPick, currentFloor, finalPickDirectionUp);
                LoadCurrent(p);
                // if current floor has remain in this direction
                if (context.hasRemain(currentFloor, finalPickDirectionUp)) {
                    context.setEleMemBookFalse(currentFloor, id, finalPickDirectionUp);
                } else {
                    context.setMemBookAllFalse(currentFloor, finalPickDirectionUp);
                }
            }
        }
        // finish load and unload
        // prepare to move
        if(!move()){
            unique_lock<mutex> sm(sleepMutex);
            cv.wait(sm);
        }
    }
}

queue<Person> Elevator::getOffCurrent() {
    // check if needs to drop
    queue<Person> getOff;
    if (currentNumPeople > 0) {
        // if some one needs to get off at this floor
        if (!floorDrop[currentFloor].empty()) {
            currentNumPeople -= floorDrop[currentFloor].size();
            while (!floorDrop[currentFloor].empty()) {
                Person p = floorDrop[currentFloor].front();
                p.setGetOffTime();
                getOff.push(p);
                floorDrop[currentFloor].pop();
            }
        }
    }
    return getOff;
}

pair<int, int> Elevator::getCurrentDesAndOpDes() const {
    return pair<int, int>{destination, oppositeDestination};
}


#endif //ELEVATOR_SIMU_ELEVATOR_HPP
