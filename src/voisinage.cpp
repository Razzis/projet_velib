#include "voisinage.hpp"
#include "solution.hpp"


void exchange( Solution* solution, Circuit* circuit1, Circuit* circuit2, int pos1, int pos2 ) {
	// rq : on passe le solution en arg pour faire des partial update plus rapide, sinon pas beosin

    int old_desequilibre_c1 = circuit1->desequilibre;
    int old_length_c1 = circuit1->length;
    int old_desequilibre_c2 = circuit2->desequilibre;
    int old_length_c2 = circuit2->length;

	auto it1 = circuit1->stations->begin();
    for (int i = 0; i < pos1; ++i) {
        it1++;
    }
	auto it2 = circuit2->stations->begin();
    for (int i = 0; i < pos2; ++i) {
        it2++;
    }



	Station* station1 = *it1;
	Station* station2 = *it2;
	
	circuit1->insert( station2, pos1);
	circuit2->insert( station1, pos2);

	circuit1->stations->erase(it1);
	circuit2->stations->erase(it2);

	solution->Partial_update(circuit1,circuit2,old_desequilibre_c1,old_length_c1,old_desequilibre_c2,old_length_c2);

	// if (circuit1!=circuit2) {
	// 	solution->Partial_update( circuit1, circuit2, old_desequilibre_c1, old_length_c1, old_desequilibre_c2, old_length_c2);
	// } else {
	// 	solution->Partial_update( circuit1, old_desequilibre_c1, old_length_c1);
	// }

}


void take( Solution* solution, Circuit* circuit1, int pos1, int pos2, Circuit* circuit2, int pos3 ) {

	int old_desequilibre_c1 = circuit1->desequilibre;
	int old_length_c1 = circuit1->length;
	int old_desequilibre_c2 = circuit2->desequilibre;
	int old_length_c2 = circuit2->length;

	auto it1 = circuit1->stations->begin();
    for (int i = 0; i < pos1; ++i) {
        it1++;
    }

	Station* station = *it1;;

	for (int i = pos1; i <= pos2; i++) {
		station = *it1;
		circuit2->insert( station, pos3+i-pos1);
		auto it2 = (it1--)++;
		circuit1->stations->erase(it2);
		it1++;
	}

	solution->Partial_update(circuit1,circuit2,old_desequilibre_c1,old_length_c1,old_desequilibre_c2,old_length_c2);

}



void move( Solution* solution, Circuit* circuit, int pos1, int pos2, int pos3 ) {

	int old_desequilibre = circuit->desequilibre;
	int old_length = circuit->length;

	auto it1 = circuit->stations->begin();
    for (int i = 0; i < pos1; ++i) {
        it1++;
    }

	Station* station = *it1;

	for (int i = pos1; i <= pos2; i++) {
		station = *it1;
		if (pos3<pos1) {
			circuit->insert( station, pos3+i-pos1);
			auto it2 = (it1--)++;
			circuit->stations->erase(it2);
		} else if (pos3>pos2) {
			circuit->insert( station, pos3);
			auto it2 = (it1--)++;
			circuit->stations->erase(it2);
		}
		it1++;
	}

	solution->Partial_update(circuit,old_desequilibre,old_length);

}



void reverse( Solution* solution, Circuit* circuit, int pos1, int pos2 ) {

	int old_desequilibre = circuit->desequilibre;
	int old_length = circuit->length;

	auto it2 = circuit->stations->begin();
    for (int i = 0; i < pos2; ++i) {
        it2++;
    }

    Station* station;
    for (int i = pos2; i >= pos1; --i) {
		station = *it2;
		circuit->insert( station, pos2+1) ;//+pos2-i);
		auto it1 = (it2--)++;
		circuit->stations->erase(it1);
	}

    solution->Partial_update(circuit,old_desequilibre,old_length);
}




