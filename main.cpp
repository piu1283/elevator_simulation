
#include <vector>
#include <thread>
#include "Elevator.hpp"
#include "Generator.hpp"
#include "Scheduler.hpp"
#include "ConsolePainter.hpp"
#include "Statistician.hpp"

using namespace ::std;

// global config
int totalFloor = 30;
int numOfEle = 3;
int osType = 0;// 0 is macos, 1 is windows
int printFreq = 1;

// generator config
int generateSpeed = 2;

// ele config
int maxNumPeopleInEle = 9;

int main() {

    vector<Elevator> eles;
    // init elevator
    for (int i = 0; i < numOfEle; i++) {
        Elevator ele(i, maxNumPeopleInEle, totalFloor);
        eles.push_back(ele);
    }
    // init scheduler
    Scheduler scheduler;
    // init towerContext
    TowerContext context(totalFloor, numOfEle);
    // init generator
    Generator generator(totalFloor, generateSpeed, -1);
    // init console painter
    ConsolePainter cp(printFreq, osType);
    // init Statistician
    Statistician stats;

    // open thread
    vector<thread> eleThreadV;
    for (int i = 0; i < numOfEle; i++) {
        thread t(&Elevator::runElevator, &eles[i], ref(context), ref(stats));
        eleThreadV.push_back(move(t));
    }
    thread schedulerThread(&Scheduler::runScheduler, &scheduler, ref(context), ref(eles));
    thread generatorThread(&Generator::runGenerator, &generator, ref(scheduler));
    thread cpThread(&ConsolePainter::runPainter, &cp, ref(context), ref(eles), ref(generator), ref(stats));
    thread statsThread(&Statistician::calculate, &stats);
    // do join
    for (int i = 0; i < numOfEle; i++) {
        eleThreadV[i].join();
    }
    schedulerThread.join();
    generatorThread.join();
    cpThread.join();
    statsThread.join();

}
