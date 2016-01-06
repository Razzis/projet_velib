#include "solver.hpp"
#include "voisinage.hpp"
#include <map>
#include <vector>

#define OPTIM_JUST_DESEQUILIBRE
//#define VERIF_CIRCUIT_NON_VIDE//fait on une recherche pour savoir si les circuits sont vides

GreedySolver::GreedySolver(Instance* inst) : Solver::Solver(inst) {
	this->solution = new Solution(inst);
    name = "GreedySolver";
    desc = "Solver par méthode glouton (intelligent)";
    cerr << "\nGreedySolver non implémenté : AU BOULOT !" << endl;
    logn1(name + ": " + desc + " inst: " + inst->name);


}
GreedySolver::~GreedySolver()  {
    // TODO
	delete this->solution;
}
// Méthode principale de ce solver, principe :
//
bool GreedySolver::solve() {
/* TODO : il n'est pas nécéssaire d'iserer lles station dans les circuit, juste de récupérer
 * les coûts engendré */
    found = false;



    Solution tmp_sol = Solution(inst);

    cout << "debut du remplissage des remorques" << endl;



    /********************************************/

    /* minimisation du déséquilibre et des distances */

    Options* args = Options::args;
        string sinserter = args->station_inserter;
        const string schooser = args->station_chooser;
        const string rchooser = args->remorque_chooser;

    vector<bool> circuit_non_vide(inst->remorques->size(),false);//va permettre de s'assurer qu'il  y a au moins une station par circuit
    bool all_circuit_non_vide = false;

    // parcours des stations de l'instance
    int Best_Cost;
    int Best_iterateur;
    vector<Station*>* station_list;

    vector<Station*>* station_list_pos = new vector<Station*>();//contient eventuellement les stations de deficit >0
    vector<Station*>* station_list_neg = new vector<Station*>();//contient eventuellement les stations de deficit <0

    if(sinserter == "DOUBLE" || sinserter == "DOUBLE_MYINSERT"){//si sinserter == double, schooser n'est plus important, on doit deja trier les stations d'une certaine façon
		/* on trie les stations par deficit croissant (donc de -beaucoup à +beaucoup et on les met dans deux vecteurs :
		 * l'un contenant les stations à déficit positif (allant de 0 à ++++) et un autre avec les station à déficit negatif (de ---- à 0 exclu)
		 */
		station_list = inst->stations;
		std::sort (station_list->begin(), station_list->end(), compare_station);

		*station_list_pos = vector<Station*>(*(inst->stations));
		*station_list_neg = vector<Station*>(*(inst->stations));

		filtrate_list(station_list_pos, station_list_neg);
		std::sort (station_list_pos->begin(), station_list_pos->end(), compare_station);

		std::sort (station_list_neg->begin(), station_list_neg->end(), compare_station);
		//std::reverse(station_list_neg->begin(), station_list_neg->end());

		for(auto it = station_list_pos->begin(); it != station_list_pos->end(); ++it){
			Station* station = *it;
			cout << "station_pos " << U::to_s(*station) << " of deficit : " << station->deficit() << endl;
		}
		for(auto it = station_list_neg->begin(); it != station_list_neg->end(); ++it){
			Station* station = *it;
			cout << "station_neg " << U::to_s(*station) << " of deficit : " << station->deficit() << endl;
		}
		cout << "pos size : " << station_list_pos->size() << endl;
		cout << "neg size : " << station_list_neg->size() << endl;
        	/*U::die("test");*/
    }else if(schooser == "SORTED"){
    	station_list = inst->stations;
    	std::sort (station_list->begin(), station_list->end(), compare_station);
    }
    else if(schooser == "RSORTED"){
        station_list = inst->stations;
        std::sort (station_list->begin(), station_list->end(), compare_station);
        std::reverse(station_list->begin(), station_list->end());
    }
    else{
    	station_list = inst->stations;
    }
    int iterateur = 0;
    int iterateur_neg = 0;
    int iterateur_pos = 0;


    Station* station;
    int Cost_insert_neg;
    int Cost_insert_pos;
    bool pos_deficit_choosed;

    vector<int> remorque_indexes;
    for(auto it = 0; it != tmp_sol.circuits->size(); it++)
    	remorque_indexes.push_back(it);
    for(auto it = station_list->begin(); it != station_list->end(); ++it){
    	if (sinserter != "DOUBLE" && sinserter != "DOUBLE_MYINSERT")
    		station = *it;
    	time_t t0;
    	time(&t0);


    	auto best_id = -1;
    	Best_Cost = 999999999;
    	Best_iterateur = -2;



    	logn1("GreedySolver::solve ajout de la station : "+U::to_s(iterateur));
    	iterateur++;

    	vector<Circuit*>* circuit_list;
    	vector<Circuit*>* sol_circuit_list;

        if(rchooser == "SORTED"){
        	 circuit_list = tmp_sol.circuits;
        	std::sort (circuit_list->begin(), circuit_list->end(), compare_circuit);

        	sol_circuit_list = this->solution->circuits;
        	std::sort (sol_circuit_list->begin(), sol_circuit_list->end(), compare_circuit);
        }
        else if(rchooser == "RSORTED"){
            circuit_list = tmp_sol.circuits;
            std::sort (circuit_list->begin(), circuit_list->end(), compare_circuit);
            std::reverse(circuit_list->begin(), circuit_list->end());

            sol_circuit_list = this->solution->circuits;
            std::sort (sol_circuit_list->begin(), sol_circuit_list->end(), compare_circuit);
            std::reverse(sol_circuit_list->begin(), sol_circuit_list->end());
        }
        else{
        	circuit_list = tmp_sol.circuits;
        	sol_circuit_list = this->solution->circuits;
        }


    	/** Pour chaque station, on determine le circuit le mieu adapté (qui minimise les desequilibre et eventullement la distance)
    	 * et on l'ajoute au circuit concerné
    	 */
    	// On parcours les différentes remorques à laquelle on pourrait atribuer la station
        int remorque_id;
    	for(auto remorque_index = 0; remorque_index < tmp_sol.circuits->size(); remorque_index++)
    	{
    		remorque_id = remorque_indexes[remorque_index];//si on ne veut aps choisir dans l'ordre habituel


    		logn7("greedy::solve Before attribution Circuit");
    		Circuit* circuit = (*circuit_list)[remorque_id];
    		// On ajoute la station dans un circuit temporaire
    		int insert_iterateur = -2;
    		int insert_iterateur_neg;
    		int insert_iterateur_pos;
    		int Cost_insert;
    		list<Station*>::iterator it_insert;
    		if (sinserter == "FRONT") {
    			Cost_insert = circuit->insertCost(station);
    		} else if (sinserter == "DOUBLE" || sinserter == "DOUBLE_MYINSERT") {
    			Cost_insert_neg = 999999999;
    			Cost_insert_pos = 999999999;

				/*while(station_list_pos->at(iterateur_pos)->deficit() > circuit->remorque->capa && remorque_index != tmp_sol.circuits->size()-1){
					remorque_index++;
					remorque_id = remorque_indexes[remorque_index];
					circuit = (*circuit_list)[remorque_id];
				}*/
    			/* pour chacun des deux vecteurs, si il lui reste des station, on calcul le cout engendré */
				if(iterateur_pos < station_list_pos->size()){

					if (sinserter == "DOUBLE"){
						Cost_insert_pos = circuit->insertCost(station_list_pos->at(iterateur_pos), -1);
						//if(abs(station_list_pos->at(iterateur_pos)->deficit()) > circuit->remorque->capa)
						//		Cost_insert_pos += 1000000*abs(station_list_pos->at(iterateur_pos)->deficit()) - circuit->remorque->capa;
						//Cost_insert_pos = (Cost_insert_pos-Cost_insert_pos%1000000)/1000000;
					}
					else{
						Cost_insert_pos = circuit->my_insertTotalCost(station_list_pos->at(iterateur_pos), insert_iterateur_pos);
					}



				}
    			if(iterateur_neg < station_list_neg->size()){
    				if (sinserter == "DOUBLE"){
    					Cost_insert_neg = circuit->insertCost(station_list_neg->at(iterateur_neg), -1);
						//if(abs(station_list_neg->at(iterateur_neg)->deficit()) > circuit->remorque->capa)
						//		Cost_insert_neg += 1000000*abs(station_list_neg->at(iterateur_neg)->deficit()) - circuit->remorque->capa;
    					//Station* Last_Station = *(circuit->stations->end());
    					//cout << "circuit->charges_courante_min->end() : " << (*circuit->charges_courante_min)[Last_Station] << endl;
    					//cout << " station_list_neg->at(iterateur_neg)->deficit() : " << station_list_neg->at(iterateur_neg)->deficit() << endl;
    					//U::die("TEST");
    					//if(iterateur > circuit->stations->size()/2 && Cost_insert_neg < 1000000 && (*circuit->charges_courante_min)[Last_Station] - station_list_neg->at(iterateur_neg)->deficit() < circuit->remorque->capa)
    						//Cost_insert_neg = Cost_insert_neg/(((*circuit->charges_courante_min)[Last_Station] - station_list_neg->at(iterateur_neg)->deficit())*
    					//			((*circuit->charges_courante_min)[Last_Station] - station_list_neg->at(iterateur_neg)->deficit()));
    					//Cost_insert_neg = (Cost_insert_neg-Cost_insert_neg%1000000)/1000000;
    				}
    				else{
    					Cost_insert_neg = circuit->my_insertTotalCost(station_list_neg->at(iterateur_neg), insert_iterateur_neg);
    				}

					/*while(station_list_pos->at(iterateur_pos)->deficit() > circuit->remorque->capa && remorque_index != tmp_sol.circuits->size()-1){
						Cost_insert_neg += 10000000;
						remorque_index++;
						remorque_id = remorque_indexes[remorque_index];
						circuit = (*circuit_list)[remorque_id];
						Cost_insert_pos = circuit->insertCost(station_list_pos->at(iterateur_pos), -1);
					}*/


    			}

    			//cout << "iterateur pos : " << iterateur_pos << endl;
				//cout << "iterateur neg : " << iterateur_neg << endl;
				//cout <<"#neg "<< U::to_s(*(station_list_pos->at(iterateur_pos))) << endl;
				//cout << "Cost_insert_pos : " << Cost_insert_pos << endl;
				//cout << "Cost_insert_neg : " << Cost_insert_neg << endl;

    			//On choisis le meilleurs des deux couts
    			if((Cost_insert_pos > Cost_insert_neg || iterateur_pos == station_list_pos->size())&& iterateur_neg < station_list_neg->size()){
    				Cost_insert = Cost_insert_neg;
    				insert_iterateur = insert_iterateur_neg;
    				station = (station_list_neg->at(iterateur_neg));
    			}
    			else if(Cost_insert_pos <= Cost_insert_neg && iterateur_pos < station_list_pos->size()){

    				Cost_insert = Cost_insert_pos;
    				insert_iterateur = insert_iterateur_pos;
    				//cout <<"#neg "<< U::to_s(*(station_list_neg->at(iterateur_neg))) << endl;
    				station = (station_list_pos->at(iterateur_pos));
    				//cout <<"#neg "<< U::to_s(*(station_list_neg->at(iterateur_neg))) << endl;

    			}
    			else{
    				cout << "iterateur pos : " << iterateur_pos << endl;
    				cout << "iterateur neg : " << iterateur_neg << endl;
    				cout << "station_list_pos->size(): " << station_list_pos->size() << endl;
    				cout << "station_list_neg->size() : " << station_list_neg->size() << endl;
    				cout << "Cost_insert_pos : " << Cost_insert_pos << endl;
    				cout << "Cost_insert_neg : " << Cost_insert_neg << endl;
    				U::die("GReedy::double erreur");
    			}

    			//Cost_insert = (Cost_insert-Cost_insert%1000000)/1000000;
    		} else if (sinserter == "BACK") {
    			Cost_insert = circuit->insertCost(station, -1);
    			Cost_insert = (Cost_insert-Cost_insert%1000000)/1000000;
    		} else if (sinserter == "BEST") {
    			//Cost_insert = circuit->insert_bestCost(station);

    			it_insert = circuit->insert_best(station);
    			Cost_insert = circuit->desequilibre;
    			circuit->update();
    			circuit->stations->erase(it_insert);
    			circuit->update();
    		} else if (sinserter == "MYINSERT") {
    			Cost_insert = circuit->my_insertCost(station, insert_iterateur);
			} else if (sinserter == "HEURISTIQUE") {
				if(station->deficit() > 0){
					if(circuit->iterateur2desequilibreMin-1 <= circuit->stations->size())
						Cost_insert = circuit->insertCost(station, -1);
					else
						Cost_insert = circuit->insertCost(station, circuit->iterateur2desequilibreMin+1);
				}
				else{
					if(circuit->iterateur2desequilibreMax-1 <= circuit->stations->size())
						Cost_insert = circuit->insertCost(station, -1);
					else
						Cost_insert = circuit->insertCost(station, circuit->iterateur2desequilibreMax+1);
				}
			}
    		else {
    			U::die("station_inserter inconnu : " + U::to_s(sinserter));
    		}
    		logn7("greedy::solve attribution de la station " + U::to_s(*station) + " à la remorque " + U::to_s(*(circuit->remorque)));
    		//cout << "inserer " << U::to_s(*station) << " dans " << U::to_s(*(circuit->remorque)) << " coute : " << Cost_insert << endl;

    		logn5("greedy::solve insertion de la station dans le circuit : done");

    		logn5("greedy::solve circuit mis à jour");








			#ifndef VERIF_CIRCUIT_NON_VIDE


			if(Cost_insert < Best_Cost){
				// On cherche à determiner la meilleur remorque à laquelle ajouter la station
				best_id = remorque_id;
				if (sinserter == "DOUBLE" || sinserter == "DOUBLE_MYINSERT") {//on a beosin de retenir si on a utilisé une station positive ou negative de la remorque best_id
					if(Cost_insert == Cost_insert_pos)
						pos_deficit_choosed = true;
					else
						pos_deficit_choosed = false;
				}
				Best_Cost = Cost_insert;
				if (sinserter == "MYINSERT" || sinserter == "DOUBLE_MYINSERT"  ) {
					Best_iterateur = insert_iterateur;
				}


				// Si le desequilibre est nul, on peut choisir cette remorque (on ne trouvera pas mieu)
				if(Best_Cost == 0)
					break;

			}
			#endif



			//retour à la solution temporaire (on la remet à la dernière "meilleur solution")
			//auto stations = *(circuit->stations);
			//stations.erase(it);
			//*tmp_sol = *(this->solution);
    	}

    	if(best_id == -1)
    		U::die("best id n'a pas bougé");

		#ifdef VERIF_CIRCUIT_NON_VIDE
    	if(circuit_non_vide.at(best_id) == false)
    		circuit_non_vide.at(best_id) = true;

    	for(auto it = circuit_non_vide.begin(); it!=circuit_non_vide.end(); ++it){
    		if(*it != true){
    			all_circuit_non_vide = false;
    			break;
    		}
    		all_circuit_non_vide = true;
    	}
		#endif
    	if(iterateur >= station_list->size()-(tmp_sol.circuits->size()+1)){//si à la fin certaine remorque n'ont pas été rempli, on rempli avec les dernière station
    		for(auto id = 0; id < tmp_sol.circuits->size(); id++){
    			if((*sol_circuit_list)[id]->stations->size() == 0){
    				best_id = id;
    				break;
    			}
    		}
    	}
    	if (rchooser == "RAND")//on melange les remorques
    		random_shuffle(remorque_indexes.begin(),remorque_indexes.end());
		Circuit* best_circuit = (*sol_circuit_list)[best_id];
		Circuit* tmp_circuit = (*circuit_list)[best_id];



		if(sinserter == "DOUBLE" || sinserter == "DOUBLE_MYINSERT") {
			if(pos_deficit_choosed)//on recupère la station concerné
				station = (station_list_pos->at(iterateur_pos));
			else
				station = (station_list_neg->at(iterateur_neg));
		}

		// On ajoute la station au circuit choisis
		if (sinserter == "FRONT") {
			best_circuit->insert(station);
			tmp_circuit->insert(station);
		} else if (sinserter == "BACK") {
			best_circuit->insert(station, -1);
			tmp_circuit->insert(station, -1);
		}
		else if (sinserter == "DOUBLE") {
			best_circuit->insert(station, -1);
			tmp_circuit->insert(station, -1);
		}else if (sinserter == "BEST") {
			best_circuit->insert_best(station);
			tmp_circuit->insert_best(station);
		} else if (sinserter == "MYINSERT" || sinserter == "DOUBLE_MYINSERT") {
			best_circuit->insert(station, Best_iterateur);
			tmp_circuit->insert(station, Best_iterateur);
		} else if (sinserter == "HEURISTIQUE") {
			if(station->deficit() > 0){
				if(tmp_circuit->iterateur2desequilibreMin-1 <= tmp_circuit->stations->size()){
					tmp_circuit->insert(station, -1);
					best_circuit->insert(station, -1);
				}
				else{
					tmp_circuit->insert(station, tmp_circuit->iterateur2desequilibreMin+1);
					best_circuit->insert(station, best_circuit->iterateur2desequilibreMin+1);
				}
			}
			else{
				if(tmp_circuit->iterateur2desequilibreMax-1 <= tmp_circuit->stations->size()){
					tmp_circuit->insert(station, -1);
					best_circuit->insert(station, -1);
				}
				else{
					tmp_circuit->insert(station, tmp_circuit->iterateur2desequilibreMax+1);
					best_circuit->insert(station, best_circuit->iterateur2desequilibreMax+1);
				}
			}
	}else {
			U::die("station_inserter inconnu : " + U::to_s(sinserter));
		}

		if (station->deficit()>=0)
			iterateur_pos++;
		else
			iterateur_neg++;


		best_circuit->update();

		//cout << *best_circuit << endl;
		tmp_circuit->update();
		//cout << *best_circuit << endl;
		//cout << *this->solution->circuits->at(1);

		time_t t1;
		time(&t1);
    }

    logn5("Circuit::greedy fin solve, before update");
    this->solution->update();
    while(this->Corrige_Greedy())
    	cout << "coorige" << endl;
    while(this->Corrige_Greedy_seconde_passe())
    	cout << "coorige2" << endl;


    this->solution->update();
    this->found = true;
    delete station_list_pos;
    delete station_list_neg;
    return found;
}

