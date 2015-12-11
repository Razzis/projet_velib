#include "circuit.hpp"

Circuit::Circuit(Instance* inst, Remorque* remorque) {
    // logn1("Circuit::Circuit: START " + remorque->name);
    this->inst = inst;
    this->remorque = remorque;
    this->stations = new list<Station*>();
    this->charge_init = floor(remorque->capa/2);
    this->charge_init_max = remorque->capa;
    this->charge_init_min = 0;
    this->desequilibre = 0;
    this->length = 0;

    this->depots = new map<Station*,int>();
    this->charges = new map<Station*,int>();

    charges_init_max = new map<Station*, int>();
    charges_init_min = new map<Station*, int>();
    charges_courante_max = new map<Station*, int>();
    charges_courante_min = new map<Station*, int>();

    // this->depots = new vector<int>();
    // this->charges = new vector<int>();
}
Circuit::Circuit(Instance* inst, Remorque* remorque, list<Station*>* stations)
        : Circuit(inst, remorque) {
    // logn1("Circuit::Circuit: START " + remorque->name +
    //                                  U::to_s(count(stations)) +
    //                                  "stations.");

    // this(inst, remorque); // INCORRECT
    this->stations->assign(stations->begin(), stations->end());
    /// this->depots->resize(this->stations->size(), 0);  // ssi vector
    /// this->charges->resize(this->stations->size(), 0); // ssi vector
}

// Attention : ceci n'est pas le constructeur par copie car il prend un
// **pointeur** en parametre
Circuit::Circuit(const Circuit* other) {
    this->inst = other->inst;
    this->remorque = other->remorque;
    this->stations = new list<Station*>(*other->stations);  // ATTENTION aux *
    this->charge_init = other->charge_init;
    this->desequilibre = other->desequilibre;
    this->length = other->length;

    this->depots = new map<Station*,int>(*other->depots);  // ATTENTION aux *
    this->charges = new map<Station*,int>(*other->charges);  // ATTENTION aux *
    // this->depots = new vector<int>(*other->depots);  // ATTENTION aux *
    // this->charges = new vector<int>(*other->charges);  // ATTENTION aux *
}

Circuit::~Circuit() {
    delete this->stations;
    delete this->depots;
    delete this->charges;
}

void Circuit::copy(Circuit* other) {
    this->remorque = other->remorque;
    this->stations->assign(other->stations->begin(), other->stations->end());
    this->charge_init = other->charge_init;
    this->desequilibre = other->desequilibre;
    this->length = other->length;

    // NON TESTÉ
    *(this->depots) = *(other->depots);
    *(this->charges) = *(other->charges);
}

void Circuit::partial_clear() {
    this->charge_init = 0;
    this->desequilibre = 0;
    this->length = 0;
    this->depots->clear();
    // this->depots->resize(this->stations->size(), 0); // ssi vector
    this->charges->clear();
    // this->charges->resize(this->stations->size(), 0); // ssi vector
}

void Circuit::clear() {
    this->partial_clear();
    this->stations->clear();
}

// Pour l'instant, ne fait rien d'autre que l'équilibrage
void Circuit::update() {
    logn5("Circuit::update BEGIN");
    // U::die("Circuit::update : non implémentée");
    this->partial_clear();
    logn6("Circuit::update: equilibage pour " + U::to_s(*remorque));
    this->equilibrate();

    // Mise à jour distance parcourue totale et déséquilibre global
    logn6("Circuit::update: mise à jour des distances du circuit " + U::to_s(*remorque));
    this->length = inst->get_dist(this->stations, this->remorque);
    logn5("Circuit::update END");
}

