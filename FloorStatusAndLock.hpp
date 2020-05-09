//
// Created by 陈宸 on 5/6/20.
//

#ifndef ELEVATOR_SIMU_FLOORSTATUSANDLOCK_HPP
#define ELEVATOR_SIMU_FLOORSTATUSANDLOCK_HPP

#include "Person.hpp"
#include <mutex>
#include <queue>
#include <vector>

using namespace ::std;

class FloorStatusAndLock{
private:
    queue<Person> upPerson;
    queue<Person> downPerson;
    mutex upMutex;
    mutex downMutex;
public:

//    FloorStatusAndLock();

    queue<Person> getNumOfPeople(int num, bool up);

    void addPeople(Person p);

    bool hasMoreUp();

    bool hasMoreDown();

    int getWaitingUpNum() const;

    int getWaitingDownNum() const;

};


queue<Person> FloorStatusAndLock::getNumOfPeople(int num, bool up) {
    queue<Person> res;
    if(up){
        unique_lock<mutex> t(upMutex, try_to_lock);
        while(num > 0 && !upPerson.empty()) {
            res.push(upPerson.front());
            upPerson.pop();
        }
        t.unlock();
    }else{
        unique_lock<mutex> t(downMutex, try_to_lock);
        while(num > 0 && !downPerson.empty()) {
            res.push(downPerson.front());
            downPerson.pop();
        }
        t.unlock();
    }
    return res;
}

void FloorStatusAndLock::addPeople(Person p) {
    if(p.goUp()){
        unique_lock<mutex> t(upMutex);
        upPerson.push(p);
        t.unlock();
    }else{
        unique_lock<mutex> t(downMutex);
        downPerson.push(p);
        t.unlock();
    }
}

bool FloorStatusAndLock::hasMoreUp() {
    return !upPerson.empty();
}

bool FloorStatusAndLock::hasMoreDown() {
    return !downPerson.empty();
}

int FloorStatusAndLock::getWaitingUpNum() const {
    return upPerson.size();
}

int FloorStatusAndLock::getWaitingDownNum() const {
    return downPerson.size();
}


#endif //ELEVATOR_SIMU_FLOORSTATUSANDLOCK_HPP
