#include <iomanip>
#include "hyperplane.h"

/**
 * @brief Construct
 */
hyperplane::hyperplane()
{
    p_1 = NULL;
    p_2 = NULL;
    t_1 = NULL;
    t_2 = NULL;
}

/**
 * @brief       Construct
 * @param dim   The dimension of the hyperplane
 */
hyperplane::hyperplane(int dim)
{
    d = dim;
    norm = new double[dim];
    p_1 = NULL;
    p_2 = NULL;
    t_1 = NULL;
    t_2 = NULL;
}


/**
 * @brief Constructor
 * @param h The old hyperplane
 */
hyperplane::hyperplane(hyperplane *h)
{
    d = h->d;
    norm = new double[d];
    for (int j = 0; j < d; j++)
        norm[j] = h->norm[j];
    offset = h->offset;
    p_1 = h->p_1;
    p_2 = h->p_2;
    t_1 = h->t_1;
    t_2 = h->t_2;
}

/**
 * @brief Construct
 *        Guarantee that n has at least one element
 * @param n         Value of the norm vector
 * @param offset    Offset
 */
hyperplane::hyperplane(int dim, double *n, double offset)
{
    d = dim;
    norm = n;
    this->offset = offset;
    p_1 = NULL;
    p_2 = NULL;
    t_1 = NULL;
    t_2 = NULL;
}

/**
 * @brief Construct
 * @param p1        The first point consisting of the hyperplane
 * @param p2        The second point consisting of the hyperplane
 * @param offset    Offset
 *
 */
hyperplane::hyperplane(point_t *p1, point_t *p2, double offset)
{
    d = p1->d;
    norm = new double[d];
    for (int i = 0; i < d; i++)
    {
        norm[i] = p1->attr[i] - p2->attr[i];
        //std::cout<<norm[i]<<" ";
    }
    this->offset = offset;
    p_1 = p1;
    p_2 = p2;
    t_1 = NULL;
    t_2 = NULL;
}

/**
 * @brief Construct (consider the numerical attributes of tuples)
 *        Assume the the categorical values are the same
 * @param t1        The first tuple consisting of the hyperplane
 * @param t2        The second tuple consisting of the hyperplane
 * @param offset    Offset
 */
hyperplane::hyperplane(tuple_t *t1, tuple_t *t2, double offset)
{
    d = t1->d_num;
    norm = new double[d];
    for (int i = 0; i < d; i++)
    {
        norm[i] = t1->attr_num[i] - t2->attr_num[i];
        //std::cout<<norm[i]<<" ";
    }
    this->offset = offset;
    p_1 = NULL;
    p_2 = NULL;
    t_1 = t1;
    t_2 = t2;
}

/**
 * @brief Construct
 * @param p1        The first point consisting of the hyperplane
 * @param p2        The second point consisting of the hyperplane
 * @param offset    Offset
 *
 */
hyperplane::hyperplane(int d1, double *p1, double *p2, double offset)
{
    d = d1;
    norm = new double[d];
    for (int i = 0; i < d; ++i)
    {
        norm[i] = p1[i] - p2[i];
        //std::cout<<norm[i]<<" ";
    }
    this->offset = offset;
    p_1 = NULL;
    p_2 = NULL;
    t_1 = NULL;
    t_2 = NULL;
}

/**
 * @brief Destructor
 *        Delete the array of norm
 */
hyperplane::~hyperplane()
{
    delete []norm;
}

/**
 * @brief       Check whether the hyperplane is the same as h
 * @param p     Point
 * @return      1 same
 *              0 different
 */
bool hyperplane::is_same(hyperplane *h)
{
    if(d!=h->d)
        return false;
    for (int i = 0; i < d; i++)
    {
        if (norm[i] != h->norm[i])
        {
            return false;
        }
    }
    return true;
}

/**
 * @brief  Print the information of the hyperplane
 */
void hyperplane::print()
{
    for (int i = 0; i < d; i++)
    {
        std::cout<<std::setprecision(10)<<norm[i]<<" ";
    }
    std::cout<<offset<<"\n";

}

/**
 * @brief       Compute the distance between the origin and
 *              the intersection of a hyperplane and a ray shooting
 * @param p     The ray shooting(vector)
 * @return      The distance
 */
double hyperplane::intersection_len(point_t *p)
{
    return p->calc_len() * (offset / (p->dot_product(norm)));
}


/**
 * @brief Used to normalize the hyperplane so that ||H||=1
 */
void hyperplane::normalization()
{
    double sum = 0;
    for (int i = 0; i < d; ++i)
        sum += norm[i] * norm[i];
    sum = sqrt(sum);
    for (int i = 0; i < d; ++i)
        norm[i] /= sum;
}










