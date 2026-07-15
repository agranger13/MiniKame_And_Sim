#ifndef kame_h
#define kame_h

#include "KameHAL.h"

// Un jeu de paramètres qui décrit un gait complet (trot, wave, etc) pour les
// 4 pattes (hanche + genou par patte).
struct GaitParams {
    int period;             // ms, durée d'un cycle complet
    int phaseDeg[4];        // déphasage par patte (0-360)
    int hipAmplitudeDeg;
    int hipNeutralDeg;      // convention HAL : 90 = neutre
    int kneeNeutralDeg;     // angle genou "posé au sol" (position debout calibrée)
    int kneeLiftDeg;        // écart par rapport au neutre pour lever le pied pendant le swing
    int legMirror[4];       // +1/-1 : pattes 1,3 montées en miroir géométrique de 2,4 (hanche ET genou)
};

// Logique robot : n'appelle QUE le HAL, jamais Arduino.h ni webots directement.
//
// Le pied est levé uniquement pendant la moitié du cycle où la hanche
// avance (retour/swing), et reste posé au sol pendant l'autre moitié où la
// hanche recule (poussée/stance) — sans ça la patte racle le sol dans les
// deux sens et le robot marche sur place au lieu d'avancer.
class Kame {

    public:
        static const int NUM_LEGS = 4;
        static const int NUM_MOTORS = NUM_LEGS * 2; // servo id 0..3=hanches, 4..7=genoux

        Kame(KameHAL& hal);

        void setGait(const GaitParams& gait);
        void update();

    private:
        KameHAL& _hal;
        GaitParams _gait;
        unsigned long _cycleRefTimeMs;
};

#endif
