#ifndef gaits_h
#define gaits_h

#include "Kame.h"

// Trot diagonal : pattes 1+4 en phase, pattes 2+3 en opposition de phase.
// Numérotation : 1=avant-droite, 2=avant-gauche, 3=arrière-droite, 4=arrière-gauche.
GaitParams trotGait();

// Démarche séquentielle ("wave") : chaque patte avance à son tour, dans
// l'ordre avant-droite, avant-gauche, arrière-droite, arrière-gauche.
GaitParams waveGait();

#endif
