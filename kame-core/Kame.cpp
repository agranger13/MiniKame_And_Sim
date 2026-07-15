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
        _gait.legMirror[i] = 1;
    }
}

void Kame::setGait(const GaitParams& gait) {
    _gait = gait;
    _cycleRefTimeMs = _hal.millis();
}

void Kame::update() {
    unsigned long elapsedSinceStart = _hal.millis() - _cycleRefTimeMs;
    unsigned long delta = elapsedSinceStart % (unsigned long)_gait.period;
    float rawProgressDeg = (float)delta * 360.0f / (float)_gait.period;

    // Démarrage progressif : sans ça, toutes les pattes sautent instantanément
    // à leur angle cible à t=0, avant que le cycle périodique ne soit établi —
    // cet à-coup initial fait pivoter le corps (~20° observé) avant que la
    // marche ne se stabilise. On ramène l'amplitude (hanche + levage) de 0 à
    // pleine valeur sur les 500 premières ms.
    const unsigned long kRampMs = 500;
    float ramp = (elapsedSinceStart >= kRampMs) ? 1.0f : (float)elapsedSinceStart / (float)kRampMs;
    float hipAmplitude = _gait.hipAmplitudeDeg * ramp;
    float kneeLift = _gait.kneeLiftDeg * ramp;

    for (int i = 0; i < NUM_LEGS; i++) {
        float phase = fmodf(rawProgressDeg + _gait.phaseDeg[i], 360.0f);
        float theta = phase * KAME_PI / 180.0f;

        // Hanche : sinusoïde classique, balancement avant/arrière autour du neutre.
        float hipAngle = _gait.hipNeutralDeg + hipAmplitude * sinf(theta);
        // Pattes montées en miroir géométrique (droite/gauche) : une même
        // commande produit un mouvement opposé. On reflète autour du centre
        // mécanique réel du joint (hipNeutralDeg, 90°) pour ces pattes-là,
        // sinon la poussée est asymétrique et le robot tourne au lieu d'avancer.
        if (_gait.legMirror[i] < 0) {
            hipAngle = 2 * _gait.hipNeutralDeg - hipAngle;
        }
        _hal.setServo(i, hipAngle);

        // La hanche avance (swing, sin croissant) pour phase < 90 ou phase >= 270 ;
        // elle recule en poussant (stance, sin décroissant) pour 90 <= phase < 270.
        // Le pied ne se lève que pendant le swing. Le reflet de hanche ci-dessus
        // reproduit le MÊME mouvement réel (physique) que la formule brute pour
        // une patte non miroir — donc la fenêtre de swing reste basée sur la
        // phase brute pour toutes les pattes (un décalage ici annule le reflet
        // de hanche et fait tourner le robot sur place au lieu d'avancer).
        bool swinging = (phase < 90.0f) || (phase >= 270.0f);
        float kneeAngle = swinging
            ? _gait.kneeNeutralDeg + kneeLift
            : _gait.kneeNeutralDeg;
        // Même miroir que la hanche : le sens de pliure du genou est aussi
        // inversé pour ces pattes-là.
        if (_gait.legMirror[i] < 0) {
            kneeAngle = 2 * _gait.hipNeutralDeg - kneeAngle;
        }
        _hal.setServo(NUM_LEGS + i, kneeAngle);
    }
}
