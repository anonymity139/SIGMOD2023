#ifndef U_2_TUPLE_T_H
#define U_2_TUPLE_T_H

#include <string>
#include <map>
#include "point_t.h"

class tuple_t
{
public:
    int id;
    int d_cat, d_num;
    double *attr_num;
    std::string *attr_cat;
    point_t* p;

    tuple_t();
    tuple_t(int dim_cat, int dim_num);
    tuple_t(int dim_cat, int dim_num, int id);
    tuple_t(tuple_t *t);
    ~tuple_t();
    void print();//print the tuple
    bool is_same(tuple_t *t);//check whether two tuples are the same
    bool is_same_cat(tuple_t *t);//check whether two tuples' categorical values are the same
    double dot_prod_num(double *v);//calculate the utility w.r.t the numerical attributes only
    double dot_prod_num(point_t *p);//calculate the utility w.r.t the numerical attributes only
    double score(double *v, std::map<std::string, double> &categorical_value);
    double regret(double *v, std::map<std::string, double> &categorical_value, double maxvalue);
    bool dominates_num(tuple_t* p);//check whether dominate p(consider numerical part)
    bool R_dominates_num(tuple_t* p, std::vector<point_t*> &R);//check whether dominate p(consider numerical part)
};


#endif //U_2_TUPLE_T_H
