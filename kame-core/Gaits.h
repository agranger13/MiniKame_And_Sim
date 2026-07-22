#ifndef gaits_h
#define gaits_h

#include "Kame.h"

// Trot diagonal : pattes avant-gauche+arriere-droite en phase, avant-droite+
// arriere-gauche en opposition. Ordre des pattes : 0=FL, 1=FR, 2=RL, 3=RR.
GaitParams trotGait();

// Demarche sequentielle ("wave") : chaque patte avance a son tour, dans
// l'ordre avant-gauche, avant-droite, arriere-gauche, arriere-droite.
GaitParams waveGait();

#endif
