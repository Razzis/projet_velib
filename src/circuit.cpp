#include "circuit.hpp"

Circuit::Circuit(Instance* inst, Remorque* remorque) {
    // logn1("Circuit::Circuit: START " + remorque->name);
    this->inst = inst;
    this->remorque = remorque;
    this->stations = new list<Station*>();
    this->charge_init = floor(float(remorque->capa)/2);
    this->charge_init_max = remorque->capa;
    this->charge_init_min = 0;
    this->desequilibre = 0;
    this->length = 0;

    this->desequilibre_max = 0;
    this->desequilibre_min = 0;

    this->iterateur2desequilibreMax = 0;
    this->iterateur2desequilibreMin = 0;

    this->depots = new map<Station*,int>();
    this->charges = new map<Station*,int>();


    this->charges_init_max = new map<Station*, int>();
    this->charges_init_min = new map<Station*, int>();
    this->charges_courante_max = new map<Station*, int>();
    this->charges_courante_min = new map<Station*, int>();
    this->desequilibre_courant = new map<Station*,int>();

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
    this->charge_init_max = other->charge_init_max;
    this->charge_init_min = other->charge_init_min;
    this->desequilibre = other->desequilibre;
    this->length = other->length;

    this->charges_init_max = new map<Station*, int>();
    this->charges_init_min = new map<Station*, int>();
    this->charges_courante_max = new map<Station*, int>();
    this->charges_courante_min = new map<Station*, int>();
    this->desequilibre_courant = new map<Station*,int>();

    this->depots = new map<Station*,int>(*other->depots);  // ATTENTION aux *
    this->charges = new map<Station*,int>(*other->charges);  // ATTENTION aux *
    for(auto it = other->stations->begin(); it != other->stations->end(); ++it){
    	Station* station = *it;

    	this->stations->push_back(station);
    	(*this->charges_init_max)[station] = (*other->charges_init_max)[station];
    	(*this->charges_init_min)[station] = (*other->charges_init_min)[station];
    	(*this->charges_courante_max)[station] = (*other->charges_courante_max)[station];
    	(*this->charges_courante_min)[station] = (*other->charges_courante_min)[station];
    	(*this->desequilibre_courant)[station] = (*other->desequilibre_courant)[station];

    	(*this->depots)[station] = (*other->depots)[station];  // ATTENTION aux *
    	(*this->charges)[station] = (*other->charges)[station];  // ATTENTION aux *

    }
    // this->depots = new vector<int>(*other->depots);  // ATTENTION aux *
    // this->charges = new vector<int>(*other->charges);  // ATTENTION aux *
}

Circuit::~Circuit() {
    delete this->stations;
    delete this->depots;
    delete this->charges;

    delete this->charges_init_max;
    delete this->charges_init_min;
    delete this->charges_courante_max;
    delete this->charges_courante_min;
    delete this->desequilibre_courant;
}

void Circuit::copy(Circuit* other) {
    this->remorque = other->remorque;
    this->stations->assign(other->stations->begin(), other->stations->end());
    this->charge_init = other->charge_init;
    this->desequilibre = other->desequilibre;
    this->length = other->length;

    // NON TESTÉ
    *(this->charges_init_max) = *(other->charges_init_max);
    *(this->charges_init_min) = *(other->charges_init_min);
    *(this->charges_courante_max) = *(other->charges_courante_max);
    *(this->charges_courante_min) = *(other->charges_courante_min);
    *(this->desequilibre_courant) = *(other->desequilibre_courant);
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


        		if (charge_init_max < 0)//recoupe le cas du dessus
        			charge_init_max = 0;
        		if(charge_init_max<=charge_init_min){
        			charge_init_finded=true;
        			charge_init_max = charge_init_min;
        		}
        		logn5("Circuit::equilibrate charge_courante_max : " + U::to_s(charge_courante_max));
        		logn5("Circuit::equilibrate charge_init_max : " + U::to_s(charge_init_max));
        		logn5("Circuit::equilibrate charge_courante_min : " + U::to_s(charge_courante_min));
        		logn5("Circuit::equilibrate charge_init_min : " + U::to_s(charge_init_min));
        	}
        	//soit on a trouvé une charge_courante_max qui correspond
        	if(this->remorque->capa - charge_courante_max >= -station->deficit() && !charge_init_finded){
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
        	/*else
        		U::die("Circuit::equilibrate : bug sur l'équilibrage1");*/
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
        		if(charge_init_max<=charge_init_min){
        			charge_init_finded=true;
        			charge_init_min = charge_init_max;
        		}
        		logn5("Circuit::equilibrate charge_courante_max : " + U::to_s(charge_courante_max));
        		logn5("Circuit::equilibrate charge_init_max : " + U::to_s(charge_init_max));
        		logn5("Circuit::equilibrate charge_courante_min : " + U::to_s(charge_courante_min));
        		logn5("Circuit::equilibrate charge_init_min : " + U::to_s(charge_init_min));
        	}
        	if(charge_init_max < 0)
        		logn2("\n Circuit::equilibrate : charge init max <0 : " + U::to_s(charge_init_max));

        	if(charge_courante_min >= station->deficit() && !charge_init_finded){
        		charge_courante_max -= station->deficit();
        		charge_courante_min -= station->deficit();
        		if(charge_courante_min < 0 || charge_courante_max < 0)
        			U::die("DIE Circuit::equilibrate \n charge_courante_min : " + U::to_s(charge_courante_min) + " charge_courante_max : " + U::to_s(charge_courante_max));
        		(*this->depots)[station] = station->deficit();
        		//(*this->charges)[station] = (*this->charges)[station-1] - station->deficit();
        	}
        	else if(charge_courante_min < station->deficit() && charge_init_finded)//min=max
        	{
        		(*this->depots)[station] = charge_courante_max;
        	    charge_courante_max = 0;
        	    charge_courante_min = 0;
        	}
        	/*else{
        		logn2("\n charge_courante_min : " + U::to_s(charge_courante_min) + "charge_courante_max : " + U::to_s(charge_courante_max));
        		U::die("Circuit::equilibrate : bug sur l'équilibrage2");
        	}*/
        	if(charge_init_max < 0)
        		logn2("\n Circuit::equilibrate : charge init max <0 : " + U::to_s(charge_init_max));
        	charge_init = charge_init_max;//si charge_init_finded == true, max=min, sinon on a le choix
        	if(charge_init_max < 0)
        		U::die("\n Circuit::equilibrate : charge init max <0 : " + U::to_s(charge_init_max));
        }

        (*this->charges_init_max)[station] = charge_init_max;
        (*this->charges_init_min)[station]= charge_init_min;
        (*this->charges_courante_max)[station] = charge_courante_max;
        (*this->charges_courante_min)[station] = charge_courante_min;

        if(charge_init_finded){

        	this->charge_init_min = charge_init;
        	this->charge_init_max = charge_init;

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
    /* si les charge min et max ne se sont pas croisé à la fin de la boucle) */
    if(!charge_init_finded){

    	this->charge_init_min = charge_init_min;
        this->charge_init_max = charge_init_max;

        this->charge_init = floor(float(charge_init_min+charge_init_max)/2);

    }
    this->maj_Depots();
    logn6("Circuit::equilibrate END");
}

