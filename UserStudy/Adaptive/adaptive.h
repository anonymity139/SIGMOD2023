#ifndef REGRET_ADAPTIVE_H
#define REGRET_ADAPTIVE_H

#include "define.h"
#include "hyperplane_set.h"
#include "cluster_h.h"
#include <s_node.h>
#include <math.h>
#include <stack>
#include <fstream>
#include "QuadProg++.hh"
using namespace quadprogpp;

void Adaptive(tuple_set *car_set, point_set *origset, point_t* u, int &Qcount, int &TID, std::ofstream &fp);


#endif //REGRET_ADAPTIVE_H
