#include "solver.hpp"
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
    if(schooser == "SORTED"){
    	station_list = inst->stations;
    	std::sort (station_list->begin(), station_list->end(), compare_station);
    }
    if(schooser == "DOUBLE"){
        	station_list = inst->stations;
        	std::sort (station_list->begin(), station_list->end(), compare_station);

        	*station_list_pos = vector<Station*>(*(inst->stations));
        	*station_list_neg = vector<Station*>(*(inst->stations));

        	filtrate_list(station_list_pos, station_list_neg);
        	std::sort (station_list_pos->begin(), station_list_pos->end(), compare_station);

        	std::sort (station_list_neg->begin(), station_list_neg->end(), compare_station);
        	//std::reverse(station_list->begin(), station_list->end());

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
    for(auto it = station_list->begin(); it != station_list->end(); ++it){
    	if (sinserter != "DOUBLE")
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
    	for(auto remorque_id = 0; remorque_id < tmp_sol.circuits->size(); remorque_id++)
    	{


    		logn7("greedy::solve Before attribution Circuit");
    		Circuit* circuit = (*circuit_list)[remorque_id];

    		// On ajoute la station dans un circuit temporaire
    		int insert_iterateur = -2;
    		int Cost_insert;
    		list<Station*>::iterator it_insert;
    		if (sinserter == "FRONT") {
    			Cost_insert = circuit->insertCost(station);
    		} else if (sinserter == "DOUBLE") {
    			Cost_insert_neg = 999999999;
    			if(iterateur_neg < station_list_neg->size()){
    				Cost_insert_neg = circuit->insertCost(station_list_neg->at(iterateur_neg), -1);
    				Cost_insert_neg = (Cost_insert_neg-Cost_insert_neg%1000000)/1000000;

    			}
    			Cost_insert_pos = 99999999;
    			if(iterateur_pos < station_list_pos->size()){
    				Cost_insert_pos = circuit->insertCost(station_list_pos->at(iterateur_pos), -1);
    				Cost_insert_pos = (Cost_insert_pos-Cost_insert_pos%1000000)/1000000;

    			}
    			/*cout << "iterateur pos : " << iterateur_pos << endl;
				cout << "iterateur neg : " << iterateur_neg << endl;
				cout <<"#neg "<< U::to_s(*(station_list_neg->at(iterateur_neg))) << endl;
				cout << "Cost_insert_pos : " << Cost_insert_pos << endl;
				cout << "Cost_insert_neg : " << Cost_insert_neg << endl;*/
    			if((Cost_insert_pos > Cost_insert_neg || iterateur_pos == station_list_pos->size())&& iterateur_neg < station_list_neg->size()){

    				Cost_insert = Cost_insert_neg;

    				station = (station_list_neg->at(iterateur_neg));




    			}
    			else if(Cost_insert_pos <= Cost_insert_neg && iterateur_pos < station_list_pos->size()){

    				Cost_insert = Cost_insert_pos;
    				//cout <<"#neg "<< U::to_s(*(station_list_neg->at(iterateur_neg))) << endl;
    				station = (station_list_pos->at(iterateur_pos));
    				//cout <<"#neg "<< U::to_s(*(station_list_neg->at(iterateur_neg))) << endl;

    			}
    			else{
    				cout << "iterateur pos : " << iterateur_pos << endl;
    				cout << "iterateur neg : " << iterateur_neg << endl;
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






			#ifdef VERIF_CIRCUIT_NON_VIDE
    		/* il faut s'assurer que les circuits ne sortent pas vide,
    		 * donc tant que tout les remplis, ne rempli que les circuit vide
    		 */
			if(circuit->desequilibre < deficit_min && (all_circuit_non_vide || !circuit_non_vide.at(remorque_id))){
				best_id = remorque_id;
				deficit_min = circuit->desequilibre;
				if (sinserter == "MYINSERT") {
					Best_iterateur = insert_interateur;
				}

			}
			#endif

			#ifndef VERIF_CIRCUIT_NON_VIDE


			if(Cost_insert < Best_Cost){
				// On cherche à determiner la meilleur remorque à laquelle ajouter la station
				best_id = remorque_id;
				if (sinserter == "DOUBLE") {
					if(Cost_insert == Cost_insert_pos)
						pos_deficit_choosed = true;
					else
						pos_deficit_choosed = false;
				}
				Best_Cost = Cost_insert;
				if (sinserter == "MYINSERT") {
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


		Circuit* best_circuit = (*sol_circuit_list)[best_id];
		Circuit* tmp_circuit = (*circuit_list)[best_id];
		if(sinserter == "DOUBLE") {
			if(pos_deficit_choosed)
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
		} else if (sinserter == "MYINSERT") {
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
    this->found = true;
    delete station_list_pos;
    delete station_list_neg;
    return found;
}

//./
