//
// Created by ChenChen on 5/4/20.
//

#ifndef ELEVATOR_SIMU_PERSON_HPP
#define ELEVATOR_SIMU_PERSON_HPP

#include <chrono>

class Person {
private:
    int targetFloor;
    int waitFloor;
    std::chrono::system_clock::time_point start;
    std::chrono::system_clock::time_point end;

public:

    Person(int tf, int wf);

    void setTargetFloor(int tf);

    void setWaitFloor(int wf);

    void setGetOffTime();

    int getTargetFloor() const;

    int getWaitFloor() const;

    bool goUp() const;

    int getTurnAroundTimeInSecond();

};

void Person::setTargetFloor(int tf) {
    targetFloor = tf;
}

void Person::setWaitFloor(int wf) {
    waitFloor = wf;
}

int Person::getTargetFloor() const {
    return targetFloor;
}

bool Person::goUp() const {
    return waitFloor<targetFloor;
}

int Person::getWaitFloor() const {
    return waitFloor;
}

Person::Person(int tf, int wf) : targetFloor(tf), waitFloor(wf) {
    start = std::chrono::system_clock::now();
}

void Person::setGetOffTime() {
    end = std::chrono::system_clock::now();
}

int Person::getTurnAroundTimeInSecond() {
    return std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
}


#endif //ELEVATOR_SIMU_PERSON_HPP
