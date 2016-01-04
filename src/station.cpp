#include "station.hpp"

// initialisation des variables statiques a l'exterieur de toute methode
int Station::last_id = 0;

string Station::classname() {
    return "Station";
}

Station::Station(Instance* inst, string name, int x, int y,
                  int capa, int ideal, int nbvp)
        : Site::Site(inst, name, x, y) {

    this->id = Station::last_id++;

    this->capa = capa;
    this->ideal = ideal;
    this->nbvp = nbvp;
}
Station::~Station() {
}

string Station::to_s() {
    ostringstream buf;
    buf << "station " << setw(5) << name
        << setw(5) << x << setw(4) << y
        << setw(8) << capa << setw(6) << ideal
        << setw(5) << nbvp ;
    return buf.str();
}

string Station::to_s_long() {
    ostringstream buf;
    buf << Site::to_s_long() << " id=" << id
                             << " capa=" << capa
                             << " ideal=" << ideal
                             << " nbvp="   << nbvp
                             ;
    return buf.str();
}
int Station::margin() {
    return this->capa - this->nbvp;
}
int Station::deficit() {
    return this->ideal - this->nbvp;
}
bool compare_station(Station* const s1, Station* const s2){
	if(s1->ideal - s1->nbvp < s2->ideal - s2->nbvp)
		return true;
	else
		return false;
}
void filtrate_list(vector<Station*>* station_list_pos, vector<Station*>* station_list_neg){

	auto it_neg = station_list_neg->begin();
	for(auto it_pos = station_list_pos->begin(); it_pos != station_list_pos->end(); ++it_pos){
		Station* station = *it_pos;
		if(station->deficit() >= 0){
			station_list_neg->erase(it_neg,station_list_neg->end());
			station_list_pos->erase(station_list_pos->begin(),it_pos--);
			break;
		}
		++it_neg;

	}



}


//./
