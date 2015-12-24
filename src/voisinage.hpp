#ifndef VOISINAGE_H
#define VOISINAGE_H

#include "common.hpp"
#include "circuit.hpp"


void exchange( Circuit* circuit1, Circuit* circuit2, int pos1, int pos2 );
void take( Circuit* circuit1, int pos1, Circuit* circuit2, int pos2 );
Solution* select_voisin( Solution* solution );

#endif
//./
