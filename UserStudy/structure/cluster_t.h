#ifndef U_2_CLUSTER_T_H
#define U_2_CLUSTER_T_H


#include <vector>
#include "point_set.h"
#include "tuple_set.h"
#include "hyperplane_set.h"

class cluster_t
{
public:
    std::vector<tuple_set*> clusters;
    std::vector<std::string*> label;

    cluster_t();
    explicit cluster_t(tuple_set *t_set);
    explicit cluster_t(cluster_t *clu_t);
    ~cluster_t();
    void print();
    void print_tuple();
    void prune(std::vector<point_t*> &R);
    void find_tupleset_relationexist(std::vector<int> &sid);
    void select_tuple(tuple_t *&t1, tuple_t *&t2, int &cid_1, int &cid_2, int &tid_1, int &tid_2);
    void select_tuple_sameset(tuple_t *&t1, tuple_t *&t2, int &cid_1, int &cid_2, int &tid_1, int &tid_2);
    int count_tuple();
    int count_cluster();
    double regret_ratio(hyperplane_set *R, int dim);
    void maintain_one(hyperplane_set *R, int dim);
    double regret_ratio(hyperplane_set *R, double *u, std::map<std::string, double> &categorical_value, double max_value, tuple_t *&maxp);
    double regret_bound(hyperplane_set *R, tuple_t *tpl);

    void prune(hyperplane_set *R);
};


#endif //U_2_CLUSTER_T_H