// Méthode d'équilibrage d'un circuit
//
void Circuit::equilibrate() {
    logn6("Circuit::equilibrate BEGIN");

    //on initialise la charge_max dans l'intervalle [0;capa]
    auto charge_init_max = this->remorque->capa;
    auto charge_init_min = 0;
    auto charge_init = -1;

    auto charge_courante_max = this->remorque->capa;
    auto charge_courante_min = 0;

    bool charge_init_finded = false;//vaut true si char_init_max==charge_init_min
    // U::die("Circuit::equilibrate : non implémentée");
    // on ajoute au fur et à mesure quand cela est possible

    auto iter = 0;//pour les logs

    for (auto it = this->stations->begin(); it != this->stations->end(); ++it) {
        Station* station = *it;
        logn7(station->to_s_long());
        logn7("Circuit::equilibrate: avant maj depots");


        // cas où il faut enlever des vélos
        if(station->deficit() < 0){
        	//si la charge_courante max est trop importante il faut la diminuer et adapter la charge_init en conséquence
        	if(this->remorque->capa - charge_courante_max < -station->deficit() && !charge_init_finded){
        		auto delta_charge_deficit = -this->remorque->capa - station->deficit() + charge_courante_max;
        		charge_courante_max -= delta_charge_deficit;
        		charge_init_max -= delta_charge_deficit;
        		if(charge_init_max<=charge_init_min)
        			charge_init_finded=true;
        		if (charge_init_max < 0)//recoupe le cas du dessus
        			charge_init_max = 0;
        	}
        	//soit on a trouvé une charge_courante_max qui correspond
        	if(this->remorque->capa - charge_courante_max >= -station->deficit()){
        		(*this->depots)[station] = station->deficit();
        		charge_courante_max += -station->deficit();
        		charge_courante_min += -station->deficit();
        		//(*this->charges)[station] = (*this->charges)[station-1] - station->deficit();
        	}
        	else if(this->remorque->capa - charge_courante_max < -station->deficit() && charge_init_finded){
        		(*this->depots)[station] = -(remorque->capa - charge_courante_max);
        		charge_courante_max += remorque->capa - charge_courante_max;
        		charge_courante_min += remorque->capa - charge_courante_max;
        		if(charge_courante_min > remorque->capa || charge_courante_max > remorque->capa)
        			U::die("\n charge_courante_min : " + U::to_s(charge_courante_min) + " charge_courante_max : " + U::to_s(charge_courante_max) + " taille remorque : " +U::to_s(remorque->capa));

        	}
        	else
        		U::die("Circuit::equilibrate : bug sur l'équilibrage1");
        	charge_init = charge_init_min;//si charge_init_finded == true, max=min, sinon on a le choix
        	if(charge_init_min < 0)
        		U::die("\n Circuit::equilibrate : charge init min <0 : " + U::to_s(charge_init_min));

        }

        // cas où il faut déposer des vélos
        else{
        	//si la charge_courante min est trop importante il faut la diminuer et adapter la charge_init en conséquence
        	if(charge_courante_min < station->deficit() && !charge_init_finded){
        		auto delta_charge_deficit = station->deficit() - charge_courante_min;
        		charge_courante_min += delta_charge_deficit;
        		charge_init_min += delta_charge_deficit;
        		if (charge_init_min > remorque->capa)
        			charge_init_min = remorque->capa;
        		if(charge_init_max<=charge_init_min)
        			charge_init_finded=true;
        	   	}
        	if(charge_init_max < 0)
        		logn2("\n Circuit::equilibrate : charge init max <0 : " + U::to_s(charge_init_max));

        	if(charge_courante_min >= station->deficit()){
        		charge_courante_max -= station->deficit();
        		charge_courante_min -= station->deficit();
        		if(charge_courante_min < 0 || charge_courante_min < 0)
        			U::die("\n charge_courante_min : " + U::to_s(charge_courante_min) + " charge_courante_max : " + U::to_s(charge_courante_max));
        		(*this->depots)[station] = station->deficit();
        		//(*this->charges)[station] = (*this->charges)[station-1] - station->deficit();
        	}
        	else if(charge_courante_min < station->deficit() && charge_init_finded)
        	{
        		(*this->depots)[station] = charge_courante_max;
        	    charge_courante_max = 0;
        	    charge_courante_min = 0;


        	}
        	else{
        		logn2("\n charge_courante_min : " + U::to_s(charge_courante_min) + "charge_courante_max : " + U::to_s(charge_courante_max));
        		U::die("Circuit::equilibrate : bug sur l'équilibrage2");
        	}
        	if(charge_init_max < 0)
        		logn2("\n Circuit::equilibrate : charge init max <0 : " + U::to_s(charge_init_max));
        	charge_init = charge_init_max;//si charge_init_finded == true, max=min, sinon on a le choix
        	if(charge_init_max < 0)
        		U::die("\n Circuit::equilibrate : charge init max <0 : " + U::to_s(charge_init_max));

        }
        (*this->charges_init_max)[station] = charge_init_max;
        (*this->charges_init_min)[station]= charge_init_min;
        (*this->charges_courante_max)[station] = charge_courante_max;
        (*this->charges_courante_max)[station] = charge_courante_min;

        if(charge_init_finded){
        	if (charge_init_max > charge_init_min){
        		this->charge_init_min = charge_init_min;
        		this->charge_init_max = charge_init_max;
        	}
        	else{
        		this->charge_init_min = charge_init;
        		this->charge_init_max = charge_init;
        	}
        	this->charge_init = charge_init;
        	break;
        }
        logn7("Circuit::equilibrate : iter" + U::to_s(iter) + " depots : " + U::to_s((*this->depots)[station]));
        iter++;
        //(*this->depots)[station] = 0;

        // le nouveau contenu de la remorque reste donc inchangé
        logn7("Circuit::equilibrate: avant maj charges");
        //this->charges->insert(std::pair<Station*,int>(station,this->charge_init));
        //(*this->charges)[station] = this->charge_init;


    }
    auto charge_courante = this->charge_init;
    for (auto it = this->stations->begin(); it != this->stations->end(); ++it) {
    	Station* station = *it;
    	if(station->deficit() > 0){
    		if (charge_courante >= station->deficit()){
    			(*this->depots)[station] = station->deficit();

    		}
    		else{
    			(*this->depots)[station] = charge_courante;
    			this->desequilibre += abs(charge_courante-station->deficit());
    		}
    	}
    	else{
    		if (remorque->capa - charge_courante >= -station->deficit()){
    		    	(*this->depots)[station] = station->deficit();
    		    }
    		else{
    		    	(*this->depots)[station] = -(remorque->capa - charge_courante);
    		    	this->desequilibre += abs(-station->deficit()-(remorque->capa - charge_courante));
    		    }
    	}
    	(*this->charges)[station] = charge_courante - (*this->depots)[station];
    	charge_courante = (*this->charges)[station];
    	if(charge_courante > remorque->capa )
    		U::die("circuit::equilibrate charge_courante>capa deficit : " + U::to_s(station->deficit()) + " charge_courante : " + U::to_s(charge_courante) + " capa : " + U::to_s(station->capa));

    	// incrémentation du desequilibre du circuit
        logn7("Circuit::equilibrate: avant maj desequilibre");


    }
    logn6("Circuit::equilibrate END");
}

