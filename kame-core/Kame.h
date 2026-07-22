#ifndef kame_h
#define kame_h

#include "KameHAL.h"

// Un jeu de parametres qui decrit un gait complet (trot, wave, etc) pour les
// 4 pattes (hanche + genou par patte). Ordre des pattes : 0=avant-gauche,
// 1=avant-droite, 2=arriere-gauche, 3=arriere-droite (FL, FR, RL, RR).
struct GaitParams {
    int period;             // ms, duree d'un cycle complet
    int phaseDeg[4];        // dephasage hanche par patte (0-360)
    int hipAmplitudeDeg;    // amplitude du balancement de hanche
    int kneeAmplitudeDeg;   // amplitude du mouvement de genou (lever/reculer)
    int kneeOffsetDeg;      // ecartement statique des pattes (genou au repos)
    int hipDir[4];          // +1/-1 : sens de hanche pour que "avancer" = positif
    int kneeDir[4];         // +1/-1 : sens de genou pour que "positif" = patte ecartee/levee
    int turnSign[4];        // +1 (gauche) / -1 (droite) : cote de la patte pour le virage differentiel
};

// Logique robot : n'appelle QUE le HAL, jamais Arduino.h ni webots directement.
//
// Chaque servo suit angle(t) = 90 + amplitude * sin(2*pi*t/period + phase),
// comme le firmware original miniKame. Le genou est dephase de 90 deg par
// rapport a la hanche pour lever le pied pendant son retour (swing).
class Kame {

    public:
        static const int NUM_LEGS = 4;
        static const int NUM_MOTORS = NUM_LEGS * 2; // servo id 0..3=hanches, 4..7=genoux

        Kame(KameHAL& hal);

        void setGait(const GaitParams& gait);
        // speed/turn dans [-1, 1] : avance/recule, tourne gauche/droite.
        void setControl(float speed, float turn);
        void update();

    private:
        KameHAL& _hal;
        GaitParams _gait;
        unsigned long _cycleRefTimeMs;
        float _speed;
        float _turn;
};

#endif
