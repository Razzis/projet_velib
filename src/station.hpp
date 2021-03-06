#ifndef STATION_H
#define STATION_H

#include "common.hpp"
#include "site.hpp"
#include "instance.hpp"

class Station : public Site {

public:

    int id; // interne et automatique

    // identificateur unique, automatique
    static int last_id;

    int capa; // capacité maxi de la station
    int ideal; // nombre idéla de vélo dans la station
    int nbvp;   // nombre de véo présent

    //utile pour trier les stations
    int barycentre_remorque_x;
    int barycentre_remorque_y;
    int dist_barycentre_remorque;

    Station(Instance* inst, string name, int x, int y,
                  int capa, int ideal, int nbvp);



    virtual ~Station();
    virtual string classname();

    virtual int margin();
    virtual int deficit();

    virtual string to_s();
    virtual string to_s_long();

};
bool compare_station(Station* const s1, Station* const s2);
void filtrate_list(vector<Station*>* station_list_pos, vector<Station*>* station_list_neg);
#endif
