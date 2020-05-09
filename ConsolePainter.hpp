//
// Created by ChenChen on 5/4/20.
//

#ifndef ELEVATOR_SIMU_CONSOLEPAINTER_HPP
#define ELEVATOR_SIMU_CONSOLEPAINTER_HPP


#include <chrono>
#include <vector>
#include <iostream>
#include "TowerContext.hpp"
#include "Elevator.hpp"
#include "Generator.hpp"
#include "Statistician.hpp"

using namespace ::std;

class ConsolePainter {
private:
    std::chrono::seconds frequency;
    char up = '^';
    char down = 'v';
    char still = '-';
    int osType = 0;

    char getStatusMark(int s) const;

public:
    explicit ConsolePainter(int freq, int ot);

    ConsolePainter();

    void drawCurrent(const vector<int>& eleCurFloor, const TowerContext &towerContext, const vector<Elevator> &eles, int totalPeopleServed,int totalPeopleGetOff, double averageTurnAroundTime);

    int getFrequency();

    void setFrequency(int freq);

    void clearScreen(int numNeedsClean);

    [[noreturn]] void runPainter(const TowerContext &towerContext, const vector<Elevator> &eles, const Generator &gen, const Statistician &stas);
};

ConsolePainter::ConsolePainter() {
    // default refresh period 1s
    frequency = std::chrono::seconds(1);
}

ConsolePainter::ConsolePainter(int freq, int ot) : osType(ot) {
    frequency = std::chrono::seconds(freq);
}

void ConsolePainter::clearScreen(int numNeedsClear) {
    if(osType == 0){
        for (int i = 0; i <= numNeedsClear; i++) {
            cout << "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b";
        }
    }else if(osType == 1){
        system("CLS");
    }
}

[[noreturn]] void ConsolePainter::runPainter(const TowerContext &towerContext, const vector<Elevator> &eles, const Generator &gen, const Statistician &stas) {
    int cleanLine = towerContext.getFloorNum() + eles.size() + 5;
    vector<int> eleCurFloor(eles.size());
    while (true) {
        clearScreen(cleanLine);
        for(int i = 0; i < eles.size(); i++){
            eleCurFloor[i] = eles[i].getCurrentFloor();
        }
        drawCurrent(eleCurFloor, towerContext, eles, gen.getAccumulate(), stas.getTotalGetOff(), stas.getTurnAroundTimeData());
        this_thread::sleep_for(frequency);
    }
}

void ConsolePainter::drawCurrent(const vector<int>& eleCurFloor, const TowerContext &towerContext, const vector<Elevator> &eles, int totalPeopleServed,int totalPeopleGetOff, double averageTurnAroundTime) {
    int eleNum = eles.size();
    cout << "Total generated (" << totalPeopleServed << "), Total Served (" << totalPeopleGetOff
         << "), AverageWaitingTime (" << averageTurnAroundTime << "s)" << endl;
    cout << "Number of elevator: " << eleNum << endl;
    for (int i = 0; i < eleNum; i++) {
        cout << i << "th elevator : " << "drop at [";
        vector<int> manInEle = eles[i].getManInEleTarget();
        for (int j : manInEle) {
            cout << j << " ";
        }
        cout << "]" << endl;
//        cout << " : des(" << eles[i].getCurrentDesAndOpDes().first + 1 << ") | opdes("
//             << eles[i].getCurrentDesAndOpDes().second + 1 << ")" << endl;
    }
    cout << endl;
    int floorNum = towerContext.getFloorNum();
    for (int i = floorNum; i > 0; i--) {
        // print floor num
        cout << i << "\t";
        // print floor & ele status
        for (int j = 0; j < eleCurFloor.size(); j++) {
            cout << "|";
            if (i - 1 == eleCurFloor[j]) {
                cout << getStatusMark(eles[j].getStatus()) << "[" << eles[j].getCurrentPeople() << "]";
//                cout << eachEle.getCurrentPeople() << getStatusMark(eachEle.getStatus()) << "  ";
            } else {
                cout << "    ";
            }
            cout << "|" << "\t";
        }
        // print wait man
        cout << "waiting : ";
        cout << "up[" << towerContext.getFloorWaitingUp(i - 1) << "] ";
        cout << "down[" << towerContext.getFloorWaitingDown(i - 1) << "]" << endl;
    }
}

int ConsolePainter::getFrequency() {
    return frequency.count();
}

void ConsolePainter::setFrequency(int freq) {
    frequency = std::chrono::seconds(freq);
}

char ConsolePainter::getStatusMark(int s) const {
    if (s > 0) {
        return up;
    } else if (s < 0) {
        return down;
    } else {
        return still;
    }
}


#endif //ELEVATOR_SIMU_CONSOLEPAINTER_HPP
