#ifndef REGRET_SEPARATION_H
#define REGRET_SEPARATION_H

#include "structure/cluster_t.h"

void separation_round(tuple_set *car_set, tuple_set *t_set, double *u, std::map<std::string, double> &categorical_value, double u_range, int &Qcount, int ROUND, int &TID, std::ofstream &fp);


#endif //REGRET_SEPARATION_H