// Insertion d'une station dans un circuit.
// si pos est absent : on ajoute à la fin de la liste des stations
//
// l'appel à ipdate est à la charge de l'appelant
//
list<Station*>::iterator Circuit::insert(Station* station, int pos) {
	list<Station*>::iterator  it_insert;
    logn5("Circuit::insert BEGIN " + station->name + " pos=" + U::to_s(pos));
    if (pos == -1) {
        it_insert = this->stations->insert(this->stations->end(), station);
    } else {
        // on avance l'itérateur jusqu'à la position pos
        auto it = this->stations->begin();
        for (int i = 0; i < pos; ++i) {
            it++;
        }
        it_insert = this->stations->insert(it, station);
    }
    logn5("Circuit::insert END");
    return it_insert;
}

// Cette brique d'insertion est opérationnelle (mais suppose que la brique
// d'équilibrage est faite), mais pas nécessairement efficace !
//
list<Station*>::iterator Circuit::insert_best(Station* station) {

	list<Station*>::iterator  it_insert;
	/** ensemble d'heuristique evitant la boucle dans certain cas **/
	bool utilisation_heuristique = false;
	/* si le desequilibre est nul et qu'a la fin du circuit il reste dans la
	 * remorque assez de vélo pour annuler le déficit de la station, on peut la
	 * mettre en back */


	/* si le desequilibre est nul et que on peut jouer (à la hausse  ou à la baisse)
	 * sur la charge initial de la remorque de manière à annuler entièrement le
	 * déficit de la station, on insert en FRONT */


	if(station->deficit() >= 0){
		Station* end = *(this->stations->end());
		if((*this->charges)[*this->stations->end()] >= station->deficit() && this->desequilibre == 0){
			it_insert = this->insert(station,-1);
			utilisation_heuristique = true;
		}
		else if(this->charge_init_max >= station->deficit()
						&& this->desequilibre == 0){
					it_insert = this->insert(station,0);
					utilisation_heuristique = true;
		}
	}
	else{
		if(this->remorque->capa - (*this->charges)[*this->stations->end()] >= -station->deficit() && this->desequilibre == 0){
			it_insert = this->insert(station,-1);
			utilisation_heuristique = true;
		}
		else if(this->remorque->capa - this->charge_init_min >= -station->deficit()
						&& this->desequilibre == 0){
					it_insert = this->insert(station,0);
					utilisation_heuristique = true;
				}

	}




	if( this->desequilibre == 0 && !utilisation_heuristique){

		cout << *this;

		cout << "------------------------------------" << endl;
		cout << "station " << *station << " à ajouter deficit : " << station->deficit() << endl;
		cout << "------------------------------------" << endl;
		U::die("###############################################");
	}





	/****************************************************************/
	if(utilisation_heuristique == false){
		Log::level += 0; // on peut modifier le level juste pour cette méthode...
		logn5("Circuit::insert_best BEGIN " + this->remorque->name +
			  " insertion de " + station->name);
		// U::die("Circuit::insert_best : non implémentée");
		int best_cost = 999999999;
		list<Station*>::iterator best_it;
		int best_pos = 0;
		int pos = 0;

		if (this->stations->size() == 0) {
			// circuit vide : il suffit d'insérer la nouvelle station à la fin
			logn6("Circuit::insert_best station VIDE => push_back simple !");
			it_insert = this->stations->insert(this->stations->end(), station);
		} else {
			for (auto it = this->stations->begin();
					  it != this->stations->end(); ++it) {
				Station* s = *it; // La station devant laquelle on va insérer "station"
				logn7("Circuit::insert_best de " + station->name +
						" avant " +  s->name);
				auto it2 = this->stations->insert(it, station);
				// On doit mettre à jour ce circuit avant d'en extraire le coût !
				this->update();
				int cost = this->get_cost();
				logn7("  Circuit::insert_best : "
					  " this_pos=" + U::to_s(pos) +
					  ", this_cost=" + U::to_s(cost) +
					  " (best_cost=" + U::to_s(best_cost) + ")");
				if (cost < best_cost) {
					best_cost = cost;
					best_pos = pos;
					best_it = it;
					logn6("Circuit::insert_best : MEILLEURE POSITION POUR L'INSTANT "
						  " avant name=" + s->name +
						  " : best_pos=" + U::to_s(pos) +
						  " => best_cost=" + U::to_s(cost));
				} else {
					// logn7("Circuit::insert_best : pas de record"
					//       " avant name=" + s->name +
					//       " this_pos=" + U::to_s(pos) +
					//       ", this_cost=" + U::to_s(cost) +
					//       " (best_cost=" + U::to_s(best_cost)) + ")";
				}
				// On remet le circuit en état avant de passer à station suivante
				this->stations->erase(it2);
				// A LA FIN
				pos++;
			}
			// On procède effectivement à la meilleure insertion
			logn6("Circuit::insert_best : "
				  "best_pos=" + U::to_s(best_pos) +
				  " avant name=" + (*best_it)->name +
				  " => get_cost=" + U::to_s(best_cost));

			it_insert = this->stations->insert(best_it, station);
		}
	}
    this->update();
    logn6("Circuit::insert_best circuit APRES insertion\n" + this->to_s_long());
    logn5("Circuit::insert_best END");
    Log::level -= 0; // ...on doit restaurer la modification du level

    return it_insert;


}

