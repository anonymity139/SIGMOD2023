#include "data_utility.h"


/*
 * @brief Generate a random point in dim-dimensional space
 * @param dim   The number of dimension
 * @return      The generated point
 */
point_t *rand_point(int dim)
{
    point_t *point_v = new point_t(dim);

    for (int i = 0; i < dim; i++)
        point_v->attr[i] = rand() * 1.0 / RAND_MAX;

    //print_point(point_v);

    return point_v;
}