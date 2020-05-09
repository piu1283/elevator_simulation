//
// Created by ChenChen on 5/4/20.
//

#ifndef ELEVATOR_SIMU_SCHEDULER_HPP
#define ELEVATOR_SIMU_SCHEDULER_HPP

#include <vector>
#include "Person.hpp"
#include <queue>
#include "Elevator.hpp"
#include "TowerContext.hpp"
#include <algorithm>
using namespace::std;


class Scheduler {
private:
    condition_variable cv;
    mutex taskMutex;
    queue<Person> taskQue;
    bool stillFirst; // whether first assign to still ele, if not, first consider ele with same direction

public:
    explicit Scheduler();

    /**
     * assign task to elevator
     * @param p
     * @param floorStatus the waiting people in this floor, up and down
     * @return the assigned elevator
     */
    int assignTask(Person p, vector<Elevator> &eleStatus);

    int assign(int floorNum, bool up,vector<Elevator> &eleStatus);

    void addTask(Person p);

    Person getTask();

    [[noreturn]] void runScheduler(TowerContext &towerContext, vector<Elevator> &eleStatus);
};

Scheduler::Scheduler() {
    stillFirst = false;
}

int Scheduler::assignTask(Person p, vector<Elevator> &eleStatus){
    int assignedEleId = assign(p.getWaitFloor(), p.goUp(), eleStatus);
    eleStatus[assignedEleId].wakeUp();
    return assignedEleId;
}


bool LesserSort (pair<int, int> a,pair<int,int> b) { return (a.second<b.second); }

int Scheduler::assign(int floorNum, bool up, vector<Elevator> &eleStatus){
    int waitFloor = floorNum;
    if(stillFirst){
        for (int i = 0; i < eleStatus.size(); i++) {
            if(eleStatus[i].isStill() && !eleStatus[i].isFull()){
                eleStatus[i].setPickUpTask(waitFloor);
                return i;
            }
        }
    }
    // same direction and the nearest.
    vector<pair<int, int>> v; // <eleId, diff>
    for (int i = 0; i < eleStatus.size(); i++) {
        if (eleStatus[i].isStill()) { // still,
            pair<int, int> pp(i, abs(eleStatus[i].getCurrentFloor() - floorNum));
            v.push_back(pp);
        } else if (!(eleStatus[i].isGoUp() ^ up)) { // same direction
            if((up && floorNum < eleStatus[i].getCurrentFloor()) || (!up && floorNum > eleStatus[i].getCurrentFloor())){
                continue;
            }
            pair<int, int> pp(i, abs(eleStatus[i].getCurrentFloor() - floorNum));
            v.push_back(pp);
        } else {
            // do nothing
        }
    }
    if (!v.empty()) {
        sort(v.begin(), v.end(), LesserSort);
        eleStatus[v[0].first].setPickUpTask(waitFloor);
        return v[0].first; // return the ele with least diff with floorNum
    }

    // if all condition not satisfied, just return first one
    return 0;
}

void Scheduler::addTask(Person p) {
    unique_lock<mutex> t (taskMutex);
    taskQue.push(p);
    cv.notify_one();
    t.unlock();
}

Person Scheduler::getTask() {
    unique_lock<mutex> t (taskMutex);
    while(taskQue.empty()){
        cv.wait(t);
    }
    Person p = taskQue.front();
    taskQue.pop();
    t.unlock();
    return p;
}

[[noreturn]] void Scheduler::runScheduler(TowerContext &towerContext, vector<Elevator> &eleStatus) {
    while (true) {
        Person p = getTask();
        int eleId = assignTask(p, eleStatus);
        towerContext.updateTowerStatus(p.getWaitFloor(), p);
        if(!towerContext.memBookAlreadyTrue(p.getWaitFloor(),p.goUp())){
            towerContext.setEleMemBookTrue(p.getWaitFloor(), eleId, p.goUp());
        }
    }
}


#endif //ELEVATOR_SIMU_SCHEDULER_HPP