string Circuit::to_s() {
    stringstream buf;
    buf << "# Circuit associé à la remorque " <<  remorque->name <<
           " de capa " << remorque->capa << endl;
    buf << "#       id, charge_init, desequ, longueur\n";
    buf << "circuit " << remorque->name
        << "        " << this->charge_init
        << "        " << this->desequilibre
        << "       "  << this->length
        << endl;
    for (auto it = this->stations->begin(); it != this->stations->end(); it++) {
        Station* s = *it;
        // Différentes possibilités pour accéder à l'élément i
        // buf << "  " << s->name << " " << (*this->depots)[s]  << endl;
        buf << "  " << s->name << " " << this->depots->at(s) << endl;
    }
    buf << "end" << endl;
    return buf.str();
}
// Affichage détaillé d'une solution (format non standard mais plus détaillé) !
string Circuit::to_s_long() {
    stringstream buf;
    buf << "# Circuit détaillé associé à la remorque " <<  remorque->name <<
           " de capa "          << remorque->capa << endl;
    buf << "#   charge_init="   << this->charge_init <<endl;
    buf << "#   desequilibre="  << this->desequilibre <<endl;
    buf << "#   distance="      << this->length <<endl;

    Site* src = this->remorque;
    for (auto it = this->stations->begin(); it != this->stations->end(); ++it) {
        Station* dst = *it;
        Arc* arc = inst->get_arc(src, dst);
        int depot = this->depots->at((Station*)arc->dst);
        int charge = this->charges->at((Station*)arc->dst);
        buf << "   " <<  arc->to_s_long()
            << " depot de "  << depot
            << " => charge = " << charge << endl;
        src = dst;
    }
    if (stations->size() != 0) {
        buf << "   " <<  inst->get_arc(stations->back(), remorque)->to_s_long();
    }
    return buf.str();
}

