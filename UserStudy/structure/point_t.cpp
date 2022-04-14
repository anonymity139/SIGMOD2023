#include "point_t.h"


/*
 * @brief Constructor
 * @param dim   Number of dimensions
 */
point_t::point_t(int dim)
{
    d = dim;
    attr = new double[dim];
    id = -1;
}

/*
 * @brief Constructor
 * @param dim   Number of dimensions
 * @param id    ID
 */
point_t::point_t(int dim, int id)
{
    d = dim;
    attr = new double[dim]();
    this->id = id;
}

/*
 * @brief Constructor
 * @param p   point
 */
point_t::point_t(point_t *p)
{
    int dim = p->d;
    d = dim;
    id = p->id;
    attr = new double[dim];
    for(int i = 0;i < dim; ++i)
        attr[i] = p->attr[i];
    value = p->value;
}

/*
 * @brief Constructor
 * @param p1   point
 */
point_t::point_t(point_t *p1, point_t *p2)
{
    d = p1->d;
    attr = new double[d];
    for(int i = 0; i < d; ++i)
        attr[i] = p1->attr[i] - p2->attr[i];
    id = -1;
}

/**
 * @brief Constructor
 *        require \sum p[i]=range
 * @param dim       The dimensionality
 * @param range     Parameter for point range
 */
point_t::point_t(int dim, double range, int seed)
{
    for(int i = 0; i < seed; i++)
        rand();
    d = dim;
    attr = new double[dim];
    double sum = 0;
    for (int i = 0; i < dim; i++)
    {
        attr[i] = ((double)rand()) / RAND_MAX;
        sum += attr[i];
    }
    attr[dim - 1] = range;
    for (int i = 0; i < dim - 1; i++)
    {
        attr[i] = (range * attr[i])/sum;
        attr[dim - 1] -= attr[i];
    }
}

/*
 * @brief Destructor
 *        delete the memory of the array
 */
point_t::~point_t()
{
    delete []attr;
}

/*
 * @brief For debug purpose, print the coordinates for a given point
 */
void point_t::print()
{
    //std::cout<< id << " ";
    for (int i = 0; i < d; i++)
        std::cout<< attr[i] << " ";
    std::cout << "\n";
}

/**
 * @brief       Check whether the point is the same as p
 * @param p     Point
 * @return      1 same
 *              0 different
 */
bool point_t::is_same(point_t *p)
{
    if(d != p->d)
        return false;
    for (int i = 0; i < d; ++i)
    {
        if (attr[i] - p->attr[i] < -EQN3 || attr[i] - p->attr[i] > EQN3)
            return false;
    }
    return true;
}

/**
 * @brief   Check whether all the attribute values are 0
 * @return  -true   all attributes values are 0
 *          -false  there exists attribute value which is not 0
 */
bool point_t::is_zero()
{
    for(int i = 0; i < d; ++i)
    {
        if(attr[i] < -EQN4 || attr[i] > EQN4)
            return false;
    }
    return true;
}

/**
 * @brief	    Calculate the dot product between two points
 * @param p     One point
 */
double point_t::dot_product(point_t *p)
{
    double result = 0;
    for(int i = 0; i < d; i++)
    {
        result += attr[i] * p->attr[i];
    }
    return result;
}

/**
 * @brief	    Calculate the dot product between two points
 * @param v     One array
 */
double point_t::dot_product(double *v)
{
    double result = 0;
    for(int i = 0; i < d; i++)
    {
        result += attr[i] * v[i];
    }
    return result;
}

/**
 * @brief check whether this dominate p(consider this=p as this dominate p)
 * @param p     Point
 * @return      1 this dominate p
 *              0 this does not dominate p
 */
int point_t::dominates_without_same(point_t *p)
{
    for (int i = 0; i < d; i++)
        if (attr[i] < p->attr[i])
            return 0;
    return 1;
}

/**
 * @brief check whether this dominate p(consider this=p as this does not dominate p)
 * @param p     Point
 * @return      1 this dominate p
 *              0 this does not dominate p
 */
int point_t::dominates(point_t *p)
{
    int d_num = 0; //number of attributes with the same value
    for (int i = 0; i < d; i++)
    {
        if (attr[i] < p->attr[i])
        {
            return 0;
        }
        if (attr[i] == p->attr[i])
        {
            d_num++;
        }
    }
    if(d_num == d)
    {
        return 1;
    }
    return 1;
}

