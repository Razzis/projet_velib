#include "solver.hpp"
#include <map>
#include <vector>

#define OPTIM_JUST_DESEQUILIBRE
//#define VERIF_CIRCUIT_NON_VIDE//fait on une recherche pour savoir si les circuits sont vides

GreedySolver::GreedySolver(Instance* inst) : Solver::Solver(inst) {
    name = "GreedySolver";
    desc = "Solver par méthode glouton (intelligent)";
    cerr << "\nGreedySolver non implémenté : AU BOULOT !" << endl;
    logn1(name + ": " + desc + " inst: " + inst->name);


}
GreedySolver::~GreedySolver()  {
    // TODO
}
// Méthode principale de ce solver, principe :
//
bool GreedySolver::solve() {
/* TODO : il n'est pas nécéssaire d'iserer lles station dans les circuit, juste de récupérer
 * les coûts engendré */
    found = false;


    this->solution = new Solution(inst);
    Solution* tmp_sol = new Solution(inst);

    cout << "debut du remplissage des remorques" << endl;



    /********************************************/

    /* minimisation du déséquilibre et des distances */

    Options* args = Options::args;
        const string sinserter = args->station_inserter;
        const string schooser = args->station_chooser;
        const string rchooser = args->remorque_chooser;

    vector<bool> circuit_non_vide(inst->remorques->size(),false);//va permettre de s'assurer qu'il  y a au moins une station par circuit
    bool all_circuit_non_vide = false;

    // parcours des stations de l'instance
    int Best_Cost;
    int Best_iterateur;
    vector<Station*>* station_list;
    if(schooser == "SORTED"){
    	station_list = inst->stations;
    	std::sort (station_list->begin(), station_list->end());
    }
    else if(schooser == "RSORTED"){
        station_list = inst->stations;
        std::sort (station_list->begin(), station_list->end());
        std::reverse(station_list->begin(), station_list->end());
    }
    else{
    	station_list = inst->stations;
    }
    for(auto it = station_list->begin(); it != station_list->end(); ++it){
    	auto best_id = -1;
    	Best_Cost = 999999999;
    	Best_iterateur = -2;
    	Station* station = *it;
    	logn2("GreedySolver::solve ajout de la station : "+U::to_s(*station));



    	/** Pour chaque station, on determine le circuit le mieu adapté (qui minimise les desequilibre et eventullement la distance)
    	 * et on l'ajoute au circuit concerné
    	 */
    	// On parcours les différentes remorques à laquelle on pourrait atribuer la station
    	for(auto remorque_id = 0; remorque_id < inst->remorques->size(); remorque_id++)
    	{
    		logn7("greedy::solve Before attribution Circuit");
    		Circuit* circuit = tmp_sol->circuits->at(remorque_id);
    		logn7("greedy::solve attribution de la station " + U::to_s(*station) + " à la remorque " + U::to_s(*(circuit->remorque)));
    		// On ajoute la station dans un circuit temporaire
    		int insert_iterateur = -2;
    		int Cost_insert;
    		if (sinserter == "FRONT") {
    			Cost_insert = circuit->insertCost(station);
    		} else if (sinserter == "BACK") {
    			Cost_insert = circuit->insertCost(station, -1);
    		} else if (sinserter == "BEST") {
    			Cost_insert = circuit->insert_bestCost(station);
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
				Best_Cost = Cost_insert;
				if (sinserter == "MYINSERT") {
					Best_iterateur = insert_iterateur;
				}

				/*#ifdef OPTIM_JUST_DESEQUILIBRE
				// Si le desequilibre est nul, on peut choisir cette remorque (on ne trouvera pas mieu)
				if(circuit->desequilibre == 0)
					break;
				#endif*/
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

		Circuit* best_circuit = this->solution->circuits->at(best_id);
		Circuit* tmp_circuit = tmp_sol->circuits->at(best_id);

		// On ajoute la station au circuit choisis
		if (sinserter == "FRONT") {
			best_circuit->insert(station);
			tmp_circuit->insert(station);
		} else if (sinserter == "BACK") {
			best_circuit->insert(station, -1);
			tmp_circuit->insert(station, -1);
		} else if (sinserter == "BEST") {
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


		best_circuit->update();
		tmp_circuit->update();
		//cout << *best_circuit << endl;
		//cout << *this->solution->circuits->at(1);


    }

    logn5("Circuit::greedy fin solve, before update");
    this->solution->update();
    this->found = true;

    return found;
}

//./