int Circuit::Partial_equilibrate(Station* s, list<Station*>::iterator insert_it, list<Station*>::iterator& it_second_part,
	bool& SecondPartExiste, bool& stationAddedAsFirstOF2Part, int& deficit_from_partial_update, int& desequilibre_before_SecondPart){
	Station* insert_station = *insert_it;
	desequilibre_before_SecondPart = 0;
	logn5("Circuit::Partial_equilibrate BEGIN");
	logn5("Circuit::Partial_equilibrate of "+U::to_s(*s)+ "From " +U::to_s(*insert_station));

	SecondPartExiste = false;
	stationAddedAsFirstOF2Part = false;
	int charge_init_max = (*this->charges_init_max)[insert_station];
	int charge_init_min = (*this->charges_init_min)[insert_station];
	if(charge_init_min >= charge_init_max)
		U::die("Circuit::Partial_equilibrate charge min >= charge_max");

	auto charge_init = -1;

	auto charge_courante_max = (*this->charges_courante_max)[insert_station];
	auto charge_courante_min = (*this->charges_courante_min)[insert_station];

	bool arret_boucle = false;
	list<Station*>::iterator it=insert_it;

	if(insert_it != this->stations->end()){
		++it;
		logn2("Circuit::partial_equilibrate MAJ it avant la boucle");
	}
	else
		arret_boucle = true;

	bool first_iteration = true;
	logn2("Circuit::Partial_Equilibrate début de la boucle");
	while(!arret_boucle || first_iteration){
		Station* station;
		if(first_iteration)
			station = s;
		else{
			station = *it;
			desequilibre_before_SecondPart += (*this->desequilibre_courant)[station];
			if(desequilibre_before_SecondPart != 0)
				logn5("Circuit::Partial_equilibrate before second part sttion : "+U::to_s(*station)+" before  "+U::to_s(desequilibre_before_SecondPart));
		}

		if(station->deficit() > 0){
			logn2("Circuit::Partial_Equilibrate deficit positif de la station "+U::to_s(*station)+" : "+U::to_s(station->deficit()));
			if(charge_courante_min > station->deficit()){
				logn2("Circuit::Partial_Equilibrate charge_courante_min > station->deficit() charge_courante_min : "+U::to_s(charge_courante_min));
				charge_courante_min -= station->deficit();
				charge_courante_max -= station->deficit();
			}
			else{
				logn2("Circuit::Partial_Equilibrate charge_courante_min <= station->deficit() charge_courante_min : "+U::to_s(charge_courante_min));
				charge_init_min += station->deficit()-charge_courante_min;
				charge_courante_max -= station->deficit();
				charge_courante_min = 0;
				if(charge_init_max <= charge_init_min){
					logn2("Circuit::Partial_Equilibrate charge_courante_max <= charge_courante_min (charge_courante_min,charge_courante_max) : ("
							+U::to_s(charge_courante_min)+","+U::to_s(charge_courante_max)+")");
					if(first_iteration)
						stationAddedAsFirstOF2Part = true;
					it_second_part = it;
					SecondPartExiste = true;
					//TODO  : a expliquer
					//deficit_from_partial_update = charge_courante_min
					//		- ( (*this->charges_courante_min)[station] - ( (*this->charges_init_min)[station] - charge_init_max ) );
					if(first_iteration){
						deficit_from_partial_update = -s->deficit();//charge_courante_max
								//- ( (*this->charges_courante_max)[insert_station] - ((*this->charges_init_max)[insert_station] - charge_init_max ) );
					}
					else{
						desequilibre_before_SecondPart -= (*this->desequilibre_courant)[station];//enlever le desequilibre de la première station de SecondPart
						if(desequilibre_before_SecondPart != 0)
							logn5("Circuit::Partial_equilibrate before second part sttion : "+U::to_s(*station)+" before  "+U::to_s(desequilibre_before_SecondPart));

						deficit_from_partial_update = -s->deficit();//charge_courante_max
							//- ( (*this->charges_courante_max)[station] - ((*this->charges_init_max)[station] - charge_init_max ) );
					}
					return charge_init_max;
				}
			}
		}
		if(station->deficit() < 0){
			logn2("Circuit::Partial_Equilibrate deficit négatif "+U::to_s(*station)+" : "+U::to_s(station->deficit()));
			if(this->remorque->capa - charge_courante_max > -station->deficit()){
				logn2("Circuit::Partial_Equilibrate this->remorque->capa - charge_courante_max > -station->deficit() charge_courante_max : "+U::to_s(charge_courante_max));
				charge_courante_max += -station->deficit();
				charge_courante_min += -station->deficit();
			}
			else{
				logn2("Circuit::Partial_Equilibrate this->remorque->capa - charge_courante_max <= -station->deficit() charge_courante_max : "+U::to_s(charge_courante_max));
				charge_init_max -= -station->deficit() - (this->remorque->capa - charge_courante_max);
				charge_courante_min += -station->deficit();
				charge_courante_max = this->remorque->capa;
				if(charge_init_max <= charge_init_min){
					logn2("Circuit::Partial_Equilibrate charge_courante_max <= charge_courante_min (charge_courante_min,charge_courante_max) : ("
							+U::to_s(charge_courante_min)+","+U::to_s(charge_courante_max)+")");
					if(first_iteration)
						stationAddedAsFirstOF2Part = true;
					it_second_part = it;
					SecondPartExiste = true;
					//TODO : expliquer
					if(first_iteration){
						deficit_from_partial_update = -s->deficit();//charge_courante_min
								//- ( (*this->charges_courante_min)[insert_station] - ((*this->charges_init_min)[insert_station] - charge_init_min ) );
					}
					else{
						desequilibre_before_SecondPart -= (*this->desequilibre_courant)[station];//enlever le desequilibre de la première station de SecondPart
						if(desequilibre_before_SecondPart != 0)
							logn5("Circuit::Partial_equilibrate before second part sttion : "+U::to_s(*station)+" before  "+U::to_s(desequilibre_before_SecondPart));

						deficit_from_partial_update = -s->deficit();//charge_courante_min
							//- ( (*this->charges_courante_min)[station] - ((*this->charges_init_min)[station] - charge_init_min ) );
					}
					//deficit_from_partial_update = charge_courante_max
					//		- ( (*this->charges_courante_max)[station] - ((*this->charges_init_max)[station] - charge_init_min ) );
					return charge_init_min;
				}
			}
		}
		logn2("Circuit::Partial_Equilibrate (charge_courante_min,charge_courante_max) : "
									+U::to_s(charge_courante_min)+","+U::to_s(charge_courante_max)+")");
		logn2("Circuit::Partial_Equilibrate (charge_init_min,charge_init_max) : "
									+U::to_s(charge_init_min)+","+U::to_s(charge_init_max)+")");
		//cout << "Stations : " << U::to_s(*station) << endl;
		//cout << *this << endl;
		if(first_iteration)
			first_iteration = false;
		else{
			logn2("Circuit::Partial_equilibre iterator increase");
			it++;
		}
		if(it == this->stations->end())
			arret_boucle = true;
	}

	return 1;

}


