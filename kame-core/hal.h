// hal.h — kame-core n'inclut QUE ça, jamais Arduino.h ni webots
struct KameHAL {
    virtual void setServo(int id, float angleDeg) = 0;
    virtual unsigned long millis() = 0;
    virtual void delayMs(unsigned long ms) = 0;
};