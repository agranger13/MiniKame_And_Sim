#include "Kame.h"
#include <cmath>

#ifndef KAME_PI
#define KAME_PI 3.14159265f
#endif

Kame::Kame(KameHAL& hal) : _hal(hal), _cycleRefTimeMs(0) {
    _gait.period = 2000;
    _gait.hipAmplitudeDeg = 0;
    _gait.hipNeutralDeg = 90;
    _gait.kneeNeutralDeg = 90;
    _gait.kneeLiftDeg = 0;
    for (int i = 0; i < NUM_LEGS; i++) {
        _gait.phaseDeg[i] = 0;
        _gait.kneeMirror[i] = 1;
    }
}

void Kame::setGait(const GaitParams& gait) {
    _gait = gait;
    _cycleRefTimeMs = _hal.millis();
}

void Kame::update() {
    unsigned long delta = (_hal.millis() - _cycleRefTimeMs) % (unsigned long)_gait.period;
    float rawProgressDeg = (float)delta * 360.0f / (float)_gait.period;

    for (int i = 0; i < NUM_LEGS; i++) {
        float phase = fmodf(rawProgressDeg + _gait.phaseDeg[i], 360.0f);
        float theta = phase * KAME_PI / 180.0f;

        // Hanche : sinusoïde classique, balancement avant/arrière autour du neutre.
        float hipAngle = _gait.hipNeutralDeg + _gait.hipAmplitudeDeg * sinf(theta);
        _hal.setServo(i, hipAngle);

        // La hanche avance (swing, sin croissant) pour phase < 90 ou phase >= 270 ;
        // elle recule en poussant (stance, sin décroissant) pour 90 <= phase < 270.
        // Le pied ne se lève que pendant le swing.
        bool swinging = (phase < 90.0f) || (phase >= 270.0f);
        float kneeAngle = swinging
            ? _gait.kneeNeutralDeg + _gait.kneeLiftDeg
            : _gait.kneeNeutralDeg;
        // Pattes montées en miroir : on reflète autour du centre mécanique
        // réel du joint (hipNeutralDeg, 90°), pas autour de kneeNeutralDeg —
        // sans ça le sens de pliure est faux pour ces pattes-là.
        if (_gait.kneeMirror[i] < 0) {
            kneeAngle = 2 * _gait.hipNeutralDeg - kneeAngle;
        }
        _hal.setServo(NUM_LEGS + i, kneeAngle);
    }
}