/**
 * @brief	Calculate the subtraction between two points.
 *          Remember to release the returned point to save memory.
 * @param p The subtractor
 * @return  The subtraction(new points)
 */
point_t *point_t::sub(point_t *p)
{
    point_t* result = new point_t(d);
    for(int i = 0; i < d; i++)
    {
        result->attr[i] = attr[i] - p->attr[i];
    }
    return result;
}

/**
 * @brief	Calculate the addition between two points.
 *          Remember to release the returned point to save memory.
 * @param p The point
 * @return  The addition(new points)
 */
point_t *point_t::add(point_t *p)
{
    point_t* result = new point_t(d);
    for(int i = 0; i < d; i++)
    {
        result->attr[i] = attr[i] + p->attr[i];
    }
    return result;
}

/**
 * @brief	Scale the point
 *          Remember to release the returned point to save memory.
 * @param c The scaled coefficient
 * @return  The scaled point
 */
point_t *point_t::scale(double c)
{
    point_t* result = new point_t(d);
    for(int i = 0; i < d; i++)
    {
        result->attr[i] = attr[i] * c;
    }
    return result;
}

/**
 * @brief Normalize the point so that ||P|| = 1
 */
void point_t::normalization()
{
    double sum = 0;
    for (int i = 0; i < d; ++i)
        sum += attr[i] * attr[i];
    sum = sqrt(sum);
    for (int i = 0; i < d; ++i)
        attr[i] /= sum;
}

/**
 * @brief Used to calculate the orthogonality of two vectors. 1 - |cos()|
 * @param p The vector
 * @return  The orthogonality
 */
double point_t::orthogonality(point_t *p)
{
    double value = cosine0(p);
    if (value >= 0)
        return 1 - value;
    else
        return 1 + value;
}

/**
 * @brief Used to calculate the orthogonality of two vectors. 1 - |cos()|
 * @param v The vector
 * @return  The orthogonality
 */
double point_t::orthogonality(double *v)
{
    double value = cosine0(v);
    if (value >= 0)
        return 1 - value;
    else
        return 1 + value;
}


/**
 * @brief Used to calculate the cos() of the angle of two vectors
 * @param p The vector
 * @return  The cos() of the angle
 */
double point_t::cosine0(point_t *p)
{
    double sum = 0, s_h1 = 0, s_h2 = 0;
    for (int i = 0; i < d; ++i)
    {
        //std::cout<< attr[i] <<"  "<<p->attr[i] * U_RANGE <<"\n";
        sum += attr[i] * p->attr[i];
        s_h1 += attr[i] * attr[i];
        s_h2 += p->attr[i] * p->attr[i];
    }
    return sum / (sqrt(s_h1) * sqrt(s_h2));
}

/**
 * @brief Used to calculate the cos() of the angle of two vectors
 * @param v The vector
 * @return  The cos() of the angle
 */
double point_t::cosine0(double *v)
{
    double sum = 0, s_h1 = 0, s_h2 = 0;
    for (int i = 0; i < d; ++i)
    {
        sum += attr[i] * v[i];
        s_h1 += attr[i] * attr[i];
        s_h2 += v[i] * v[i];
    }
    return sum / (sqrt(s_h1) * sqrt(s_h2));
}

/**
 * @brief       Calculate the distance between two points
 * @param p     The points
 * @return      The distance
 */
double point_t::calc_l1_dist(point_t *p)
{
    double diff = 0;
    for(int i = 0; i < d; i++)
    {
        diff += (double) abs(attr[i] - p->attr[i]);
    }
    return diff;
}

/**
 * @brief   Calculate the distance between the point to the origin
 * @return  The distance
 */
double point_t::calc_len()
{
    double diff = 0;
    for(int i = 0; i < d; i++)
    {
        diff += attr[i] * attr[i];
    }
    return (double)sqrt(diff);
}

/**
 * @brief       Extract the numerical part of the utility vector
 * @return      The numerical utility vector
 */
point_t *point_t::extract_num(int d_num, int d_new_attr)
{
    point_t* u0 = new point_t(d_num);//for tuples of numerical attributes
    for (int i = 0; i < d_num; i++)
        u0->attr[i] = attr[d_new_attr + i];
    return u0;
}

/**
 * @brief   Check whether the sum of all attribute values is equal to u_range
 * @return  1   equal
 *          0   not equal
 */
bool point_t::sum_eq(double u_range)
{
    double s = 0;
    for(int i = 0; i < d; ++i)
        s += attr[i];
    return u_range - s > -EQN2 && u_range - s < EQN2;
}







