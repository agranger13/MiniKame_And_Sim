#include "Gaits.h"

// Pattes 1 et 3 (droite) montées en miroir géométrique des pattes 2 et 4
// (gauche) dans MiniKame.wbt — confirmé visuellement en calibration. Affecte
// hanche ET genou : sans ça la poussée est asymétrique (le robot tourne) ou
// le genou plie dans le mauvais sens.
static void setLegMirror(GaitParams& g) {
    g.legMirror[0] = -1;
    g.legMirror[1] = 1;
    g.legMirror[2] = -1;
    g.legMirror[3] = 1;
}

GaitParams trotGait() {
    GaitParams g;
    g.period = 2000;              // ms (0.5 Hz)
    g.phaseDeg[0] = 0;
    g.phaseDeg[1] = 180;
    g.phaseDeg[2] = 180;
    g.phaseDeg[3] = 0;
    g.hipAmplitudeDeg = 18;       // réduit depuis 29° : les pattes avant/arrière du
                                  // même côté (hanches espacées de 6cm seulement)
                                  // se pinçaient près du corps à 29°

    g.hipNeutralDeg = 90;
    g.kneeNeutralDeg = 45;        // position "debout" validée en calibration (tient entre 40 et 50°)
    g.kneeLiftDeg = 15;           // levage du pied pendant le swing
    setLegMirror(g);
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
    g.hipAmplitudeDeg = 18;       // voir trotGait() : évite le pincement avant/arrière
    g.hipNeutralDeg = 90;
    g.kneeNeutralDeg = 45;
    g.kneeLiftDeg = 15;
    setLegMirror(g);
    return g;
}
