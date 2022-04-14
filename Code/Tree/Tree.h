#ifndef RUN_TREE_H
#define RUN_TREE_H

#include "structure/cluster_t.h"
#include "structure/rnode_tree.h"
#include "QuadProg++.hh"
#include <sys/time.h>
#include "Ctree.h"

using namespace quadprogpp;


void special(tuple_set *t_set, double *u, std::map<std::string, double> &categorical_value, int &Qcount);

#endif //RUN_TREE_H
