#include "WebotsHAL.h"
#include <cmath>

using namespace webots;

WebotsHAL::WebotsHAL(Robot* robot, Motor* motors[], int numMotors, int timeStepMs)
    : _robot(robot), _motors(motors), _numMotors(numMotors), _timeStepMs(timeStepMs) {
}

// convention HAL : 0-180°, 90° = neutre -> radians autour de la position centrale du HingeJoint
void WebotsHAL::setServo(int id, float angleDeg) {
    if (id < 0 || id >= _numMotors) return;
    float rad = (angleDeg - 90.0f) * (float)M_PI / 180.0f;
    _motors[id]->setPosition(rad);
}

unsigned long WebotsHAL::millis() {
    return (unsigned long)(_robot->getTime() * 1000.0);
}

void WebotsHAL::delayMs(unsigned long ms) {
    unsigned long start = millis();
    while (millis() - start < ms) {
        if (_robot->step(_timeStepMs) == -1) break;
    }
}