bool GreedySolver::Corrige_Greedy(){
	int old_desequilibre = this->solution->desequilibre;
    bool heuristique_possible = false;

    if(this->solution->desequilibre > 0){//on ne retraite que si le dessequilibre est non nul
    	for(auto it = this->solution->circuits->begin(); it != this->solution->circuits->end(); ++it){//on cherche un circuit de desequilibre non nul
    		Circuit* current_circuit = *it;
    		Station* Last_station = *(--current_circuit->stations->end());
    		/*for(auto it3 = current_circuit->stations->begin(); it3 != current_circuit->stations->end(); ++it3){
    			Last_station = *it3;//Dernière station du circuit (On espère que c'est elle qui engendre le déficit)
    			if((*current_circuit->desequilibre_courant)[Last_station] != 0)
    				break;
    		}*/
    		//cout << "Last_station : " << U::to_s(*Last_station) << " def : " << (*current_circuit->desequilibre_courant)[Last_station] << endl;
    		if(current_circuit->desequilibre > 0 && (*current_circuit->desequilibre_courant)[Last_station] > 0){//pour l'instant on ne traite que les deficit > 0
    			heuristique_possible = true;
    			if(abs(Last_station->deficit()) > current_circuit->remorque->capa){//il faut au moins que la remorque ait la taille pour absorber le deficit
    				for(auto it2 = this->solution->circuits->begin(); it2 != this->solution->circuits->end(); ++it2){// on cherche une remorque de taille suffisate
    					Circuit* circuit = *it2;
    					if(circuit->remorque->capa >= abs(Last_station->deficit())){
							move(current_circuit, circuit,
									current_circuit->stations->size()-1, -1);

							current_circuit->update();

    					}
    				}
    			}
    		}
    		else if(current_circuit->desequilibre > 0 && (*current_circuit->desequilibre_courant)[Last_station] < 0){//pour l'instant on ne traite que les deficit > 0
    			heuristique_possible = true;
    			if(abs(Last_station->deficit()) > current_circuit->remorque->capa){//il faut au moins que la remorque ait la taille pour absorber le deficit
    				for(auto it2 = this->solution->circuits->begin(); it2 != this->solution->circuits->end(); ++it2){// on cherche une remorque de taille suffisate
    					Circuit* circuit = *it2;
    					if(circuit->remorque->capa >= abs(Last_station->deficit())){
							move(current_circuit, circuit,
									current_circuit->stations->size()-1, -1);

							current_circuit->update();

    					}
    				}
    			}
    		}
    	}
    }

    this->solution->update();

    if(heuristique_possible){
    	heuristique_possible = false;
    	for(auto it = this->solution->circuits->begin(); it != this->solution->circuits->end(); ++it){//on parcours les remorques pour chercher celle dont la dernière stations est desequilibrée
    		Circuit* current_circuit = *it;
    		Station* Last_station = *(--current_circuit->stations->end());
    		Circuit* current_circuit_charges;

    		if(current_circuit->desequilibre > 0 && (*current_circuit->desequilibre_courant)[Last_station] > 0){

    			bool charge_courante_positive_finded = true;


    			while(charge_courante_positive_finded && this->solution->desequilibre > 0){//tant qu'il reste des stations à deplacer et du desequilibre a traiter
    				charge_courante_positive_finded = false;

					for(auto it2 = this->solution->circuits->begin(); it2 != this->solution->circuits->end(); ++it2){//on parcours les circuits à la recherche des circuit dont la charge final est non nul
						current_circuit_charges = *it2;

						if(U::to_s(*(current_circuit_charges->remorque)) != U::to_s(*(current_circuit->remorque))){//on evite de voiloir inserer-desinserer dans la meme stations
							// = *(--current_circuit_charges->stations->end());
							int deficit;
							int desequilibre_courant;
							for(auto it3 = current_circuit_charges->stations->begin(); it3 != current_circuit_charges->stations->end(); ++it3){
								Station*  Last_station_charge = *(it3);
								deficit = Last_station_charge->deficit();
								desequilibre_courant = (*current_circuit_charges->desequilibre_courant)[Last_station_charge];
							}
							//cout << "Last_station_charge : " << U::to_s(*Last_station_charge) << endl;
							//cout << "Last_station : " << U::to_s(*Last_station) << endl;
							//cout << "current_circuit : " << U::to_s(*(current_circuit->remorque)) << endl;
							//cout << "current_circuit_charges : " << U::to_s(*(current_circuit_charges->remorque)) << endl;
							//cout << "(*current_circuit_charges->charges_courante_min)[Last_station_charge] : " << (*current_circuit_charges->charges_courante_min)[Last_station_charge] << endl;
							//cout << "deficit_absorbe : " << deficit_absorbe << endl;
							//deficit_absorbe < (*current_circuit->charges_courante_min)[Last_station]
							cout << "ok1" << endl;
							if((*current_circuit->desequilibre_courant)[Last_station] != 0
									&& deficit <= 0
									&& desequilibre_courant == 0){
								charge_courante_positive_finded = true;
								cout << "ok" << endl;
								move(current_circuit_charges, current_circuit,
										current_circuit_charges->stations->size()-1, current_circuit->stations->size()-1);
								cout << "ko" << endl;
								this->solution->update();

							}
						}
					}
    			}
    		}


    		else if(current_circuit->desequilibre > 0 && (*current_circuit->desequilibre_courant)[Last_station] < 0){

    			bool charge_courante_positive_finded = true;


    			while(charge_courante_positive_finded && this->solution->desequilibre > 0){//tant qu'il reste des stations à deplacer et du desequilibre a traiter
    				charge_courante_positive_finded = false;

					for(auto it2 = this->solution->circuits->begin(); it2 != this->solution->circuits->end(); ++it2){//on parcours les circuits à la recherche des circuit dont la charge final est non nul
						current_circuit_charges = *it2;

						if(U::to_s(*(current_circuit_charges->remorque)) != U::to_s(*(current_circuit->remorque))){//on evite de voiloir inserer-desinserer dans la meme stations

							int deficit;
							int desequilibre_courant;
							for(auto it3 = current_circuit_charges->stations->begin(); it3 != current_circuit_charges->stations->end(); ++it3){
								Station* Last_station_charge = *(it3);
								deficit = Last_station_charge->deficit();
								desequilibre_courant = (*current_circuit_charges->desequilibre_courant)[Last_station_charge];
							}
							//cout << "Last_station_charge : " << U::to_s(*Last_station_charge) << endl;
							//cout << "Last_station5 : " << U::to_s(*Last_station) << endl;
							//cout << "current_circuit : " << U::to_s(*(current_circuit->remorque)) << endl;
							//cout << "current_circuit_charges : " << U::to_s(*(current_circuit_charges->remorque)) << endl;
							//cout << "(*current_circuit_charges->charges_courante_min)[Last_station_charge] : " << (*current_circuit_charges->charges_courante_min)[Last_station_charge] << endl;
							if((*current_circuit->desequilibre_courant)[Last_station] != 0//on a pas tout réparé
									&& deficit >= 0
									&& desequilibre_courant == 0){
								charge_courante_positive_finded = true;

								move(current_circuit_charges, current_circuit,
										current_circuit_charges->stations->size()-1, current_circuit->stations->size()-1);

								this->solution->update();

							}
						}
					}
    			}
    		}


    	}
    }
    if (old_desequilibre != this->solution->desequilibre)
    	return true;
    else
    	return false;
}



