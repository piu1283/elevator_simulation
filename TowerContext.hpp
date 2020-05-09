//
// Created by ChenChen on 5/6/20.
//

#ifndef ELEVATOR_SIMU_TOWERCONTEXT_HPP
#define ELEVATOR_SIMU_TOWERCONTEXT_HPP

#include "FloorStatusAndLock.hpp"
#include "Person.hpp"
#include <vector>
#include <queue>

class TowerContext {
private:
    vector<pair<vector<bool>, vector<bool>>> memBook;
    vector<FloorStatusAndLock> towerStatus;
    mutex memBookMutex;
public:
    TowerContext(int totalFloor, int numOfEle);

    void updateTowerStatus(int floorNum, Person p);

    void setEleMemBookTrue(int floorNum, int eleId, bool up);

    void setEleMemBookFalse(int floorNum, int eleId, bool up);

    void setMemBookAllFalse(int floorNum, bool up);

    queue<Person> getInEle(int num, int floorNum, bool up);

    bool memBookAlreadyTrue(int floorNum, bool up);

    bool needGoUp(int floorNum, int eleId);

    bool hasRemain(int floorNum, bool up);

    int getFloorNum() const;

    int getFloorWaitingUp(int floorNum) const;

    int getFloorWaitingDown(int floorNum) const;


};


TowerContext::TowerContext(int totalFloor, int numOfEle) {
    memBook = vector<pair<vector<bool>, vector<bool>>>(totalFloor,
                                                       {vector<bool>(numOfEle, false), vector<bool>(numOfEle, false)});
    towerStatus = vector<FloorStatusAndLock>(totalFloor);
}

void TowerContext::updateTowerStatus(int floorNum, Person p) {
    towerStatus[floorNum].addPeople(p);
}

bool TowerContext::memBookAlreadyTrue(int floorNum, bool up) {
    if(up){
        for (bool b : memBook[floorNum].first) {
            if(b) {
                return true;
            }
        }
    }else{
        for (bool b : memBook[floorNum].second) {
            if(b) {
                return true;
            }
        }
    }
    return false;
}

void TowerContext::setEleMemBookTrue(int floorNum, int eleId, bool up) {
    unique_lock<mutex> t(memBookMutex);
    if (up) {
        memBook[floorNum].first[eleId] = true;
    }else{
        memBook[floorNum].second[eleId] = true;
    }
}

void TowerContext::setEleMemBookFalse(int floorNum, int eleId, bool up) {
    unique_lock<mutex> t(memBookMutex);
    if (up) {
        memBook[floorNum].first[eleId] = false;
    }else{
        memBook[floorNum].second[eleId] = false;
    }
}

queue <Person> TowerContext::getInEle(int num, int floorNum, bool up) {
    queue <Person> res;
    res = towerStatus[floorNum].getNumOfPeople(num, up);
    return res;
}

bool TowerContext::needGoUp(int floorNum, int eleId) {
    return memBook[floorNum].first[eleId];
}

bool TowerContext::hasRemain(int floorNum, bool up) {
    if (up) {
        return towerStatus[floorNum].hasMoreUp();
    }else{
        return towerStatus[floorNum].hasMoreDown();
    }
}

void TowerContext::setMemBookAllFalse(int floorNum, bool up) {
    unique_lock<mutex> t(memBookMutex);
    if (up) {
        int size = memBook[floorNum].first.size();
        for (int i = 0; i < size; i++) {
            memBook[floorNum].first[i] = false;
        }
    }else{
        int size = memBook[floorNum].second.size();
        for (int i = 0; i < size; i++) {
            memBook[floorNum].second[i] = false;
        }
    }
}

int TowerContext::getFloorNum() const {
    return towerStatus.size();
}

int TowerContext::getFloorWaitingUp(int floorNum) const {
    return towerStatus[floorNum].getWaitingUpNum();
}

int TowerContext::getFloorWaitingDown(int floorNum) const {
    return towerStatus[floorNum].getWaitingDownNum();
}


#endif //ELEVATOR_SIMU_TOWERCONTEXT_HPP
