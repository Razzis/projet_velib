#ifndef CIRCUIT_HPP
#define CIRCUIT_HPP

#include "common.hpp"
#include "instance.hpp"

// Un circuit représente le résultat d'une tournée associée à une remorque donnée.
// Les principaux attributs sont :
//   - inst : l'instance propriétaire
// Attributs primaires (indispensables pour construire une solution)
//   - remorque : la remorque correspondant à ce circuit,
//   - charge_init : charge initiale de la remorque
//   - tournees_data : tableau de paires [station, depot]
//     (n'est PAS mémorisé en attribut)
// Attributs dérivés (calculés à partir des attributs primaires)
//   - stations : la liste des stations gérées par cette remorque
//   - depots[Station] : la Hash des dépots des stations visitées
//   - arcs : liste des arcs représentant le chemin (plus pratique à exploiter
//     que la liste des stations visitées)
//
// Pour l'instant, un Circuit n'est pas intelligent : tout est piloté par la
// classe Solution (qui doit donc avoir un accès complet à tous les attributs)
//
// idée : créer une classe Service pour représenter un couple
//  <Station*, int depos>
class Circuit  {
public:

    // L'instance du problème
    Instance* inst;

    // La remorque associée à ce circuit
    Remorque* remorque;

    // charge initiale de la remorque
    int charge_init;
    //si il y a un choix sur la charge init
    int charge_init_min;
    int charge_init_max;

    //numeros de la stations de déficit relatif max et min
    int desequilibre_max;
    int desequilibre_min;

    int iterateur2desequilibreMax;
    int iterateur2desequilibreMin;

    map<Station*, int>* charges_init_max;
    map<Station*, int>* charges_init_min;
    map<Station*, int>* charges_courante_max;
    map<Station*, int>* charges_courante_min;
    map<Station*, int>* desequilibre_courant;

    // les stations visitées
    list<Station*>* stations;



    // Une map (Hash en Ruby ou Dict en python) contenant le dépos associé
    // à chaque remorque servie
    // depots = new map<Station*,int>()
    map<Station*,int>* depots;
    /// vector<int>* depots;

    // charge de la remorque **après** passage à une station
    // (peut-être utilisé pour vérifier la remorque ou optimiser une insertion)
    // TODO: ? on pourrait vouloir imposer charges->at(remorque) - charge_init ?
    map<Station*,int>* charges;
    //    map<Station*,int>* charges_min; // pas utilisé
    //    map<Station*,int>* charges_max; // pas utilisé
    /// vector<int>* charges;

    // Désequilibre total (toujours positif ou nul !)
    int desequilibre;

    // longueur de ce circuit
    int length;

    // la liste des arcs parcourus par la remorque
    // vector<Arc*> arcs;

    // Construction d'un circuit sans stations
    //
    Circuit(Instance* inst, Remorque* remorque);

    // Construction d'un circuit à partir d'une liste de stations
    //
    Circuit(Instance* inst, Remorque* remorque, list<Station*>* stations);

    // Attention : ceci n'est pas le constructeur par copie car il prend un
    // **pointeur** en parametre
    Circuit(const Circuit* circ);

    virtual ~Circuit();

    void copy(Circuit* other);

    // vide la liste des stations
    void clear();




    // Ne vide que les attributs dérivés à partir de la liste des stations
    // desservies (en vue de regénérer des attributs dérivés valides)
    void partial_clear();

    // Calul le dépos optimum en chaque staion, la charge initiale de la remorque
    // et les autres attributs dérivés
    // Pourrait s'appeler equilibate equilibrate, mais update est plus générique
    void update();

    //mise à jour des différente map de circuit (dépot, charges, ...)
    void maj_Depots();

    // retourne le coût mélangeant déséquilibre et distance totale (mesure
    // permettant la comparaison simple de deux solutions)
    inline int get_cost() {
        // return 1000*this->desequilibre + this->length;
        return 1000000*this->desequilibre + this->length;
    }
    // retourne une chaine de la forme  "1-1234"  séparant la valeur du déséquilibre
    // et la distance totale (pour affichage des messages ou dans le nom des
    // fichiers de solution)
    inline string get_cost_string() {
        return U::to_s(this->desequilibre) + "-" + U::to_s(this->length);
    }

    // procède à la mise à jour des attributs charge_init, charges et depots
    // de façon à equilibrer au mieux les stations gérées par la remorque de ce
    // circuit.
    void equilibrate();

    //equilibrage partiel : pour un circuit déjà équilibré auquel on a ajouté une station, on cherche a mettre à jour la charge init
    // et à determiner ou commence la second part (voir my_insert pour definission de la seconde part)
    int Partial_equilibrate(Station* s, list<Station*>::iterator insert_it, list<Station*>::iterator& it_second_part,
    		bool& SecondPartExiste, bool& stationAddedAsFirstOF2Part, int& deficit_from_partial_update, int& desequilibre_before_SecondPart);

    list<Station*>::iterator insert(Station* s, int pos=0);
    list<Station*>::iterator insert_best(Station* s);

    int insertCost(Station* s, int pos=0);
    int insert_bestCost(Station* s);

    list<Station*>::iterator my_insert(Station* s);

    int my_insertCost(Station* s, int& Best_iterateur);


    int my_insertSecondPart(Station* s, const list<Station*>::iterator& it2insert);
    int my_insertFirstPart(Station* s, const list<Station*>::iterator& it2insert);

    bool is_first_part(const list<Station*>::iterator& it);//determine si l'itérateur se trouve dans la partie qui determine lacharge init (first part) ou non
    int compute_cost(Station* s, int desequilibre, const list<Station*>::iterator& it2insert);
    //retire du circuit la station concerné
    void erase_station(const Station& station, list<Station*>::iterator  it);

    string to_s();
    string to_s_long();

};

// Décrit le circuit dans la sortie standart (cout)
std::ostream& operator<<( ostream &flux, Circuit const& circuit );
bool operator<(Circuit const& c1, Circuit const& c2);
#endif
//./

