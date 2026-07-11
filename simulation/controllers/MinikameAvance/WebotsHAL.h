#ifndef webots_hal_h
#define webots_hal_h

#include "../../../kame-core/KameHAL.h"
#include <webots/Robot.hpp>
#include <webots/Motor.hpp>

class WebotsHAL : public KameHAL {

    public:
        WebotsHAL(webots::Robot* robot, webots::Motor* motors[], int numMotors, int timeStepMs);
        void setServo(int id, float angleDeg) override;
        unsigned long millis() override;
        void delayMs(unsigned long ms) override;

    private:
        webots::Robot* _robot;
        webots::Motor** _motors;
        int _numMotors;
        int _timeStepMs;
};

#endif
