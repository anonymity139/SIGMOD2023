#ifndef PRUNING_H
#define PRUNING_H

#include "../structure/data_struct.h"
#include "../structure/data_utility.h"
#include "../structure/point_set.h"
#include "../structure/hyperplane_set.h"

#include "operation.h"
#include "lp.h"
#include "../structure/rtree.h"
#include "../UH/frame.h"
#include "../qhull/io.h"
#include <queue>

// the domination options
#define HYPER_PLANE 1
#define CONICAL_HULL 2

// the skyline options
#define SQL 1
#define RTREE 2

//  the stopping options
#define NO_BOUND 1
#define EXACT_BOUND 2
#define APPROX_BOUND 3

using namespace std;

/**
 * @brief Conduct halfspace intersection by invoking Qhull
 * @param rPtr   Contains the data of all the halfspace
 * @param wPtr   The extreme points of the intersection and the necessary halfspaces are shown in wPtr
 */
int halfspace(FILE* rPtr, FILE* wPtr);


/**
 * @brief Get the set of extreme points of R (bounded by the extreme vectors)
 * @param ext_vec    The hyperplanes(in the form of point) bounding the utility range R
 * @return  All the extreme points of R
 */
vector<point_t*> get_extreme_pts(vector<point_t*>& ext_vec);

int hyperplane_dom(point_t *p_i, point_t *p_j, vector<point_t *> ext_pts);

// use the seqentail way for maintaining the candidate set
void sql_pruning(point_set* P, vector<int>& C_idx, vector<point_t*>& ext_vec, double& rr, int stop_option, int dom_option);

// use the branch-and-bound skyline (BBS) algorithm for maintaining the candidate set
void rtree_pruning(point_set* P, vector<int>& C_idx, vector<point_t*>& ext_vec, double& rr,  int stop_option, int dom_option);

/**
 * @brief Get an "exact" upper bound bound in O(|ext_pts|^2) time based on R
 * @param ext_pts       All the hyperplanes(in the form of points) bounding R
 * @return The regret ratio
 */
double get_rrbound_exact(vector<point_t*> ext_pts);

#endif