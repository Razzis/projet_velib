#include "solver.hpp"
#include "voisinage.hpp"

double TAU_MIN = 0.001;
int id_recuit = 1;

AnnealingSolver::AnnealingSolver(Instance* inst) : Solver::Solver(inst) {
    name = "AnnealingSolver";
    desc = "Solver par recuit simulé";
    //exit(1);
}
AnnealingSolver::~AnnealingSolver()  {
	delete this->solution;
    // TODO
}
// Méthode principale de ce solver, principe :
//
bool AnnealingSolver::solve() {
    found = false;


	/* je sais pas trop quoi, récupère les args */
	if (log2()) {
        cout << "\n---CarloSolver::solve START size="
             << inst->stations->size() << "---\n";
    }
    Options* args = Options::args;
    // const string sinserter = args->station_inserter;
    // const string rchooser = args->remorque_chooser;
    srand(args->seed);
    int itermax = args->itermax;
    // Par défaut (-1) on ne fait qu'une seule itération
    itermax =  itermax == -1 ? 1 : itermax;



	/******************************************************************/

	Solution* sol = new Solution(inst);
	int max_prop = 10;

	//sol = apply_one_greedy(sol);
	GreedySolver* solver = new GreedySolver(inst);
	solver->solve();

	if (solver->found)
		sol = solver->get_solution();
	else
		U::die("AnnealingSolver::Solve : echec du greedy");


	bool is_valid = true;
	for (auto it = sol->circuits->begin();
				it != sol->circuits->end();
				++it) {
		if ( (*it)->length == 0 ) {
			is_valid = false;
		}
	}
	if(!is_valid)
		U::die("Annealing::solve Pas valide");



	this->solution = sol;
	int desequilibre_sol = (sol->get_cost() - sol->get_cost()%1000000)/1000000;
	sol = this->apply_one_recuit(sol, 0.1*desequilibre_sol, 0.90, max_prop, args->itermax);

	//dans le deuxieme recuit on utilise un voisinage plus grossier
	sol = this->apply_one_recuit(sol, 0.1*sol->get_cost(), 0.90, 10*max_prop, 2*args->itermax);

	/* récup de la sol et fin fct */
	this->solution = sol;
	//delete sol;
    sol->update();
    this->solution->update();
    this->found = true;
    //this->solution = sol;
    return found;
}




