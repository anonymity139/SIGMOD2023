#ifndef U_2_S_NODE_H
#define U_2_S_NODE_H

#include "point_t.h"
#include "hyperplane.h"

// spherical-tree related data structures
class s_node
{
public:
    bool is_leaf;
    point_t* center;
    double angle;
    std::vector<s_node*> child;
    std::vector<hyperplane*> hyper;

    s_node(int dim);
    double upper_orthog(point_t *n);
    double lower_orthog(point_t *n);
};


#endif //U_2_S_NODE_H
