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
#include "operation.h"
using namespace quadprogpp;

void Adaptive(point_set *origset, point_t* u, int &Qcount, int mode);

void Adaptive_prune(point_set *origset, point_t* u, int &Qcount, int mode);

#endif //REGRET_ADAPTIVE_H
