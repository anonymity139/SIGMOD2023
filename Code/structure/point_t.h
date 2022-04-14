#ifndef U_2_POINT_T_H
#define U_2_POINT_T_H
#include "define.h"

class point_t
{
public:
    int			d;
    COORD_TYPE*	attr;
    int			id;
    double      value;

    explicit point_t(int dim);
    point_t(int dim, int id);
    point_t(point_t *p);
    point_t(point_t *p1, point_t *p2);
    point_t(int dim, double range, int seed);
    ~point_t();
    void print(); //print the point
    bool is_same(point_t *p);//check whether the point is the same as p
    bool is_zero();
    bool sum_eq(double u_range);
    double dot_product(point_t* p);//calculate the dot product between two points
    double dot_product(double *v);//calculate the dot product between two points
    point_t* sub(point_t* p);//calculate the difference between two points
    point_t* add(point_t* p);//calculate the sum between two points
    point_t* scale(double c);//scale down the points
    void normalization();
    double cosine0(point_t *p);
    double cosine0(double *v);
    double orthogonality(point_t *p);
    double orthogonality(double *v);
    double calc_l1_dist(point_t* p);//calculate the l1 distance between two points
    double calc_len();//calculate the distance between the point to the origin
    int dominates_without_same(point_t* p);//check whether dominate p(consider this=p as this dominate p)
    int dominates(point_t* p);//check whether dominate p(consider this=p as this does not dominate p2)
    point_t* extract_num(int d_num, int d_new_attr);//extract the numerical part of the utility vector
    void printResult(std::ofstream &out_cp, char *name, int Qcount, timeval t1, int mode);

};










#endif //U_2_POINT_T_H
