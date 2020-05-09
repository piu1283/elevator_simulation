//
// Created by ChenChen on 5/7/20.
//

#ifndef ELEVATOR_SIMU_STATISTICIAN_HPP
#define ELEVATOR_SIMU_STATISTICIAN_HPP

#include <mutex>
#include <queue>
#include "Person.hpp"

using namespace ::std;

class Statistician {
private:
    double timeSum = 0;
    double totalPerson = 0;
    double averageTurnAroundTime = 0;
    queue<Person> getOffQueue;
    mutex pm;
    condition_variable cv;

public:
    Statistician() = default;

    [[noreturn]] void calculate();

    void putPeople(queue<Person> p);

    double getTurnAroundTimeData() const;

    int getTotalGetOff() const;

};

void Statistician::putPeople(queue<Person> ps) {
    unique_lock<mutex> t(pm);
    while (!ps.empty()) {
        ps.front().setGetOffTime();
        getOffQueue.push(ps.front());
        ps.pop();
    }
    cv.notify_one();
}

[[noreturn]] void Statistician::calculate() {
    queue<Person> p;
    while(true) {
        unique_lock<mutex> t(pm);
        while (getOffQueue.empty()) {
            cv.wait(t);
        }
        getOffQueue.swap(p);
        t.unlock();
        while (!p.empty()) {
            timeSum += p.front().getTurnAroundTimeInSecond();
            p.pop();
            totalPerson++;
        }
        averageTurnAroundTime = timeSum / totalPerson;
    }
}

double Statistician::getTurnAroundTimeData() const {
    return averageTurnAroundTime;
}

int Statistician::getTotalGetOff() const {
    return totalPerson;
}


#endif //ELEVATOR_SIMU_STATISTICIAN_HPP
