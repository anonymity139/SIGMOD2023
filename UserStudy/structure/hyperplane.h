#ifndef U_2_HYPERPLANE_H
#define U_2_HYPERPLANE_H

#include "point_t.h"
#include "tuple_t.h"

class hyperplane
{
public:
    int d;          //dimension
    double* norm;   //parameter of the norm vector
    double offset;
    point_t* p_1;
    point_t* p_2;
    tuple_t* t_1;
    tuple_t* t_2;

    hyperplane(hyperplane* h);
    hyperplane(point_t* p1, point_t* p2, double offset);
    hyperplane(tuple_t* t1, tuple_t* t2, double offset);//consider numerical part of the tuple
    hyperplane(int d, double* p1, double* p2, double offset);
    hyperplane();
    hyperplane(int dim);
    hyperplane(int dim, double* n, double offset);
    ~hyperplane();
    bool is_same(hyperplane *h);//Check whether the hyperplane is the same as h
    void print();
    void normalization();
    double intersection_len(point_t* p);//compute the distance between the intersection of a hyperplane and a ray shooting
};


#endif //U_2_HYPERPLANE_H
