#ifndef U_2_R_NODE_H
#define U_2_R_NODE_H
#include "hyperplane_set.h"
#include "tuple_set.h"
#include "cluster_t.h"
#include "../Others/operation.h"
#include <iomanip>

class r_node;

class node_relation
{
public:
    r_node *node_1;
    r_node *node_2;
    r_node *node_sum;
    int type[2]; //type[0]: A + B = C/2C or A + B =-C/-2C   type[1]: A+ B or A-B
    int updated_count;
    bool expired;

    node_relation();
    node_relation(r_node* n1, r_node* n2, r_node* nodesum, int ty1, int ty2);
    void print();
};


class r_node
{
public:
    int ID, dim, len, updated, is_modified, isRelationFound;
    std::string* s_1;
    std::string* s_2;
    std::map<int, int> relationNode;
    std::vector<inequality> ineqleq, ineqgeq;//<= >=, true denotes newly added
    vector<point_t*> *ext_leq, *ext_geq;
    std::vector< std::pair<node_relation*, int>> relation;//int: the node's place in the relation
    std::vector< std::pair<tuple_set*, tuple_set*>> sets;

    r_node();
    r_node(std::string *s1, std::string *s2, int d_num, int d_cat);
    r_node(tuple_set* t_1, tuple_set* t_2);
    r_node(tuple_set *t_1, tuple_set *t_2, int index);
    ~r_node();
    bool fit(tuple_set* t_1, tuple_set* t_2, int index);
    int fit(std::string* s1, std::string* s2);
    void print();
    void print_cat();
    int is_sum(r_node* n1, r_node* n2);
    int is_difference(r_node* n1, r_node* n2);
    int add(r_node *n, string *str);
    int subtract(r_node *n, string *str);
    int add2(r_node *n, string *str);
    int subtract2(r_node *n, string *str);
    int is_sum_posi(r_node *n1, r_node *n2);
    void update(std::vector<point_t*> &R);
    void update(hyperplane_set *R);
    int update(double *v, int direction, hyperplane_set *R, int round, std::vector<double> &c);
    int update_round(double *v, int direction, hyperplane_set *R, int round, std::vector<double> &c, double u_range);
    int update_threshold(double *v, int direction, hyperplane_set *R, int round, std::vector<double> &c, double u_range, double THRESHOLD);
    int update_without_check(double *v, int direction, hyperplane_set *R, std::vector<double> &c);
    int update_without_check_round(double *v, int direction, hyperplane_set *R, std::vector<double> &c, double u_range);
    int update_without_check_threshold(double *v, int direction, hyperplane_set *R, std::vector<double> &c, double u_range, double THRESHOLD);

    void update_numerical_utilityrange(double *v, int direction, hyperplane_set *R, double urange);
    void update_tupleset_relation(int direction);
    void update(std::pair<node_relation*, int> nr, int direction, hyperplane_set *R, int round);
    void update_round(std::pair<node_relation*, int> nr, int direction, hyperplane_set *R, int round, double u_range);
    void update_threshold(std::pair<node_relation*, int> nr, int direction, hyperplane_set *R, int round, double u_range, double THRESHOLD);

    void update_coeff();
    void clean(int direction);
    void clean();
    void tuple_prune(hyperplane_set* R);
    void tuple_prune_all_rnode(hyperplane_set *R);
    double tuple_prune_all_rnode_distance(hyperplane_set *R);
    double regret_check(tuple_t *t, vector<tuple_t*> &tuples, int position, std::vector<point_t*> &R);

    bool order_check(tuple_t *tp1, tuple_t *tp2, int position, point_t *ut);

    void tuple_prune_all_rnode_aggre(hyperplane_set *R);
};

#endif //U_2_R_NODE_H
