//
// Created by 王伟程 on 2020/8/27.
//

#include "s_node.h"

/**
 * @brief Constructor
 * @param dim The number of dimension
 */
s_node::s_node(int dim)
{
    center = new point_t(dim);
}

/**
 * @brief Used to calculate the largest orthogonality of the spherical cap w.r.t a vector
 * @param n The vector
 * @return  The largest orthogonality
 */
double s_node::upper_orthog(point_t *n)
{
    double alpha0 = n->cosine0(center); //cos(a)
    alpha0 = acos(alpha0); //angle
    double theta0;
    if (angle >= 1)
        theta0 = 0;
    else
        theta0 = acos(angle);
    if ((alpha0 - theta0) < PI / 2 && (alpha0 + theta0) > PI / 2)
        return 1;
    else
    {
        double v1 = fabs(cos(alpha0 + theta0));
        double v2 = fabs(cos(alpha0 - theta0));
        if (v1 < v2)
            return 1 - v1;
        else
            return 1 - v2;
    }
}


/**
 * @brief Used to calculate the smallest orthogonality of the spherical cap w.r.t a vector
 * @param n          The vector
 * @return           The smallest orthogonality
 */
double s_node::lower_orthog(point_t *n)
{
    double alpha0 = n->cosine0(center);
    alpha0 = acos(alpha0); //angle
    double theta0;
    if (angle >= 1)
        theta0 = 0;
    else
        theta0 = acos(angle);
    if (alpha0 < theta0 || (alpha0 + theta0) > PI)
        return 0;
    else
    {
        double v1 = fabs(cos(alpha0 + theta0));
        double v2 = fabs(cos(alpha0 - theta0));
        if (v1 > v2)
            return 1 - v1;
        else
            return 1 - v2;
    }
}
