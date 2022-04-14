#ifndef REGRET_SEPARATION_H
#define REGRET_SEPARATION_H

#include "structure/cluster_t.h"


void separation(tuple_set *t_set, double *u, std::map<std::string, double> &categorical_value, double u_range,
                      int &Qcount, int ROUND, int mode);

void separation_confidence(tuple_set *t_set, double *u, std::map<std::string, double> &categorical_value, double u_range,
                           int &Qcount, int ROUND, double diffRatio, int mode);


#endif //REGRET_SEPARATION_H
