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
    for(auto it = inst->stations->begin(); it != inst->stations->end(); ++it){
    	Station* station = *it;
    	logn2("ajout de la station : "+U::to_s(*station));

    	auto deficit_min = 1000000;
    	auto length_min = 1000000;
    	auto best_id = -1;

    	/** Pour chaque station, on determine le circuit le mieu adapté (qui minimise les desequilibre et eventullement la distance)
    	 * et on l'ajoute au circuit concerné
    	 */
    	// On parcours les différentes remorques à laquelle on pourrait atribuer la station
    	for(auto remorque_id = 0; remorque_id < inst->remorques->size(); remorque_id++)
    	{
    		Circuit* circuit = tmp_sol->circuits->at(remorque_id);
    		// On ajoute la station dans un circuit temporaire
    		list<Station*>::iterator  it_insert;
    		if (sinserter == "FRONT") {
    			it_insert = circuit->insert(station);
    		} else if (sinserter == "BACK") {
    			it_insert = circuit->insert(station, -1);
    		} else if (sinserter == "BEST") {
    			it_insert = circuit->insert_best(station);
    		} else {
    			U::die("station_inserter inconnu : " + U::to_s(sinserter));
    		}

    		logn5("greedy::solve insertion de la station dans le circuit : done");
    		circuit->update();
    		logn5("greedy::solve circuit mis à jour");






			#ifdef VERIF_CIRCUIT_NON_VIDE
    		/* il faut s'assurer que les circuits ne sortent pas vide,
    		 * donc tant que tout les remplis, ne rempli que les circuit vide
    		 */
			if(circuit->desequilibre < deficit_min && (all_circuit_non_vide || !circuit_non_vide.at(remorque_id))){
				best_id = remorque_id;
				deficit_min = circuit->desequilibre;
			}
			#endif

			#ifndef VERIF_CIRCUIT_NON_VIDE
			if(circuit->desequilibre < deficit_min){
				// On cherche à determiner la meilleur remorque à laquelle ajouter la station
				best_id = remorque_id;
				deficit_min = circuit->desequilibre;
				/*#ifdef OPTIM_JUST_DESEQUILIBRE
				// Si le desequilibre est nul, on peut choisir cette remorque (on ne trouvera pas mieu)
				if(circuit->desequilibre == 0)
					break;
				#endif*/
			}
			#endif

			#ifndef OPTIM_JUST_DESEQUILIBRE
			else if(circuit->length < length_min && circuit->desequilibre == deficit_min){
				best_id = remorque_id;
				length_min = circuit->length;
			}
			#endif

			//retour à la solution temporaire (on la remet à la dernière "meilleur solution")
			circuit->erase_station(*station, it_insert);
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
		} else {
			U::die("station_inserter inconnu : " + U::to_s(sinserter));
		}
		/*best_circuit->update();
		cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
		cout << "BEST" << endl;
		cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
		cout << *best_circuit;*/


    }


    this->solution->update();
    this->found = true;
    return found;
}

//./
