/**
 * @file VacPi.cpp
 * @brief VacPi main library with abstract logic
 * @author Benedikt Volkmer
 * @version 0.1
 * @date 2018-03-09
 */
#include "VacPi.h"

using namespace VacPi;
Looper::Looper(Movements* movements) {
    this->movements = movements;
    this->currentState = new States::LinearRun(this->movements);
}

Looper::~Looper() {
    delete this->currentState;
}

void Looper::loop(Obstruction obstruction) {
    //TOOD: Implement behaviour when reaching a step
    if (obstruction.dustLevel || (obstruction.left && obstruction.right)) {
        movements->stopAll();
    } else {
        movements->startVacuum();
        States::State* newState = this->currentState->run(obstruction.front);
        if (this->currentState != newState) {
            delete this->currentState;
            this->currentState = newState;
        }
    }
}

using namespace States;

EdgeRun::EdgeRun(Movements* movements): State(movements) {}
State* EdgeRun::run(bool obstruction) {
    timer++;
    obstructionTimer++;
    if (obstruction) {
        obstructionTimer = 0;
        straightTimer = 0;
        movements->curve(Backward, RIGHT, Fast);
        return this;
    } else if (timer > timerThreshold) {
        movements->rotate45(LEFT);
        return new LinearRun(movements);
    } else if (obstructionTimer > obstructionTimerThreshold) {
        return new Circling(movements);
    }
    straightTimer++;
    if (straightTimer <= 5) {
        movements->moveStraight(Forward);
    } else {
        movements->curve(Forward, RIGHT, Slow);
    }
    return this;
}

LinearRun::LinearRun(Movements* movements): State(movements) { }
State* LinearRun::run(bool obstruction) {
    timer++;
    if (obstruction) {
        movements->rotate45(LEFT);
        return this;
    } else if (timer > timerThreshold) {
        return new Circling(movements);
    }
    movements->moveStraight(Forward);
    return this;
}

Circling::Circling(Movements* movements): State(movements) {}
State* Circling::run(bool obstruction) {
    timer++;
    if (obstruction) {
        return new EdgeRun(movements);
    } else if (timer > timerThreshold) {
        return new Circling(movements);
    }
    movements->spiral(LEFT, timer);
    return this;
}
