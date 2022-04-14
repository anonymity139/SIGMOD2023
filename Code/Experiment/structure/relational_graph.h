#ifndef U_2_RELATIONAL_GRAPH_H
#define U_2_RELATIONAL_GRAPH_H

#include "hyperplane_set.h"
#include "tuple_set.h"
#include "cluster_t.h"
#include "r_node.h"
#include "rnode_tree.h"
#include <queue>

class relational_graph
{
public:
    std::vector<r_node*> list;
    std::vector<node_relation*> rlist;

    relational_graph();
    explicit relational_graph(cluster_t* clu);
    relational_graph(cluster_t *clu, rnode_tree *tree);
    void set_relation();
    void set_relation_fast(rnode_tree *tree);
    void findRelation(r_node *nd, rnode_tree *tree);


    void print();
    void print_rlist();
    void print_list();
    int find_node(std::string *s1, std::string *s2, int &direction);
    void update_basedR(hyperplane_set* R);
    void update_all(tuple_t *tp1, tuple_t *tp2, hyperplane_set *R, double u_range, int x);

    void update_all_round(tuple_t *tp1, tuple_t *tp2, hyperplane_set *R, double u_range, int x, rnode_tree *tree, int ROUND);
    void update_all_threshold(tuple_t *tp1, tuple_t *tp2, hyperplane_set *R, double u_range, int x, rnode_tree *tree, double THRESHOLD);

    void prune(tuple_set *t_set, std::vector<point_t*> &R);
    double regret_ratio(cluster_t *c, std::vector<point_t*> &R, int &sid);
    double regret_ratio(cluster_t *c, hyperplane_set *R, tuple_t *&maxp);
    double regret_ratio2(cluster_t *c, hyperplane_set *R, point_t*ap, tuple_t *&maxp);
};


#endif //U_2_RELATIONAL_GRAPH_H