void Circuit::maj_Depots(){
	bool SecondPart = false;
    auto charge_courante = this->charge_init;
    this->desequilibre = 0;
    logn3("Circuit::maj_depot START");
    int iterateur = 1;
    for (auto it = this->stations->begin(); it != this->stations->end(); ++it) {
    	Station* station = *it;
    	logn3("Circuit::maj_depot charge_courante : " + U::to_s(charge_courante));
    	logn3("Circuit::maj_depot deficit : " + U::to_s(station->deficit()));
    	logn3("Circuit::maj_depot de la station : " + U::to_s(*station));
    	if(station->deficit() > 0){
    		if (charge_courante >= station->deficit()){
    			(*this->desequilibre_courant)[station] = 0;
    			(*this->depots)[station] = station->deficit();

    		}
    		else{
    			/*if((*this->charges_init_max)[station] != (*this->charges_init_min)[station]){
    		    	cout << "########" << endl << *this << endl;
    		    	cout << "charge_init_max : " << (*this->charges_init_max)[station] << endl;
    		    	cout << "charge_init_min : " << (*this->charges_init_min)[station] << endl;
    		    	cout << "charge_courante : " << charge_courante << endl;
    		    	cout << " deficit : " << station->deficit() <<endl;
    				U::die("Circuit::maj_depot charge_init_max!=charge_iit_max");
    			}*/
    			(*this->depots)[station] = charge_courante;
    			(*this->desequilibre_courant)[station] = station->deficit()-charge_courante;
    			this->desequilibre += abs(charge_courante-station->deficit());
    		}
    	}
    	else{
    		if (this->remorque->capa - charge_courante >= -station->deficit()){
    			logn3("Circuit::majDepot remrque_capa : "+U::to_s(this->remorque->capa));
    			(*this->desequilibre_courant)[station] = 0;
    				(*this->depots)[station] = station->deficit();
    		    }
    		else{
    			/*if((*this->charges_init_max)[station] != (*this->charges_init_min)[station]){
    		    	cout << "########" << endl << *this << endl;
    		    	cout << "charge_init_max : " << (*this->charges_init_max)[station] << endl;
    		    	cout << "charge_init_min : " << (*this->charges_init_min)[station] << endl;
    			    U::die("Circuit::maj_depot charge_init_max!=charge_iit_max");
    			}*/
    		    	(*this->depots)[station] = -(this->remorque->capa - charge_courante);
    		    	(*this->desequilibre_courant)[station] = -(-station->deficit()-(this->remorque->capa - charge_courante));
    		    	this->desequilibre += abs(-station->deficit()-(this->remorque->capa - charge_courante));
    		    }
    	}

    	(*this->charges)[station] = charge_courante - (*this->depots)[station];
    	charge_courante = (*this->charges)[station];
    	if((*this->charges_init_max)[station] == (*this->charges_init_min)[station])
    		SecondPart = true;
    	if(SecondPart){
			(*this->charges_init_max)[station] = this->charge_init;
			(*this->charges_init_min)[station]= this->charge_init;
			(*this->charges_courante_max)[station] = charge_courante;
			(*this->charges_courante_min)[station] = charge_courante;
    	}

    	if((*this->desequilibre_courant)[station] > this->desequilibre_max){
    		this->desequilibre_max = (*this->desequilibre_courant)[station];
    		this->iterateur2desequilibreMax = iterateur;
    	}
    	if((*this->desequilibre_courant)[station] < this->desequilibre_min){
    		this->desequilibre_min = (*this->desequilibre_courant)[station];
    		this->iterateur2desequilibreMin = iterateur;
    	}
    	iterateur++;


    	if(charge_courante > this->remorque->capa )
    		U::die("circuit::equilibrate charge_courante>capa deficit : " + U::to_s(station->deficit()) + " charge_courante : " + U::to_s(charge_courante) + " capa : " + U::to_s(station->capa));

    	// incrémentation du desequilibre du circuit
        logn3("Circuit::majDepot end, desequilibre : "+ U::to_s(this->desequilibre));


    }
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

int Circuit::insertCost(Station* station, int pos) {
	list<Station*>::iterator  it_insert;
	int Best_Cost;
    logn5("Circuit::insert BEGIN " + station->name + " pos=" + U::to_s(pos));
    if (pos == -1) {
        it_insert = this->stations->insert(this->stations->end(), station);
        this->update();
        Best_Cost = this->get_cost();
        this->stations->erase(it_insert);
        this->update();
    } else {
        // on avance l'itérateur jusqu'à la position pos
        auto it = this->stations->begin();
        for (int i = 0; i < pos; ++i) {
            it++;
        }
        it_insert = this->stations->insert(it, station);
        this->update();
        Best_Cost = this->get_cost();
        this->stations->erase(it_insert);
        this->update();
    }
    logn5("Circuit::insert END");
    return Best_Cost;
}

// Cette brique d'insertion est opérationnelle (mais suppose que la brique
// d'équilibrage est faite), mais pas nécessairement efficace !
//
list<Station*>::iterator Circuit::insert_best(Station* station) {
	logn5("Circuit::insert_best insert_best BEGIN");

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




/*	if( this->desequilibre == 0 && !utilisation_heuristique){

		cout << *this;

		cout << "------------------------------------" << endl;
		cout << "station " << *station << " à ajouter deficit : " << station->deficit() << endl;
		cout << "------------------------------------" << endl;
		U::die("###############################################");
	}*/





	/****************************************************************/
	if(utilisation_heuristique == false){
		Log::level += 0; // on peut modifier le level juste pour cette méthode...
		logn5("Circuit::insert_best BEGIN " + this->remorque->name +
			  " insertion de " + station->name);
		// U::die("Circuit::insert_best : non implémentée");
		float best_cost = 999999999999;
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
	logn6("Circuit::insert_best circuit avant update finale");
    this->update();
    logn6("Circuit::insert_best circuit APRES insertion\n" + this->to_s_long());
    logn5("Circuit::insert_best END");
    Log::level -= 0; // ...on doit restaurer la modification du level

    return it_insert;


}




int Circuit::insert_bestCost(Station* station) {

	//int Best_cost_CPR = this->insert_bestCost(station);
	list<Station*>::iterator it_BestInsert;
	it_BestInsert = this->insert_best(station);

	this->update();
	int Best_Cost = this->desequilibre;
	/*if(Best_Cost != Best_Cost_CPR){
		cout << "######AVEC#####" << endl;
		cout << *this << endl;
	}*/
	this->stations->erase(it_BestInsert);
	this->update();
	/*if(Best_Cost != Best_Cost_CPR){
		cout << "#######SANS######" << endl;
		cout << *this << endl;
		U::die("");
	}*/
	return Best_Cost;


}




list<Station*>::iterator Circuit::my_insert(Station* s){
	int Best_iterateur;
	this->my_insertCost(s, Best_iterateur);
	return this->insert(s, Best_iterateur);

}

int Circuit::my_insertCost(Station* s, int& Best_iterateur){
	/* un circuit peut etre divisé en deux parties : une ou tout les desequilibres sont nuls (le début du circuit),
	 * et le complémentaire de cette partie. On introduit une variable boolenne FirstPart pour savoir dans quelle partie on se situe
	 */
	int Cost;
	int BestCost = 999999999;
	logn5("Circuit::my_insert BEGINING");
	bool FirstPart = true;
	int iterateur = 1;
	list<Station*>::iterator it2;




	it2 = this->insert(s,0);
	this->update();
	BestCost = this->desequilibre;
	Best_iterateur = 0;
	Cost = BestCost;
	this->stations->erase(it2);
	this->update();

	if(this->stations->size() > 0){
		for(auto it = this->stations->begin(); it != this->stations->end(); ++it){

			if(iterateur == this->stations->size())
				iterateur=-1;
			Station* station = *it;

			this->update();
			FirstPart = this->is_first_part(it);

			if(FirstPart){



				Cost = this->my_insertFirstPart(s, it);


				/*list<Station*>::iterator it2;

				it2 = this->insert(s,iterateur);

				this->update();
*/
				/*int realCost = this->get_cost();
				cout << "REAL" << endl;
				cout << *this << endl;
				cout << "REAL" << endl;
				this->stations->erase(it2);
				this->update();
				cout << "cou3" << endl;
				if((realCost - realCost%1000000) != 1000000*Cost){
					cout << *this << endl;

					cout << "insertStation : " << U::to_s(*station) << endl;
					cout << "Station" << U::to_s(*s) << endl;
					cout << "realCOst : " << (realCost - realCost%1000000) << "(" << realCost << ")" << endl;
					cout << "Cost : " << Cost << endl;
					U::die("Cost différents2");
				}*/


			}
			else{



				Cost = this->my_insertSecondPart(s, it);




			}
			if(Cost < BestCost){
				BestCost = Cost;
				Best_iterateur = iterateur;

			}
			iterateur++;





		}
	}




	return BestCost;//attention a savoir si quand  on insert, on insert avant ou apres  it2

}
int Circuit::my_insertFirstPart(Station* s, const list<Station*>::iterator& it2insert){
	logn2("Circuit::my_insertFirstPart insertion de : "+U::to_s(*s));
	bool SecondPartExiste, stationAddedAsFirstOf2Part;
	list<Station*>::iterator it_second_part;
	int deficit_from_partial_update;
	int desequilibre_before_SecondPart;
	int Charge_init = this->Partial_equilibrate(s, it2insert, it_second_part, SecondPartExiste, stationAddedAsFirstOf2Part,
			deficit_from_partial_update, desequilibre_before_SecondPart);

	if(SecondPartExiste){

		list<Station*>::iterator LastIterator = this->stations->end();
		LastIterator--;
		Station* LastStation =  *LastIterator;
		int charge_init_old = this->charge_init;
		int charge_init_new = Charge_init;

		auto desequilibre_circuit = this->desequilibre - abs(desequilibre_before_SecondPart);
		logn5("Circuit::my_insert LastStation charge_courante_min : "+U::to_s((*this->charges_courante_min)[LastStation]));
		logn5("Circuit::my_insert LastStation charge_courante_max : "+U::to_s((*this->charges_courante_max)[LastStation]));
		logn5("Circuit::my_insert LastStation charge_init_new : "+U::to_s(charge_init_new));
		logn5("Circuit::my_insert LastStation charge_init_old : "+U::to_s(charge_init_old));
		logn5("Circuit::my_insert LastStation desequilibre_circuit : "+U::to_s(desequilibre_circuit));
		logn5("Circuit::my_insert LastStation desequilibre_before_SecondPart : "+U::to_s(desequilibre_before_SecondPart));


		int deficit_a_absorber = deficit_from_partial_update;// - ((*this->charges_init_max)[LastStation] - (*this->charges_init_min)[LastStation]);
		deficit_a_absorber -= desequilibre_before_SecondPart;
		auto it_start = it_second_part;

		if(stationAddedAsFirstOf2Part){
			logn2("Circuit::my_insertFirstPart To SecondPart");
			//return this->my_insertSecondPart(s,it2insert);
		}


		Station* Station2Start = *it_start;

		logn5("Circuit::my_insert deficit a absorber : "+U::to_s(deficit_a_absorber));

		auto it2 = it_start;
		while(it2 != this->stations->end() || stationAddedAsFirstOf2Part){//on va mettre a jour les deficit engendré par les autres stations

			int charge_courante;
			int desequilibre_courant;

			Station* station = *it2;
			if(stationAddedAsFirstOf2Part){
				station = s;
				list<Station*>::iterator Prev_iterator = it2insert;
				bool premiere_position = false;
				if(!premiere_position){
					Station* Old_Station = *Prev_iterator;
					Station2Start = *Prev_iterator;
					desequilibre_courant = 0;
					charge_courante = (*this->charges_courante_min)[Old_Station] + charge_init_new - (*this->charges_init_min)[Old_Station];


				}
				else{
					if(s->deficit() > 0){
						charge_courante = 0;
						desequilibre_courant = s->deficit()-charge_init_new;
					}
					else{
						charge_courante = this->remorque->capa;
						desequilibre_courant = -(-s->deficit()- (this->remorque->capa-charge_init_new) );
					}
				}

			}
			else{//la derniere station ne devrais aps etre modifiée
				charge_courante = (*this->charges_courante_min)[station] + charge_init_new - (*this->charges_init_min)[station];// - s->deficit();

				desequilibre_courant = (*this->desequilibre_courant)[station];

			}

			logn5("Circuit::my_insert MAJ station : "+U::to_s(*station));
			logn5("Circuit::my_insert deficit a absorber : "+U::to_s(deficit_a_absorber));
			logn5("Circuit::my_insert desequilibre : "+U::to_s(desequilibre_circuit));
			logn5("Circuit::my_insert desequilibre_courant : "+U::to_s(desequilibre_courant));
			if(deficit_a_absorber == 0){
				if(desequilibre_courant == 0){
					if(charge_courante < 0){

						desequilibre_circuit += -charge_courante;
						desequilibre_courant += -charge_courante;
						charge_courante = 0;

						logn5("Circuit::my_insert deficit a absorber : "+U::to_s(deficit_a_absorber));
						logn5("Circuit::my_insert desequilibre : "+U::to_s(desequilibre_circuit));

					}

					if(charge_courante > this->remorque->capa){

						desequilibre_circuit += charge_courante - this->remorque->capa;
						desequilibre_courant -= charge_courante - this->remorque->capa;
						charge_courante = this->remorque->capa;



						logn5("Circuit::my_insert deficit a absorber : "+U::to_s(deficit_a_absorber));
						logn5("Circuit::my_insert desequilibre : "+U::to_s(desequilibre_circuit));
						logn5("Circuit::my_insert charge_courante : "+U::to_s(charge_courante));
					}
				}
				else if(desequilibre_courant > 0){// On veut recuperer des velos
					if(charge_courante < 0){
						desequilibre_circuit += -(charge_courante);
						desequilibre_courant += -charge_courante;
						charge_courante = 0;
					}
					else if(charge_courante - desequilibre_courant < 0){// Mais on en a pas assez au min
						desequilibre_circuit -= abs( -(charge_courante) );
						desequilibre_courant -= abs( charge_courante);
						charge_courante = 0;
					}
					else{// on a assez de velo, ou même plus qu'il n'en faut
						desequilibre_circuit -= abs(desequilibre_courant);
						desequilibre_courant = 0;
						charge_courante -= desequilibre_courant;
					}
					logn5("Circuit::my_insert deficit a absorber : "+U::to_s(deficit_a_absorber));
					logn5("Circuit::my_insert desequilibre : "+U::to_s(desequilibre_circuit));
					logn5("Circuit::my_insert charge_courante : "+U::to_s(charge_courante));
				}
				else if(desequilibre_courant < 0){// On veut poser des velos
					if(charge_courante > this->remorque->capa){
						desequilibre_circuit += abs(charge_courante - this->remorque->capa);
						desequilibre_courant += abs(charge_courante - this->remorque->capa);
						charge_courante = this->remorque->capa;

					}
					else if(charge_courante - desequilibre_courant > this->remorque->capa){

						desequilibre_circuit -= abs(charge_courante - this->remorque->capa);
						desequilibre_courant -= -abs(charge_courante - this->remorque->capa);
						charge_courante = this->remorque->capa;


					}
					else if(charge_courante - desequilibre_courant < this->remorque->capa){
						desequilibre_circuit -= abs(desequilibre_courant);
						charge_courante += -desequilibre_courant;
						desequilibre_courant = 0;
					}
					logn5("Circuit::my_insertb deficit a absorber : "+U::to_s(deficit_a_absorber));
					logn5("Circuit::my_insert desequilibre : "+U::to_s(desequilibre_circuit));
					logn5("Circuit::my_insert charge_courante : "+U::to_s(charge_courante));
				}
			}
			if(deficit_a_absorber < 0){
				if(desequilibre_courant > 0){
					if(charge_courante + deficit_a_absorber >= desequilibre_courant){
						desequilibre_circuit += -desequilibre_courant;
						deficit_a_absorber -= -(desequilibre_courant);
						desequilibre_courant = 0;
						if(deficit_a_absorber < 0)
							U::die("Circuit::my_insertFirst insert4");//normalement ça n'arrive pas

						logn2("Circuit::my_insert4 desequilibre : "+U::to_s(desequilibre_circuit));
					}
				}
				else if(desequilibre_courant == 0){
					if(charge_courante < -deficit_a_absorber){
						desequilibre_circuit += -deficit_a_absorber - charge_courante;
						desequilibre_courant += -deficit_a_absorber - charge_courante;
						deficit_a_absorber -= -(-deficit_a_absorber - charge_courante);//deficit a absorber negatif d'où le moins

						logn2("Circuit::my_insert5 desequilibre : "+U::to_s(desequilibre_circuit));
					}/*pas besoin de mettre a jour la charge_courante_min car on le fait apres*/

				}
				else{
					if(-desequilibre_courant > -deficit_a_absorber){
						desequilibre_circuit -= -deficit_a_absorber;
						desequilibre_courant -= -deficit_a_absorber;
						deficit_a_absorber = 0;
						logn2("Circuit::my_insert6 desequilibre : "+U::to_s(desequilibre_circuit));
					}
					else{
						desequilibre_circuit -= -desequilibre_courant;
						deficit_a_absorber -= desequilibre_courant;
						desequilibre_courant = 0;
						logn2("Circuit::my_insert7 desequilibre : "+U::to_s(desequilibre_circuit));
					}
				}

			}
			if(deficit_a_absorber > 0){

				if(desequilibre_courant < 0){

					if(this->remorque->capa - charge_courante < deficit_a_absorber){
						desequilibre_circuit += deficit_a_absorber-(this->remorque->capa - charge_courante);//attention charge max
						desequilibre_courant += -(deficit_a_absorber-(this->remorque->capa - charge_courante));
						deficit_a_absorber -= deficit_a_absorber-(this->remorque->capa - charge_courante);




					}
					logn2("Circuit::my_insert8 desequilibre : "+U::to_s(desequilibre_circuit));
					logn2("Circuit::my_insert8 deficit_a_absorber : "+U::to_s(deficit_a_absorber));
					logn2("Circuit::my_insert8 charge_courante : "+U::to_s(charge_courante));
					logn2("Circuit::my_insert8 desequilibre_courant : "+U::to_s(desequilibre_courant));
				}
				else if(desequilibre_courant == 0){
					if(this->remorque->capa - charge_courante < deficit_a_absorber){

						desequilibre_circuit += deficit_a_absorber - (this->remorque->capa - charge_courante);
						desequilibre_courant -= deficit_a_absorber - (this->remorque->capa - charge_courante);
						deficit_a_absorber -= deficit_a_absorber - (this->remorque->capa - charge_courante);
						logn2("Circuit::my_insert9 desequilibre : "+U::to_s(desequilibre_circuit));
					}/*pas besoin de mettre a jour la charge_courante_min car en s'en resert pas*/
				}
				else{
					if(desequilibre_courant  > deficit_a_absorber){
						desequilibre_circuit -= deficit_a_absorber;
						desequilibre_courant -= deficit_a_absorber;
						deficit_a_absorber = 0;

						logn2("Circuit::my_insert10 desequilibre : "+U::to_s(desequilibre_circuit));
					}
					else{
						if(charge_courante + deficit_a_absorber - desequilibre_courant >= 0){
							desequilibre_circuit -= (desequilibre_courant);
							deficit_a_absorber -= desequilibre_courant;
							desequilibre_courant = 0;

							logn2("Circuit::my_insert11 desequilibre : "+U::to_s(desequilibre_circuit));
						}
						else{
							desequilibre_circuit -= desequilibre_courant + (charge_courante + deficit_a_absorber - desequilibre_courant);
							auto tmp = desequilibre_courant + (charge_courante + deficit_a_absorber - desequilibre_courant);
							deficit_a_absorber -= desequilibre_courant + (charge_courante + deficit_a_absorber - desequilibre_courant);
							desequilibre_courant -= tmp;
							logn2("Circuit::my_insert12 desequilibre : "+U::to_s(desequilibre_circuit));
							logn2("Circuit::my_insert12 deficit_a_absorber : "+U::to_s(deficit_a_absorber));
							logn2("Circuit::my_insert12 charge_courante : "+U::to_s(charge_courante));
							logn2("Circuit::my_insert12 desequilibre_courant : "+U::to_s(desequilibre_courant));
						}
					}
				}


			}

			charge_courante += deficit_a_absorber;

			if(charge_courante > this->remorque->capa){// dans ce cas min==max
				logn5("Circuit::my_insert Corection charge_courante : "+U::to_s(charge_courante));
				desequilibre_courant += -(charge_courante - this->remorque->capa);
				deficit_a_absorber -= (charge_courante - this->remorque->capa);
				desequilibre_circuit += abs( -(charge_courante - this->remorque->capa) );
				charge_courante = this->remorque->capa;
			}
			if(charge_courante < 0){//je crois que dans ce cas min==max
				logn5("Circuit::my_insert Corection charge_courante : "+U::to_s(charge_courante));
				desequilibre_courant += -(charge_courante);
				desequilibre_circuit += -(charge_courante);
				deficit_a_absorber -= charge_courante;//deficit_a_absorber <0
				charge_courante = 0;
			}

			if(charge_courante == (*this->charges_courante_min)[station] &&
					desequilibre_courant == (*this->desequilibre_courant)[station] &&
					deficit_a_absorber == 0 &&
					!stationAddedAsFirstOf2Part){// si on a consomé tout le desequilibre et que l'état de nos station est le même que celui avant ajout, l'ajout ne modifiera plus rien.
				break;//alors on quitte la boucle
			}



			if (stationAddedAsFirstOf2Part){
				stationAddedAsFirstOf2Part = false;
			}
			else{
				++it2;
			}
			//desequilibre_circuit += deficit_a_absorber;
			logn5("Circuit::my_insert charge_courante : "+U::to_s(charge_courante));
			//if(deficit_a_absorber == 0)
				//break;
		}




		auto cost = compute_cost(s, desequilibre_circuit, it2insert);
		return cost;


	}
	else{
		return compute_cost(s, 0, it2insert);
	}

	/*this->stations->erase(it2);
	this->update();*/


}
int Circuit::my_insertSecondPart(Station* s, const list<Station*>::iterator& it2insert){

	log5("Circuit::my_insertSecondPart BEGIN");
	/* dans ce cas, la charge_init est fixée (on ne la modifiera pas ajoutant la station).
	 * il faut calculer le deficit engendré par la station avec la charge courante juste avant l'insertion,
	 * et repercuter l'effet de la nouvelle charge courante après la station
	 */
	auto desequilibre_circuit = this->desequilibre;//on recupère le desequilibre du circuit
	logn5("Circuit::my_insertSecondPart desequilibre0 : "+U::to_s(desequilibre_circuit));

	auto charge_courante_old = (*this->charges_courante_min)[*it2insert];//min==max car la charge_init est fixée

	auto charge_courante_new = charge_courante_old;

	//mise à jour de la charge courante pour la stations insérée et des deficits associé.

	if(s->deficit()>=0){//il manque des velos dans s
		if(s->deficit() > charge_courante_old){//l'ancienne charge n'est pas suffisante
			charge_courante_new = 0;
			desequilibre_circuit += s->deficit() - charge_courante_old;//le surplus passe en déséquilibre
			logn2("Circuit::my_insertSecondPart desequilibre1 : "+U::to_s(desequilibre_circuit));
		}
		else
			charge_courante_new = charge_courante_old - s->deficit();//la charge_old peut absorber le deficit
	}
	else{//il y a trop de vélo en s
		if(-s->deficit() > this->remorque->capa - charge_courante_old){//il n'y a pas assez de place dans la remorque
			charge_courante_new = this->remorque->capa;
			desequilibre_circuit += -s->deficit() - (this->remorque->capa - charge_courante_old);
			logn2("Circuit::my_insertSecondPart desequilibre2 : "+U::to_s(desequilibre_circuit));
		}
		else
			charge_courante_new = charge_courante_old + (-s->deficit());//le charge old permet d'absorber le deficit
	}

	// il faut ajouter les deficits correspondant au stations suivant celle inséré
	/* on va mettre à le desequilibre. supposons le deficit de la station positif, si à un moment, la remroque est vide
	 * ce deficit se transforme en desequilibre. l'idée est de regarder où le déficit apporté se transforme en déséquilibre, et les
	 * conséquences qui en découlent.
	 */
	auto it_start = it2insert;
	++it_start;//on a déja traité la station it2insert

	auto deficit_a_absorber = charge_courante_new - charge_courante_old;

	for(auto it2 = it_start; it2 != this->stations->end(); ++it2){//on va mettre a jour les deficit engendré par les autres stations

		Station* station = *it2;
		logn5("Circuit::my_insert MAJ station : "+U::to_s(*station));
		logn5("Circuit::my_insert deficit a absorber : "+U::to_s(deficit_a_absorber));
		logn5("Circuit::my_insert desequilibre : "+U::to_s(desequilibre_circuit));
		logn5("Circuit::my_insert charge_courante_min : "+U::to_s((*this->charges_courante_min)[station]));
		logn5("Circuit::my_insert charge_courante_max : "+U::to_s((*this->charges_courante_max)[station]));
		int charge_courante_min = (*this->charges_courante_min)[station];
		int charge_courante_max = (*this->charges_courante_max)[station];
		int desequilibre_courant = (*this->desequilibre_courant)[station];


		if(deficit_a_absorber < 0){
			if(desequilibre_courant > 0){
				desequilibre_circuit += -deficit_a_absorber;
				deficit_a_absorber = 0;
				logn2("Circuit::my_insert4 desequilibre : "+U::to_s(desequilibre_circuit));
			}
			else if(desequilibre_courant == 0){
				if(charge_courante_min < -deficit_a_absorber){
					desequilibre_circuit += -deficit_a_absorber - charge_courante_min;
					deficit_a_absorber -= -(-deficit_a_absorber - charge_courante_min);//deficit a absorber negatif d'où le moins
					logn2("Circuit::my_insert5 desequilibre : "+U::to_s(desequilibre_circuit));
				}/*pas besoin de mettre a jour la charge_courante_min car en s'en resert pas*/
			}
			else{
				if(-desequilibre_courant > -deficit_a_absorber){
					desequilibre_circuit -= -deficit_a_absorber;
					deficit_a_absorber = 0;
					logn2("Circuit::my_insert6 desequilibre : "+U::to_s(desequilibre_circuit));
				}
				else{
					desequilibre_circuit -= -desequilibre_courant;
					deficit_a_absorber -= desequilibre_courant;
					logn2("Circuit::my_insert7 desequilibre : "+U::to_s(desequilibre_circuit));
				}
			}

		}
		if(deficit_a_absorber > 0){

			if(desequilibre_courant < 0){
				desequilibre_circuit += deficit_a_absorber;
				deficit_a_absorber = 0;
				logn2("Circuit::my_insert8 desequilibre : "+U::to_s(desequilibre_circuit));
			}
			else if(desequilibre_courant == 0){
				if(this->remorque->capa - charge_courante_min < deficit_a_absorber){
					desequilibre_circuit += deficit_a_absorber - (this->remorque->capa - charge_courante_min);
					deficit_a_absorber -= deficit_a_absorber - (this->remorque->capa - charge_courante_min);
					logn2("Circuit::my_insert9 desequilibre : "+U::to_s(desequilibre_circuit));
				}/*pas besoin de mettre a jour la charge_courante_min car en s'en resert pas*/
			}
			else{
				if(desequilibre_courant > deficit_a_absorber){
					desequilibre_circuit -= deficit_a_absorber;
					deficit_a_absorber = 0;
					logn2("Circuit::my_insert10 desequilibre : "+U::to_s(desequilibre_circuit));
				}
				else{
					desequilibre_circuit -= desequilibre_courant;
					deficit_a_absorber -= desequilibre_courant;
					logn2("Circuit::my_insert11 desequilibre : "+U::to_s(desequilibre_circuit));
				}
			}


		}
		//desequilibre_circuit += deficit_a_absorber;

		if(deficit_a_absorber == 0)
				break;
	}




	auto cost = compute_cost(s, desequilibre_circuit, it2insert);
	return cost;
}
int Circuit::compute_cost(Station* s, int desequilibre, const list<Station*>::iterator& it2insert){
	list<Station*>::iterator it = it2insert;
	Site* s0 = *it2insert;
	Site* s1 = s;
	Site* Remorque = this->remorque;
	if(it != this->stations->end()){
		++it;
		if(it != this->stations->end()){
			Site* s2 = *(it);
			//U::die("dist : ("+U::to_s(*s0) + "," +U::to_s(*s2) + "), "+U::to_s(inst->get_dist(s0,s2)));
			//return 1000000*desequilibre + this->length + inst->get_dist(s0,s1) + inst->get_dist(s1,s2) - inst->get_dist(s0,s2);
			return desequilibre;
		}
	}

	return desequilibre;
	//return 1000000*desequilibre + this->length + inst->get_dist(s0,s1) - inst->get_dist(s0,Remorque) + inst->get_dist(s1,Remorque);



}

bool Circuit::is_first_part(const list<Station*>::iterator& it){
	Station* station = *it;
	if((*this->charges_init_min)[station] == (*this->charges_init_max)[station])
		return false;
	else if ((*this->charges_init_min)[station] < (*this->charges_init_max)[station]){
		return true;
	}
	else
		U::die("DIE!!! Circuit::first_part this->charges_init_min > this->charges_init_max");
	U::die("Circuit::is_first_part DIE");
	return true;
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
        int desequilibre_courant = this->desequilibre_courant->at((Station*)arc->dst);
        buf << "   " <<  arc->to_s_long()
            << " depot de "  << depot
            << " => charge = " << charge
            << " , déséquilibre courant : " << desequilibre_courant << endl;
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
	flux << "desequilibre : " << circuit.desequilibre << endl;
	flux << "taille de la remorque : " << circuit.remorque->capa << endl;
	for (auto it = circuit.stations->begin(); it != circuit.stations->end(); ++it) {
		Station* current_station = *it;
		flux << "------------------------------------" << endl;
		flux << U::to_s(*current_station) << endl;
		flux << "Station charge : " << (*circuit.charges)[current_station] << endl;
		flux << "Station depot : " << (*circuit.depots)[current_station] << endl;

		flux << "Station charge_courante_min : " << (*circuit.charges_courante_min)[current_station] << endl;
		flux << "Station charge_courante_max : " << (*circuit.charges_courante_max)[current_station] << endl;
		flux << "Station charge_init_min : " << (*circuit.charges_init_min)[current_station] << endl;
		flux << "Station charge_init_max : " << (*circuit.charges_init_max)[current_station] << endl;
		flux << "Station desequilibre_courant : " << (*circuit.desequilibre_courant)[current_station] << endl;


	}
	flux << "------------------------------------" << endl;
	flux <<"###############################################" << endl;
    //Affichage des attributs

    return flux;

}
//./

