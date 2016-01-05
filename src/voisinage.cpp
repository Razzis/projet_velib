#include "voisinage.hpp"
#include "solution.hpp"


void exchange(Solution* voisin,  Circuit* circuit1, Circuit* circuit2, int pos1, int pos2 , int id_recuit) {
	
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

	int old_desequilibre_c1 = circuit1->desequilibre;
	int old_length_c1 = circuit1->length;
	int old_desequilibre_c2 = circuit2->desequilibre;
	int old_length_c2 = circuit2->length;

	if(id_recuit == 1){
	//si on erase apres, les it ne sont plus bon ?
		circuit1->stations->erase(it1);
		circuit2->stations->erase(it2);
		circuit1->update();//pour que les cout du erase soit compté
		circuit2->update();
		//circuit1->insert( station2, pos1);
		//circuit2->insert( station1, pos2);

		circuit1->my_insert( station2);
		circuit2->my_insert( station1);
	}
	else{

		circuit1->insert( station2, pos1);
		circuit2->insert( station1, pos2);

		circuit1->stations->erase(it1);
		circuit2->stations->erase(it2);


	}

	logn5("Voisin::exhange Insertion OK");



	//circuit1->update(); // utile ?
	//circuit2->update();
	if(circuit1 != circuit2)
		voisin->Partial_update(circuit1,circuit2, old_desequilibre_c1, old_length_c1, old_desequilibre_c2, old_length_c2);
	else
		voisin->Partial_update(circuit1, old_desequilibre_c1, old_length_c1);
	logn5("Voisin::exhange END");

}


void take(Solution* voisin, Circuit* circuit1, int pos1, Circuit* circuit2, int pos2 , int id_recuit) {

	auto it1 = circuit1->stations->begin();
    for (int i = 0; i < pos1; ++i) {
        it1++;
    }
	auto it2 = circuit2->stations->begin();
    for (int i = 0; i < pos2; ++i) {
        it2++;
    }

	Station* station = *it1;
	//circuit2->insert( station, pos2);
	int old_desequilibre_c1 = circuit1->desequilibre;
	int old_length_c1 = circuit1->length;
	int old_desequilibre_c2 = circuit2->desequilibre;
	int old_length_c2 = circuit2->length;

	if(id_recuit == 1){
		circuit1->stations->erase(it1);
		circuit1->update();
		circuit2->my_insert(station);
	}
	else{
		circuit2->insert(station, pos2);
		circuit1->stations->erase(it1);

	}


	//circuit1->update(); // utile ?
	//circuit2->update();
	if(circuit1 != circuit2)
		voisin->Partial_update(circuit1,circuit2, old_desequilibre_c1, old_length_c1, old_desequilibre_c2, old_length_c2);
	else
		voisin->Partial_update(circuit1, old_desequilibre_c1, old_length_c1);

}


void select_voisin(Solution* voisin, Solution* solution , int id_recuit) {

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
			//return
		} else {
			exchange(voisin, c1, c2, station_id1, station_id2, id_recuit);
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
		take(voisin,from,take_id,to,place_id, id_recuit);


	}

}



//./
