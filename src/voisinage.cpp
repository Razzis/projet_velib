#include "voisinage.hpp"
#include "solution.hpp"


void exchange( Circuit* circuit1, Circuit* circuit2, int pos1, int pos2 ) {
	
	// on v�rfifie que les stations � �changer existent bien
	//if ( pos1 > circuit1->size() )||( pos2 > circuit2->size() ) {
		//U:die("�change impossible : l'un des circuits ne contient pas le nombre de stations suppos�");
	//}
	logn5("Voisin::exhange START");
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
	
	logn5("Voisin::exhange Insertion : TRY");
	logn5("Voisin::exhange Insertion : TRY");
	circuit1->insert( station2, pos1);
	circuit2->insert( station1, pos2);

	logn5("Voisin::exhange Insertion OK");

	circuit1->stations->erase(it1);
	circuit2->stations->erase(it2);

	circuit1->update(); // utile ?
	circuit2->update();
	logn5("Voisin::exhange END");

}


void take( Circuit* circuit1, int pos1, Circuit* circuit2, int pos2 ) {

	auto it1 = circuit1->stations->begin();
    for (int i = 0; i < pos1; ++i) {
        it1++;
    }
	auto it2 = circuit2->stations->begin();
    for (int i = 0; i < pos2; ++i) {
        it2++;
    }

	Station* station = *it1;
	circuit2->insert( station, pos2);
	circuit1->stations->erase(it1);

	circuit1->update(); // utile ?
	circuit2->update();

}


Solution* select_voisin( Solution* solution ) {

	Solution* voisin = new Solution(solution->inst);
	voisin->copy(solution);

	int maxtry = 10;

	double exch_or_take = ((double) rand() / (RAND_MAX));

	if ( exch_or_take >= 0.5 ) {
		// alors on fait un �change

		// choix rand des circuits qui vont s'�changer une station (pe �change de position sur un m�me circuit)
		int circuit_id1 = rand() % voisin->circuits->size();
		int circuit_id2 = rand() % voisin->circuits->size();
		logn2("Voisin::select_voisin EXHANGE");
		logn2("circuit_id1 : " + U::to_s(circuit_id1));
		logn2("circuit_id2 : " + U::to_s(circuit_id2));

		Circuit* c1 = voisin->circuits->at(circuit_id1);
		Circuit* c2 = voisin->circuits->at(circuit_id2);

		// choix rand des stations � �changer
		int station_id1 = rand() % c1->stations->size();
		int station_id2 = rand() % c2->stations->size();



		int trys = 0;
		/*while (( station_id2 !=  station_id1 -(1+station_id1)*abs(circuit_id1-circuit_id2) )&&( trys < maxtry )) {
			station_id2 = rand() % c2->stations->size();
			trys++;
		}*/
		logn2("station_id1 : " + U::to_s(station_id1));
		logn2("station_id2 : " + U::to_s(station_id2));
		// �change
		if (( circuit_id1 == circuit_id2 )&&( station_id1 == station_id2 )) {
			return voisin;
		} else {
			exchange( c1, c2, station_id1, station_id2 );
		}

	} else {
		// alors on fait un take

		// choix rand des circuits impliqu�s
		int from_id = rand() % voisin->circuits->size();
		int to_id = rand() % voisin->circuits->size();
		Circuit* from = voisin->circuits->at(from_id);
		Circuit* to = voisin->circuits->at(to_id);

		// choix rand de la station � prende
		int take_id = rand() % from->stations->size();
		int place_id;
		// choix rand de la position o� la mettre
		if (from_id == to_id) {
			place_id = rand() % to->stations->size();
		} else {
			place_id = rand() % (to->stations->size()+1);
		}

		//d�placement
		take(from,take_id,to,place_id);

	}

	voisin->update();
	return voisin;
}



//./
