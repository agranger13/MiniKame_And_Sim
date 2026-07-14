#include "Gaits.h"

// Pattes 1 et 3 (droite) montées en miroir des pattes 2 et 4 (gauche) dans
// MiniKame.wbt — confirmé visuellement en calibration (1 et 3 pliaient le
// genou dans le mauvais sens). On inverse le sens de pliure pour ces deux-là.
static void setKneeMirror(GaitParams& g) {
    g.kneeMirror[0] = -1;
    g.kneeMirror[1] = 1;
    g.kneeMirror[2] = -1;
    g.kneeMirror[3] = 1;
}

GaitParams trotGait() {
    GaitParams g;
    g.period = 2000;              // ms (0.5 Hz)
    g.phaseDeg[0] = 0;
    g.phaseDeg[1] = 180;
    g.phaseDeg[2] = 180;
    g.phaseDeg[3] = 0;
    g.hipAmplitudeDeg = 29;       // ~0.5 rad — balancement avant/arrière
    g.hipNeutralDeg = 90;
    g.kneeNeutralDeg = 45;        // position "debout" validée en calibration (tient entre 40 et 50°)
    g.kneeLiftDeg = 15;           // levage du pied pendant le swing
    setKneeMirror(g);
    return g;
}

GaitParams waveGait() {
    GaitParams g;
    g.period = 2400;              // ms — plus lent que le trot, une patte à la fois
    // Ordre : avant-droite (0°) -> avant-gauche (90°) -> arrière-droite (180°) -> arrière-gauche (270°)
    g.phaseDeg[0] = 0;
    g.phaseDeg[1] = 90;
    g.phaseDeg[2] = 180;
    g.phaseDeg[3] = 270;
    g.hipAmplitudeDeg = 29;
    g.hipNeutralDeg = 90;
    g.kneeNeutralDeg = 45;
    g.kneeLiftDeg = 15;
    setKneeMirror(g);
    return g;
}