Solution* AnnealingSolver::apply_one_recuit(Solution* sol, double T0, double coeff_maj_T0, double nb_explo_voisin, int nb_iteration_max) {
	/* je sais pas trop quoi, récupère les args */
	if (log2()) {
        cout << "\n---CarloSolver::solve START size="
             << inst->stations->size() << "---\n";
    }
    Options* args = Options::args;
    // const string sinserter = args->station_inserter;
    // const string rchooser = args->remorque_chooser;
    srand(args->seed);
    // Par défaut (-1) on ne fait qu'une seule itération
    int itermax =  nb_iteration_max == -1 ? 1 : nb_iteration_max;

    // Best_sol = new Solution(inst);
    Solution* Best_sol = sol;
	/******************************************************************/

	Solution* tmp_sol = new Solution(inst);
	tmp_sol->copy(sol);
	double T = T0; // température initiale
	double tau = 1.0; // taux d'acceptation, sert pour le critère de convergence
	double p = 1.0; // init de la proba de garder une sol moins bonne
	double proba;
	int max_prop = nb_explo_voisin;
	bool derniere_variation = false;
	double T_init_petite_variation;
	int iter_init_petite_variation;


	/* grosse boucle des familles */
	int iter = 0; // pour compter le nb de tours
	if (log2()) {
		cout << "\n" << iter << ": " << this->solution->get_cost();
    }

	ofstream csv_file("Recuit"+U::to_s(id_recuit)+".csv", ios::out | ios::trunc);

	csv_file << "Iter,Temp,Cost,Taux d'acceptation,proba" << endl;
	csv_file << iter << "," << T << "," << tmp_sol->get_cost() << "," << tau << "," << p << endl;
;
	if(csv_file){
		while ( (tau>TAU_MIN)&&(iter<itermax) ) {






			int count_proposition = 0;
			int count_accepted_proposition = 0;
			for (int i=0; i < max_prop; ++i) {
				// tirage d'une solution voisine
				Solution voisin = Solution(inst);
				select_voisin(&voisin, tmp_sol, id_recuit);//pas les même voisinage suivant le nuemros de recuit
				/*cout << "######### sol ###########" << endl;
				cout << sol->to_s_long() << endl;
				cout << "######### voisin ###########" << endl;
				cout << voisin->to_s_long() << endl;*/
				// On doit vérifier que chaque remorque visite au moins une station
				bool is_valid = true;
				for (auto it = voisin.circuits->begin();
							it != voisin.circuits->end();
							++it) {
					if ( (*it)->length == 0 ) {
						is_valid = false;
					}
				}


				if (is_valid) {

					// voisin valide
					int voisin_cost;
					int tmp_sol_cost;
					int Best_sol_cost;
					if(id_recuit == 1){
						voisin_cost = voisin.desequilibre;
						tmp_sol_cost = tmp_sol->desequilibre;
					}
					else{
						voisin_cost = voisin.get_cost();
						tmp_sol_cost = tmp_sol->get_cost();
					}
					//cout << "voisin_cost : " << voisin_cost << ", sol_cost : " << tmp_sol_cost << endl;

					if ( voisin_cost < tmp_sol_cost ) {
						// voisin meilleur que la solution actuelle :
						tmp_sol->copy(&voisin);
						if (log2()) {
							//cout << "-" << flush;
						}
						//count_proposition++;
						count_accepted_proposition++;




						if(id_recuit == 1){
							tmp_sol_cost = tmp_sol->desequilibre;
							Best_sol_cost = Best_sol->desequilibre;
						}
						else{
							tmp_sol_cost = tmp_sol->get_cost();
							Best_sol_cost = Best_sol->get_cost();
						}
						if ( tmp_sol_cost < Best_sol_cost ) {
							// voisin meilleur que la meilleure solution
							Best_sol->copy(&voisin);
							if (log2()) {
								cout << "\n" << iter << ": " << voisin.get_cost();
							}
						// On enregistre cette solution dans un fichier temporaire
						//string tmpname = this->solution->get_tmp_filename();
						//U::write_file(tmpname, this->solution->to_s());
						}

					} else {								// voisin pas meilleur, testons si oui ou non on le garde
						p = (double) exp(- static_cast<double>( voisin_cost - tmp_sol_cost )/T );
						//cout << "p : " << p << endl;
						//cout << "delta E : "<< voisin->get_cost() - sol->get_cost() << endl;
						/*if(voisin->get_cost() - sol->get_cost() == 0)
							U::die("meurt voisin !!! ");*/
						proba = ((double) rand() / (RAND_MAX));
						if (proba<p && (voisin_cost - tmp_sol_cost) != 0) {

							/*cout << "######### sol ###########" << endl;
							cout << sol->to_s_long() << endl;
							cout << "######### voisin ###########" << endl;
							cout << voisin->to_s_long() << endl;*/

							// voisin accepté
							tmp_sol->copy(&voisin);

							//cout << "+" << flush;

							//count_proposition++;
							count_accepted_proposition++;
							// mise à jour de la température
							//T = 0.8*T;
						} else {
							// voisin rejeté

							//cout << "." << flush;

							//count_proposition++;
						}

					}
				//cout << "voisin_cost : " << voisin_cost << ", tmp_cost : " << tmp_sol_cost << ", p : " << p << endl;
				} else {
					// voisin invalide
					// log2("x"); // afficherait "L2x" or on veut seulement "x"
					if (log2()) { cout << "x" << flush; }
				}
				count_proposition++;


			}

			iter++;
			// mise à jour taux d'acceptation
			tau = (double) count_accepted_proposition/count_proposition;
			cout << "count_proposition : " << count_proposition << endl;
			cout << "count_accepted_proposition : " << count_accepted_proposition << endl;
			cout << "tau : " << tau << endl;
			csv_file << iter << "," << T << "," << tmp_sol->get_cost() << "," << tau << "," << p << endl;
			//if(iter == 1)
				//U::die("voisin_cost");
			// MAJ temperature
			//a la fin, on redonne un coup de boost a la temperature
			if(tau<TAU_MIN && !derniere_variation && id_recuit == 2){
				derniere_variation=true;
				T=1.1*T;
				tau = 1.1*TAU_MIN;
				T_init_petite_variation = T;
				iter_init_petite_variation = iter;
			}

			if(!derniere_variation || id_recuit == 1)
				T = coeff_maj_T0*T;
			else if(derniere_variation){
				T = T_init_petite_variation*log(double(iter_init_petite_variation))/log(double(iter));
			}


			/*if(tau<TAU_MIN && derniere_variation && id_recuit == 2){
				derniere_variation=false;
				T=0.2*T0;
				coeff_maj_T0 = 1-0.5*(1-coeff_maj_T0);
				tau = 1.1*TAU_MIN;
			}*/

		} // \while
		csv_file.close();
	}
	else{
		U::die("AnnealingSolver::apply_one_recuit Erreur ouverture du csv file");
	}

	if (log2()) { cout << endl << flush; } // on ne veut pas voir le prefix "L2:"
	logn2("AnnealingSolver::solve: END");
	id_recuit++;


	return Best_sol;
}











/* petit truc greedy pour créer une sol de base (j'ai enlevé tout random pour faire des test, on peut les remettre), repris du code du prof dans carloSolver */
Solution* AnnealingSolver::apply_one_greedy(Solution* sol) {
    // U::die("CarloSolver::apply_one_greedy: non implémenté !");
    logn4("CarloSolver::apply_one_greedy BEGIN");
    sol->clear();
    Options* args = Options::args;
    const string sinserter = args->station_inserter;
    const string schooser = args->station_chooser;
    const string rchooser = args->remorque_chooser;

    auto stations = new vector<Station*>(*inst->stations);
    if (schooser == "RAND") {
        // On mélange le vector par la lib standard c++
        //random_shuffle(stations->begin(), stations->end());
    };

    int remorque_id = -1; // sélection des remorques à tour de rôle
    for (unsigned j = 0; j < stations->size(); j++) {
        Station* station = stations->at(j);

        if (rchooser == "ALT") {
            remorque_id = (remorque_id + 1) % inst->remorques->size();
        } else if (rchooser == "RAND") {
			remorque_id = (remorque_id + 1) % inst->remorques->size();
            //remorque_id = rand() % inst->remorques->size();
        } else {
            U::die("remorque_chooser inconnu : " + U::to_s(rchooser));
        }
        Circuit* circuit = sol->circuits->at(remorque_id);

        // if (circuit->remorque->name == "r2" && station->name == "s8") {
        //     Log::level += 7;
        // }
        if (sinserter == "FRONT") {
            circuit->insert(station);
        } else if (sinserter == "BACK") {
            circuit->insert(station, -1);
        } else if (sinserter == "BEST") {
            circuit->insert_best(station);
        } else if (sinserter == "MYINSERT") {
           circuit->my_insert(station);
        } else {
            U::die("station_inserter inconnu : " + U::to_s(sinserter));
        }
    }
    logn5("annealingSolver::apply_one_greedy: avant appel à sol->update");
    sol->update();

    logn4("annealingSolver::apply_one_greedy END");
    return sol;
};
//./
