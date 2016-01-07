#ifndef VOISINAGE_H
#define VOISINAGE_H

#include "common.hpp"
#include "circuit.hpp"


void exchange( Solution* solution, Circuit* circuit1, Circuit* circuit2, int pos1, int pos2 );
void take( Solution* solution, Circuit* circuit1, int pos1, int pos2, Circuit* circuit2, int pos3 );
void move( Solution* solution, Circuit* circuit, int pos1, int pos2, int pos3 );
void reverse( Solution* solution, Circuit* circuit, int pos1, int pos2 );
Solution* select_voisin( Solution* solution, Solution* voisin );
void compute_proba_circuit(Circuit* circuit, map<Station*, double> &proba);
void select_station( Circuit* circuit, map<Station*, double> &proba, int& pos1);
void select_station( Circuit* circuit, map<Station*, double> &proba, int& pos1, int& pos2);
void select_station( Circuit* circuit, map<Station*, double> &proba, int& pos1, int& pos2, int& pos3);

//Solution* select_voisin2( Solution* solution, Solution* voisin, int iter, int itermax );

#endif
//./
