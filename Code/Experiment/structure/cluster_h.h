#ifndef U_2_CLUSTER_H_H
#define U_2_CLUSTER_H_H

#include "point_t.h"
#include "hyperplane.h"

//a cluster of points/vectors described by numerical attributes
class cluster_h
{
public:
    point_t* center;
    std::vector<hyperplane*> h_set;

    cluster_h();
    ~cluster_h();
};


#endif //U_2_CLUSTER_H_H
