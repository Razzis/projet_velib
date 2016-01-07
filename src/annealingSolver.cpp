#include "solver.hpp"
#include "voisinage.hpp"

int id_recuit = 1 ;

AnnealingSolver::AnnealingSolver(Instance* inst) : Solver::Solver(inst) {
    name = "AnnealingSolver";
    desc = "Solver par recuit simulé";
    this->solution = new Solution(inst);
    //cerr << "\nAnnealingSolver non implémenté : AU BOULOT !" << endl;
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

    // Parametres du(des) recuit(s) *********************************************************

    Solution* sol = new Solution(inst);
    double T; // température initiale
    double TAU_MIN = 0.0001;
    // *************************************************************************************




	// je sais pas trop quoi, récupère les args ********************************************
	
    if (log2()) {
        cout << "\n---CarloSolver::solve START size="
             << inst->stations->size() << "---\n";
    }
    Options* args = Options::args;
    // const string sinserter = args->station_inserter;
    // const string rchooser = args->remorque_chooser;
    int itermax = args->itermax;
    // Par défaut (-1) on ne fait qu'une seule itération
    itermax =  itermax == -1 ? 1 : itermax;
    // *************************************************************************************




	// appel à un glouton pour générer une première solution *******************************

	GreedySolver* solver = new GreedySolver (inst);
	solver->solve();
	// cas d'erreur du glouton :
	if (solver->found) {
		sol = solver->get_solution();
	} else {
		U::die("AnnealingSolver::Solve : echec du greedy");
	}
	// validité de la solution du glouton :
    //Ad : on pourrait zapper cette vérif,
    //ce qui compte c'est de construire une sol valide à partir de ça,
    //mais peu importe que celle ci le soit non ?
	bool is_valid = true;
	for (auto it = sol->circuits->begin();
				it != sol->circuits->end();
				++it) {
		if ( (*it)->length == 0 ) {
			is_valid = false;
		}
	}
	if(!is_valid) {
		U::die("Annealing::solve non valide");
	}

	//this->apply_one_greedy(sol);
    this->solution->copy(sol);
    // **************************************************************************************




    // Premier coup de recuit **************************************************************
    // température initiale en fct de la sol gloutonne
    //int desequilibre_sol = (sol->get_cost() - sol->get_cost()%1000000)/1000000;
    T = sol->get_cost();
    sol = this->recuit(sol, 1.2*T, 0.99, 50, 10*itermax, TAU_MIN);
    // *************************************************************************************




    // Second coup de recuit ***************************************************************
    //sol = this->recuit(sol, 0.1*sol->get_cost(), 0.99, 60, 10*itermax, TAU_MIN);
    // *************************************************************************************



    // tests des commandes de voisinage ***************************************************
    //(inutile, je le laisse au cas où quelqu'un veut tester par lui même)
    // Circuit* c1 = sol->circuits->at(0);
    // Circuit* c2 = sol->circuits->at(1);
    // exchange(c1,c1,4,1);
    // take(sol, c1,3,5,c2,0);
    // move(sol, c1,2,3,2);
    // reverse(sol,c1,1,4);
    // *************************************************************************************




	// récup de la sol et fin fct **********************************************************
    this->solution->copy(sol);
    this->solution->update();
    this->found = true;
    return found;
    // *************************************************************************************
}