Solution* select_voisin( Solution* solution, Solution* voisin ) {
	voisin->clear();
	voisin->copy(solution);

	// on tire 2 circuits au hasard
	int circuit_id1 = rand() % voisin->circuits->size();
	int circuit_id2 = rand() % voisin->circuits->size();


	if (circuit_id1 != circuit_id2) {
		// si les 2 circuits sont distincts, on va prendre des stations � l'un pour les donner � l'autre
		// ie faire un take

		// r�cup des circuits et calcul des probas associ�es
		Circuit* circuit1 = voisin->circuits->at(circuit_id1);
		Circuit* circuit2 = voisin->circuits->at(circuit_id2);
		map<Station*, double> proba1;
		map<Station*, double> proba2;
		compute_proba_circuit(circuit1, proba1);
		compute_proba_circuit(circuit2, proba2);

		// selection des stations via leur position
		int pos3 = select_station(circuit2, proba2);
		int pos1 = select_station(circuit1, proba1);
		int pos2 = select_station(circuit1, proba1);
		

		// take
		take( voisin, circuit1, pos1, pos2, circuit2, pos3 );

		} else if (circuit_id1 == circuit_id2) {
		// si on a tir� 2 fois le m�me circuit, on peut soit d�placer des stations, soit retourner une seq de stations
		// ie faire un move ou un reverse

		// r�cup du circuit et calcul des probas associ�es
		Circuit* circuit = voisin->circuits->at(circuit_id1);
		map<Station*, double> proba;
		compute_proba_circuit(circuit, proba);

		// tirage random pour savoir si on reverse ou move
		double reverse_or_move = double(rand())/double(RAND_MAX);

		if (reverse_or_move<=0.5) {
			// selection des stations via leur position
			int pos1 = select_station(circuit, proba);
			int pos2 = select_station(circuit, proba);
			
			// reverse
			reverse( voisin, circuit, min(pos1,pos2), max(pos1,pos2) );

		} else {
			// selection des stations via leur position
			int pos1 = select_station(circuit, proba);
			int pos2 = select_station(circuit, proba);
			int pos3 = select_station(circuit, proba);

			// move (s'assurer de l'ordre valide des int pos)
			if (pos3<=min(pos1,pos2) || pos3>=max(pos1,pos2)) {
				move( voisin, circuit, min(pos1,pos2), max(pos1,pos2), pos3 );
			} else {
				move( voisin, circuit, min(pos1,pos2), pos3, max(pos1,pos2) );
			}

		}
	}



	return voisin;
}
