bool GreedySolver::Corrige_Greedy_seconde_passe(){
	int old_desequilibre = this->solution->desequilibre;
    bool heuristique_possible = false;

    if(this->solution->desequilibre > 0){//on ne retraite que si le dessequilibre est non nul
    	for(auto it = this->solution->circuits->begin(); it != this->solution->circuits->end(); ++it){//on cherche un circuit de desequilibre non nul
    		Circuit* current_circuit = *it;
    		Station* Last_station;
    		int iterateur_last_station = 0;
    		for(auto it3 = current_circuit->stations->begin(); it3 != current_circuit->stations->end(); ++it3){
    			Last_station = *it3;//Dernière station du circuit (On espère que c'est elle qui engendre le déficit)
    			if((*current_circuit->desequilibre_courant)[Last_station] != 0)
    				break;
    			iterateur_last_station++;
    		}
    		//cout << "Last_station : " << U::to_s(*Last_station) << " def : " << (*current_circuit->desequilibre_courant)[Last_station] << endl;
    		if(current_circuit->desequilibre > 0 && (*current_circuit->desequilibre_courant)[Last_station] < 0){//pour l'instant on ne traite que les deficit > 0
    			heuristique_possible = true;

    			int desequilibre_a_corriger = (*current_circuit->desequilibre_courant)[Last_station];
    			for(auto it2 = this->solution->circuits->begin(); it2 != this->solution->circuits->end(); ++it2){
    				Circuit* circuit = *it2;
    				bool break_seconde_boucle = false;
    				int iterateur_station_to_move = 0;
    				for(auto it3 = circuit->stations->begin(); it3 != circuit->stations->end(); ++it3){
    					Station* station = *it3;
    					if(station->deficit() == -desequilibre_a_corriger){
    						move(circuit, current_circuit, iterateur_station_to_move, iterateur_last_station);
    						break_seconde_boucle = true;
    						break;
    					}
    					iterateur_station_to_move++;
    				}
    				if(break_seconde_boucle)
    					break;
    			}

    		}
    	}
    }

    this->solution->update();
    if (old_desequilibre != this->solution->desequilibre)
    	return true;
    else
    	return false;
}




//./
