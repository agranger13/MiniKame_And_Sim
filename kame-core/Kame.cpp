#include "Kame.h"
#include <cmath>

#ifndef KAME_PI
#define KAME_PI 3.14159265f
#endif

Kame::Kame(KameHAL& hal) : _hal(hal), _cycleRefTimeMs(0), _speed(1.0f), _turn(0.0f) {
    _gait.period = 600;
    _gait.hipAmplitudeDeg = 0;
    _gait.kneeAmplitudeDeg = 0;
    _gait.kneeOffsetDeg = 0;
    _gait.kneeSwingWindowDeg = 360;
    for (int i = 0; i < NUM_LEGS; i++) {
        _gait.phaseDeg[i] = 0;
        _gait.hipDir[i] = 1;
        _gait.kneeDir[i] = 1;
        _gait.turnSign[i] = 1;
    }
}

void Kame::setGait(const GaitParams& gait) {
    _gait = gait;
    _cycleRefTimeMs = _hal.millis();
}

void Kame::setControl(float speed, float turn) {
    _speed = speed;
    _turn = turn;
}

void Kame::update() {
    unsigned long elapsedSinceStart = _hal.millis() - _cycleRefTimeMs;
    unsigned long delta = elapsedSinceStart % (unsigned long)_gait.period;
    float rawProgressDeg = (float)delta * 360.0f / (float)_gait.period;

    float moving = fminf(1.0f, fabsf(_speed) + fabsf(_turn));

    for (int i = 0; i < NUM_LEGS; i++) {
        float hipPhaseDeg = fmodf(rawProgressDeg + _gait.phaseDeg[i], 360.0f);
        float hipTheta = hipPhaseDeg * KAME_PI / 180.0f;

        // Virage differentiel : on ajoute/retranche turn selon le cote de la patte.
        float gain = _speed + _gait.turnSign[i] * _turn;
        if (gain > 1.0f) gain = 1.0f;
        if (gain < -1.0f) gain = -1.0f;

        float hipAngle = 90.0f + _gait.hipDir[i] * _gait.hipAmplitudeDeg * gain * sinf(hipTheta);
        _hal.setServo(i, hipAngle);

        // Genou dephase de 90 deg par rapport a la hanche (son pic de levage tombe a
        // hipPhaseDeg == 0), actif seulement si le robot bouge.
        float kneePhaseDeg = fmodf(hipPhaseDeg + 90.0f, 360.0f);
        float kneeTheta = kneePhaseDeg * KAME_PI / 180.0f;

        // En dehors de sa fenetre de levage, le genou reste fige a kneeOffsetDeg
        // (patte au sol, immobile) : indispensable pour un gait sequentiel type
        // wave, ou les 3 pattes non-swingantes doivent rester des appuis stables.
        float centeredDeg = fmodf(hipPhaseDeg + 540.0f, 360.0f) - 180.0f; // (-180, 180], 0 = pic
        bool inSwingWindow = fabsf(centeredDeg) <= _gait.kneeSwingWindowDeg / 2.0f;
        float kneeSine = inSwingWindow ? sinf(kneeTheta) : 0.0f;

        float kneeAngle = 90.0f + _gait.kneeDir[i] *
            (_gait.kneeOffsetDeg + _gait.kneeAmplitudeDeg * moving * kneeSine);
        _hal.setServo(NUM_LEGS + i, kneeAngle);
    }
}
