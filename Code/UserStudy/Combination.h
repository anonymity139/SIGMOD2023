#ifndef REGRET_COMBINATION_H
#define REGRET_COMBINATION_H

#include "structure/cluster_t.h"
#include "structure/rnode_tree.h"
#include "QuadProg++.hh"
using namespace quadprogpp;

void combination_round(tuple_set *car_set, tuple_set *t_set, double *u, std::map<std::string, double> &categorical_value, double u_range, int &Qcount, int ROUND, int &TID, ofstream &fp);



#endif //REGRET_COMBINATION_H
