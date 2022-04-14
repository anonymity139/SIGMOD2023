#ifndef MAXUTILITY_H
#define MAXUTILITY_H

#include "../structure/define.h"
#include "../structure/data_struct.h"
#include "../structure/data_utility.h"

#include <vector>
#include <algorithm> 

#include "../structure/rtree.h"
#include "../Others/lp.h"
#include "../Others/pruning.h"
#include <queue>

#define RANDOM 1
#define SIMPLEX 2
#define SIMPLEX_FLY 3

using namespace std;

// the main interactive algorithm
point_t* max_utility(point_set* P, point_t* u, int s,  double epsilon, int maxRound, int &Qcount, double &Csize,
                     int cmp_option, int stop_option, int prune_option, int dom_option, int mode);

#endif