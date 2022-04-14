#ifndef REGRET_COMBINATION_H
#define REGRET_COMBINATION_H

#include "structure/cluster_t.h"
#include "structure/rnode_tree.h"
#include "QuadProg++.hh"
using namespace quadprogpp;

void combination(tuple_set *t_set, double *u, std::map<std::string, double> &categorical_value, double u_range, int &Qcount, int ROUND, int mode);

void combination_confidence(tuple_set *t_set, double *u, std::map<std::string, double> &categorical_value, double u_range,
                       int &Qcount, int ROUND, double diffRatio, int mode);

#endif //REGRET_COMBINATION_H
