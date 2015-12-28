#ifndef VOISINAGE_H
#define VOISINAGE_H

#include "common.hpp"
#include "circuit.hpp"


void exchange(Solution* voisin,  Circuit* circuit1, Circuit* circuit2, int pos1, int pos2 , int id_recuit );
void take(Solution* voisin,  Circuit* circuit1, int pos1, Circuit* circuit2, int pos2 , int id_recuit );
void select_voisin(Solution* voisin, Solution* solution, int id_recuit );

#endif
//./
