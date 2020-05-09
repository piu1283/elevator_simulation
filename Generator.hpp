#include <climits>
//
// Created by ChenChen on 5/6/20.
//

#ifndef ELEVATOR_SIMU_GENERATOR_HPP
#define ELEVATOR_SIMU_GENERATOR_HPP

#include "Person.hpp"
#include "Scheduler.hpp"
#include <chrono>
#include <random>

using namespace ::std;

class Generator {
private:
    int totalFloor;
    int speed;
    int totalNum;
    int accumulate;
    default_random_engine e;
public:
    explicit Generator(int tf);

    Generator(int tf, int sp, int total);

    int getAccumulate() const;

    void runGenerator(Scheduler &scheduler);

    Person generate();
};

Generator::Generator(int tf) : totalFloor(tf), speed(5), totalNum(-1){
    e.seed((unsigned) time(nullptr));
}

Person Generator::generate() {
    uniform_int_distribution<unsigned> u(0, totalFloor - 1);
    int waitFloor (u(e));
    int targetFloor(u(e));
    while (waitFloor == targetFloor){
        targetFloor = u(e);
    }
    Person p (targetFloor, waitFloor);
    return p;
}

void Generator::runGenerator(Scheduler &scheduler) {
    accumulate = 0;
    while (totalNum < 0 || accumulate < totalNum) {
        accumulate++;
        Person p = generate();
        scheduler.addTask(p);
        this_thread::sleep_for(chrono::seconds(speed));
    }
}

Generator::Generator(int tf, int sp, int total) : totalFloor(tf), speed(sp), totalNum(total) {
    e.seed((unsigned) time(nullptr));
}

int Generator::getAccumulate() const {
    return accumulate;
}


#endif //ELEVATOR_SIMU_GENERATOR_HPP
