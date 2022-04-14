#ifndef U_2_HYPERPLANE_SET_H
#define U_2_HYPERPLANE_SET_H

#include "point_t.h"
#include "hyperplane.h"
#include "point_set.h"
#include "inequality.h"
#include "tuple_set.h"

class hyperplane_set
{
public:
    std::vector<hyperplane*> hyperplanes;
    std::vector<point_t*> ext_pts;

    hyperplane_set();
    hyperplane_set(int dim, double u_range);//constructor: initial some hyperplanes so that u[i]>=0
    hyperplane_set(int d_num, int dt_cat, double u_range);
    ~hyperplane_set();

    //Prepare the file for computing the convex hull (the utility range R) via halfspace interaction
    void write(point_t* feasible_pt, char* filename);

    void print();//print the information of the hyperplane set

    void set_ext_pts(double u_range);//find all the extreme points of the hyperplane set, refine the bounded hyperplanes
    void find_boundary(point_set *p_set, double u_range);
    bool is_top_geq(std::vector<double*> &pset, double *v);
    bool is_top_geq(std::vector< std::pair<double*, int>> &pset, double *v);
    bool is_top_leq(std::vector<double*> &pset, double *v);
    bool is_top_leq(std::vector< std::pair<double*, int>> &pset, double *v);

    double top_leq_check(std::vector<inequality> &pset, double *v);
    double top_geq_check(std::vector<inequality> &pset, double *v);
    double top_leq_check_threshold(std::vector<inequality> &pset, double *v, double &utility);
    double top_geq_check_threshold(std::vector<inequality> &pset, double *v, double &utility);
    double istop_leq(std::vector<double*> &pset, double *v);
    double istop_geq(std::vector<double*> &pset, double *v);


    bool R_dominate(double *p1, double *p2);
    bool R_dominate_geq(std::vector<std::pair<double*, int>> &x, double *v);
    bool R_dominate_leq(std::vector<std::pair<double*, int>> &x, double *v);

    bool dominate(std::vector<inequality> &x, std::vector<point_t*> *ext, double *v, int direction);
    bool dominated(std::vector<inequality> &x, std::vector<point_t *> *ext, double *v, int direction, int *dom);
    double dominate_distance(std::vector<inequality> &x, std::vector<point_t *> *ext, double *v, int direction);

    void find_ext_leq(std::vector<inequality> &ineqleq, int i, std::vector<point_t*> &ext);
    void find_ext_geq(std::vector<inequality> &ineqgeq, int i, std::vector<point_t*> &ext);

    int check_relation(hyperplane *h);//check the relation between the hyperplane and the hyperplane set
    int check_relation_positive(hyperplane *h);//check whether the hyperplane set is on the positive side of the hyperplane

    void average_point(point_t* ap);


    double regret_bound(point_t *p1, point_t *p2);
    double regret_bound(point_set *P);
    double regret_bound(point_set *P, std::vector<int> &C_idx);
    double regret_bound(point_t *est_u, point_set *P);

    point_t* appx_center();

    void prune_same_cat(tuple_set *tset);


};


#endif //U_2_HYPERPLANE_SET_H
