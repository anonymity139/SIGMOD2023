#ifndef U_2_POINT_SET_H
#define U_2_POINT_SET_H

#include "point_t.h"
#include <string>


class point_set
{
public:
    std::vector<point_t*> points;

    point_set();
    point_set(point_set *p_set);
    point_set(const char* input);
    point_set(int d_num, int dt_cat, std::vector<point_t*> &pset, double u_range);
    ~point_set();

    void print();//print all the points in the set
    void random(double RandRate);//reload the points Randomly
    void delete_same();//delete the same points, keep one left
    point_set* skyline();//compute the skyline set
    point_set* sort(point_t *u);//sort points based on their utility w.r.t. linear function u
    void prune(point_t *p);
};


#endif //U_2_POINT_SET_H