void compute_proba_circuit(Circuit* circuit, map<Station*, double> &proba) {

	double sum_proba = 0.0;

	// ce gros caca est hyperlourd en calcul mais peut �tre fait � la cr�ation de l'instance,
	// en gros faut que je le d�place ********************************************************
	int min_dist = 9999999;
	Site* here;
	Site* a;
	Site* b;
	Site* c;
	Station* hereaussi;
	for ( auto it = circuit->stations->begin(); it != circuit->stations->end(); it++ ) {
		here = *it;
		min_dist = 9999999;
		for ( auto jt = circuit->inst->stations->begin(); jt != circuit->inst->stations->end(); jt++) {
		  	a = *jt;
			for ( auto kt = circuit->inst->stations->begin(); kt != circuit->inst->stations->end(); kt++) {
		  		b = *kt;
		  		if (a!=here && b!=here && a!=b) {
		    		//min_dist = (*this->inst->dists_grid)[here->sid * this->inst->nb_sites + s->sid];
		    		min_dist = min(min_dist, circuit->inst->get_dist(here,a)+circuit->inst->get_dist(here,b));
		  		}
		  	}
		  	for (auto rq = circuit->inst->remorques->begin(); rq != circuit->inst->remorques->end(); rq++) {
		  		c = *rq;
		  		if (a!=here) {
		  			min_dist = min(min_dist, circuit->inst->get_dist(here,a)+circuit->inst->get_dist(here,c));
		  			//min_dist = min(min_dist, circuit->inst->get_dist(here,c)+circuit->inst->get_dist(here,c));
		  		}
		  	}
		}
		sum_proba = sum_proba - min_dist;
		hereaussi = *it;
		proba[hereaussi] = -min_dist;
	}
	// ******************************************************************************************



	// ce bout de caca ci est aussi trop lourd sur les grosses instances, mais lui il doit �tre recalcul� ici � chaque fois...
	Station* src = *circuit->stations->begin();
	auto st = circuit->stations->begin();

	proba[src] = circuit->inst->get_dist(circuit->remorque, src) + proba[src];
	sum_proba = sum_proba + circuit->inst->get_dist(circuit->remorque,src);
    while (src != circuit->stations->back() && circuit->stations->size()!=1) {
    	st++;
        Station* dst = *st;
        proba[src] = circuit->inst->get_dist(src, dst) + proba[src];
        proba[dst] = circuit->inst->get_dist(src, dst) + proba[dst];
        sum_proba = sum_proba + 2*circuit->inst->get_dist(src, dst);
        src = dst;
	}
	proba[src] = circuit->inst->get_dist(src, circuit->remorque) + proba[src];
	sum_proba = sum_proba + circuit->inst->get_dist(src, circuit->remorque);

	for ( auto it = circuit->stations->begin(); it != circuit->stations->end(); it++ ) {
		Station* hereaussi = *it;
		proba[hereaussi] = double(proba[hereaussi]+1)/(double(sum_proba)+double(circuit->stations->size()));
	}
}









int select_station( Circuit* circuit, map<Station*,double> &proba) {
	double p = double(rand())/double(RAND_MAX);
	int pos = 0;
	double sum_proba = 0.0;
	auto st = circuit->stations->begin();
	while (!((sum_proba <= p) && (p < (sum_proba + proba[*st])))) {
		sum_proba = sum_proba + proba[*st];
		st++;
		pos++;
	}

	return pos;
}






























/*
Solution* select_voisin2( Solution* solution, Solution* voisin, int iter, int itermax ) {

	voisin->copy(solution);

	if (iter <= (double) itermax*1/3) {
		// alors on fait un �change

		// choix rand des circuits qui vont s'�changer une station
		int circuit_id1 = rand() % voisin->circuits->size();
		int circuit_id2 = rand() % (voisin->circuits->size()-1);
		if (circuit_id1 == circuit_id2) {
			circuit_id2 = (circuit_id2+1)%voisin->circuits->size();
		}
		Circuit* c1 = voisin->circuits->at(circuit_id1);
		Circuit* c2 = voisin->circuits->at(circuit_id2);

		// choix rand des stations � �changer
		int station_id1 = rand() % c1->stations->size();
		int station_id2 = rand() % c2->stations->size();

		// �change
		exchange( voisin, c1, c2, station_id1, station_id2 );

	} else if (iter <= (double) itermax*2/3) {
		// alors on fait un take/

		// choix rand des circuits impliqu�s
		int from_id = rand() % voisin->circuits->size();
		int to_id = rand() % (voisin->circuits->size()-1);
		if (from_id == to_id) {
			to_id = (from_id+1)%voisin->circuits->size();
		}
		Circuit* from = voisin->circuits->at(from_id);
		Circuit* to = voisin->circuits->at(to_id);

		// choix rand de la station � prende
		int take_id = rand() % from->stations->size();
		// choix rand de la position o� la mettre
		int place_id = rand() % (to->stations->size()+1);

		//d�placement
		take(voisin, from,take_id,to,place_id);

	} else {
		// alors on fait un move

		int circuit_id = rand() % voisin->circuits->size();
		Circuit* c = voisin->circuits->at(circuit_id);
		int station_id1 = rand() % c->stations->size();
		int station_id2 = rand() % c->stations->size();

		move( voisin, c, station_id1, station_id2 );
	}

	voisin->update();
	return voisin;
}
*/

//./
