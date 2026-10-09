#include "Gaits.h"

// Sens de rotation de la hanche pour que "positif" = patte vers l'avant.
// Pattes gauches (femur vers +y) : avancer = rotation horaire (-z).
static void setHipDir(GaitParams& g) {
    g.hipDir[0] = -1; // FL
    g.hipDir[1] = 1;  // FR
    g.hipDir[2] = -1; // RL
    g.hipDir[3] = 1;  // RR
}

// Sens du genou pour que "positif" = patte ecartee/levee.
static void setKneeDir(GaitParams& g) {
    g.kneeDir[0] = 1;  // FL
    g.kneeDir[1] = -1; // FR
    g.kneeDir[2] = 1;  // RL
    g.kneeDir[3] = -1; // RR
}

// Cote de chaque patte, pour le virage differentiel : gauche = +1, droite = -1.
static void setTurnSign(GaitParams& g) {
    g.turnSign[0] = 1;  // FL
    g.turnSign[1] = -1; // FR
    g.turnSign[2] = 1;  // RL
    g.turnSign[3] = -1; // RR
}

GaitParams trotGait() {
    GaitParams g;
    g.period = 600;              // ms
    g.phaseDeg[0] = 0;            // FL
    g.phaseDeg[1] = 180;          // FR
    g.phaseDeg[2] = 180;          // RL
    g.phaseDeg[3] = 0;            // RR
    g.hipAmplitudeDeg = 20;       // ~0.35 rad
    g.kneeAmplitudeDeg = 17;      // ~0.30 rad
    g.kneeOffsetDeg = 14;         // ~0.25 rad
    g.kneeSwingWindowDeg = 360;   // genou toujours en mouvement : trot = gait dynamique,
                                  // 2 pattes diagonales en appui suffisent a tout instant
    setHipDir(g);
    setKneeDir(g);
    setTurnSign(g);
    return g;
}

GaitParams waveGait() {
    GaitParams g;
    g.period = 2400;              // ms — plus lent que le trot, une patte a la fois
    // Ordre : avant-gauche (0 deg) -> avant-droite (90) -> arriere-gauche (180) -> arriere-droite (270)
    g.phaseDeg[0] = 0;
    g.phaseDeg[1] = 90;
    g.phaseDeg[2] = 180;
    g.phaseDeg[3] = 270;
    g.hipAmplitudeDeg = 20;
    g.kneeAmplitudeDeg = 17;
    g.kneeOffsetDeg = 14;
    // Gait sequentiel : une seule patte leve a la fois, les 3 autres doivent
    // rester des appuis fixes pour la stabilite statique. Les pattes sont
    // espacees de 90 deg -> une fenetre de 90 deg fait lever exactement une
    // patte a la fois (les 3 autres figees a kneeOffsetDeg).
    g.kneeSwingWindowDeg = 90;
    setHipDir(g);
    setKneeDir(g);
    setTurnSign(g);
    return g;
}