Solution* AnnealingSolver::recuit(Solution* solution, double T0, double prog_geo_T, int max_prop, int itermax, double TAU_MIN) {

    // recup et init des param ************************************************************
    Solution* sol = new Solution(inst);
    sol->copy(solution);
    Solution* voisin = new Solution(solution->inst);
    double T = T0; // température initiale
    double tau = 1.0; // taux d'acceptation, sert pour le critère de convergence
    double p = 1.0; // init de la proba de garder une sol moins bonne
    double proba;
    int iter = 0; // pour compter le nb de tours
    bool derniere_variation = false;
    double T_last_var;
    int start_last_var;
    // ************************************************************************************
    
    


    // fichier pour récup un truc à plot *************************************************
    int id_recuit = 1;
    ofstream csv_file("Recuit"+U::to_s(id_recuit)+".csv", ios::out | ios::trunc);
    csv_file << "Iter,Temp,Cost,Taux d'acceptation,proba" << endl;
    csv_file << iter << "," << T << "," << sol->length << "," << tau << "," << p << endl;
    if (!csv_file) {
        U::die("AnnealingSolver::apply_one_recuit Erreur ouverture du csv file");
    }
    // ***********************************************************************************




    // début de la boucle ****************************************************************
    if (log2()) {
        cout << "\n" << iter << ": " << solution->get_cost();
    }
    while ( (tau>TAU_MIN)&&(iter<itermax) ) {
        
        cout << "." << flush;

        int count_proposition = 0;
        int count_accepted_proposition = 0;

        for (int i=0; i < max_prop; ++i) {
            
            // tirage d'une solution voisine
            voisin = select_voisin(sol, voisin);
            //voisin = select_voisin2(sol, voisin, i, max_prop);
        
            // On doit vérifier que chaque remorque visite au moins une station
            bool is_valid = true;
            for (auto it = voisin->circuits->begin();
                        it != voisin->circuits->end();
                        ++it) {
                if ( (*it)->length == 0 ) {
                    is_valid = false;
                }
            }

            if (is_valid) { 
                // voisin valide
                count_proposition++;

                if ( voisin->get_cost() < sol->get_cost() ) {
                    // voisin meilleur que la solution actuelle :
                    sol->copy(voisin);
                    if (log2()) {
                        //cout << "-" << flush;
                    }
                    count_accepted_proposition++;

                    if ( sol->get_cost() < solution->get_cost() ) {
                        // voisin meilleur que la meilleure solution
                        solution->copy(voisin);
                        if (log2()) {
                            cout << "\n" << iter << ": " << voisin->get_cost();
                        }
                        // On enregistre cette solution dans un fichier temporaire
                        //string tmpname = solution->get_tmp_filename();
                        //U::write_file(tmpname, solution->to_s());
                    }
                
                } else {
                    // voisin pas meilleur, testons si oui ou non on le garde
                    p = (double) exp(-static_cast<double>( voisin->get_cost() - sol->get_cost() )/T );
                    proba = ((double) rand() / (RAND_MAX));

                    if (proba<p && p!=1) {//on garde pas si c'est le même
                        // voisin accepté
                        sol->copy(voisin);
                        if (log2()) {
                            //cout << "+" << flush;
                        }
                        count_accepted_proposition++;
                    } else {
                        // voisin rejeté
                        //if (log2()) { cout << "." << flush; }
                    }
                
                }
            } else {
                // voisin invalide
                // log2("x"); // afficherait "L2x" or on veut seulement "x"
                //if (log2()) { cout << "x" << flush; }

            } // \if voisin valide

        } // \for sur les propositions

        iter++;

        // mise à jour taux d'acceptation
        tau = (double) count_accepted_proposition/count_proposition;

        // mise à jour de la température. La coder dans une fonction à part ?
        if (tau<TAU_MIN && !derniere_variation && id_recuit==2) {
            derniere_variation = true;
            T = 1.1*T;
            tau = 1.1*TAU_MIN;
            T_last_var = T;
            start_last_var = iter;
        }

        if (!derniere_variation || id_recuit==1) {
            T = prog_geo_T*T;
        } else if (derniere_variation) {
            T = T_last_var*log(double(start_last_var))/log(double(iter));
        }

        // écriture dans le csv
        csv_file << iter << "," << T << "," << sol->get_cost() << "," << tau << "," << p << endl;

    } // \while
    
    // fermeture du csv
    csv_file.close();

    // evitons les fuites memoire
    delete voisin;
    delete sol;
    //***************************************************************************************

    

    
    // récup de la sol et fin fct **********************************************************
    if (log2()) { cout << endl << flush; } // on ne veut pas voir le prefix "L2:"
    logn2("AnnealingSolver::solve: END");

    id_recuit++;
    solution->update();
    return solution;
    // *************************************************************************************
}











/* petit truc greedy pour créer une sol de base rapidement (repris du code du prof dans carloSolver) */
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
        random_shuffle(stations->begin(), stations->end());
    };

    int remorque_id = -1; // sélection des remorques à tour de rôle
    for (unsigned j = 0; j < stations->size(); j++) {
        Station* station = stations->at(j);

        if (rchooser == "ALT") {
            remorque_id = (remorque_id + 1) % inst->remorques->size();
        } else if (rchooser == "RAND") {
			//remorque_id = (remorque_id + 1) % inst->remorques->size();
            remorque_id = rand() % inst->remorques->size();
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