void Circuit::erase_station(const Station& station, list<Station*>::iterator  it){
	logn5("Circuit::erase_station erasing Station " + U::to_s(station) + " from remorque : " + U::to_s(*(this->remorque)));
	this->stations->erase(it);
	logn5("Circuit::erase_station erasing done");
}

// Surchage de l'opérateur de flux pour afficher le contenue d'un circuit
std::ostream& operator<<(ostream &flux, Circuit const& circuit){
	flux << "###############################################" << endl;
	flux << "affichage du circuit associé à la remorque " << U::to_s(*(circuit.remorque)) << endl;
	flux << "charge initiale : " << circuit.charge_init << endl;
	flux << "charge initiale min : " << circuit.charge_init_min << endl;
	flux << "charge initiale max : " << circuit.charge_init_max << endl;
	flux << "taille de la remorque : " << circuit.remorque->capa << endl;
	for (auto it = circuit.stations->begin(); it != circuit.stations->end(); ++it) {
		Station* current_station = *it;
		flux << "------------------------------------" << endl;
		flux << U::to_s(*current_station) << endl;
		flux << "Station charge : " << (*circuit.charges)[current_station] << endl;
		flux << "Station depot : " << (*circuit.depots)[current_station] << endl;

		flux << "Station charge_courante_min : " << (*circuit.charges_courante_min)[current_station] << endl;
		flux << "Station charge_courante_max : " << (*circuit.charges_courante_max)[current_station] << endl;
		flux << "Station charge_init_min : " << (*circuit.charges_init_max)[current_station] << endl;
		flux << "Station charge_init_max : " << (*circuit.charges_init_min)[current_station] << endl;

	}
	flux << "------------------------------------" << endl;
	flux <<"###############################################" << endl;
    //Affichage des attributs

    return flux;

}
//./

