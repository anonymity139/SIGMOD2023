#include <iostream>
#include "tuple_t.h"

/**
 * @brief Construct
 * @param dim_cat   Number of categorical attributes
 * @param dim_num   Number of numerical attributes
 */
tuple_t::tuple_t(int dim_cat, int dim_num)
{
    d_cat = dim_cat;
    d_num = dim_num;
    attr_cat = new std::string[dim_cat];
    attr_num = new double[dim_num];
    id = -1;
}

/**
 * @brief Construct
 * @param dim_cat   Number of categorical attributes
 * @param dim_num   Number of numerical attributes
 * @param id        ID
 */
tuple_t::tuple_t(int dim_cat, int dim_num, int id)
{
    d_cat = dim_cat;
    d_num = dim_num;
    attr_cat = new std::string[dim_cat];
    attr_num = new double[dim_num];
    this->id = id;
}

/**
 * @brief Construct
 */
tuple_t::tuple_t(){}

/**
 * @brief       Constructor
 * @param t     The tuple
 */
tuple_t::tuple_t(tuple_t *t)
{
    id = t->id;
    d_cat = t->d_cat;
    d_num = t->d_num;
    attr_num = new double[d_num];
    for(int i = 0; i < d_num; i++)
    {
        attr_num[i] = t->attr_num[i];
    }
    attr_cat = new std::string[d_cat];
    for(int i = 0; i < d_cat; i++)
    {
        attr_cat[i] = t->attr_cat[i];
    }
    p = t->p;
}

/**
 * @brief Destructor
 */
tuple_t::~tuple_t()
{
    delete[] attr_cat;
    delete[] attr_num;
}

/**
 * @param print the information of the tuple
 */
void tuple_t::print()
{
    std::cout<< id << "  ";
    for (int j = 0; j < d_cat; j++)//categorical attributes
    {
        std::cout<< attr_cat[j]<<"  ";
    }
    for (int j = 0; j < d_num; j++)//numerical attributes
    {
        std::cout<< attr_num[j]<<"  ";
    }
    std::cout<<std::endl;
}

/**
 * @brief       Check whether two tuples are the same
 * @param t     The tuple
 * @return      1 same
 *              0 different
 */
bool tuple_t::is_same(tuple_t *t)
{
    for(int i = 0; i < d_num; i++)
    {
        if(attr_num[i] != t->attr_num[i])
        {
            return false;
        }
    }
    for(int i = 0; i < d_cat; i++)
    {
        if(attr_cat[i] != t->attr_cat[i])
        {
            return false;
        }
    }
    return true;
}

/**
 * @brief       Check whether two tuples' categorical values are the same
 *              Ignore numerical attributes
 * @param t     The tuple
 * @return      1 same
 *              0 different
 */
bool tuple_t::is_same_cat(tuple_t *t)
{
    for(int i = 0; i < d_cat; i++)
    {
        if(attr_cat[i] != t->attr_cat[i])
        {
            return false;
        }
    }
    return true;
}

/**
 * @brief         Assume the categorical value are the same (use with function is_same_cat(tuple_t *t))
 *                Check whether dominate p(consider numerical part)
 *                Consider this=p as this does not dominate p2)
 * @param p       The tuple
 * @return        1 this dominate p
 *                0 this does not dominate p
 */
bool tuple_t::dominates_num(tuple_t *p)
{
    int count = 0; //number of attributes with the same value
    for (int i = 0; i < d_num; i++)
    {
        if (attr_num[i] < p->attr_num[i])
        {
            return false;
        }
        if (attr_num[i] == p->attr_num[i])
        {
            count++;
        }
    }
    return d_num >= count;
}

/**
 * @brief         Assume the categorical value are the same
 *                Check whether R_dominate p based on the utility range(consider numerical part)
 *                Consider this=p as this does not dominate p2)
 * @param p       The tuple
 * @param R       The utility range
 * @return        1 this dominate p
 *                0 this does not dominate p
 */
bool tuple_t::R_dominates_num(tuple_t *p, std::vector<point_t*> &R)
{
    int count = 0, size = R.size(); //number of attributes with the same value
    double *v = new double[d_num]; //set difference as a vector
    double dot = 0;
    for(int i = 0; i < d_num; i++)
    {
        v[i] = attr_num[i] - p->attr_num[i];
    }
    for (int i = 0; i < size; i++)
    {
        dot = R[i]->dot_product(v);
        if (dot < -EQN2)
            return false;
        //if (dot < EQN2 && dot > -EQN2)
        //{
        //    count++;
        //}
    }
    return true;
}

/**
 * @brief       Calculate the utility of the numerical part
 * @param v     The utility function
 * @return      The utility
 */
double tuple_t::dot_prod_num(double *v)
{
    double result = 0;
    for(int i = 0; i < d_num; i++)
    {
        result += attr_num[i] * v[i];
    }
    return result;
}


/**
 * @brief       Calculate the utility of the numerical part
 * @param p     The point
 * @return      The utility
 */
double tuple_t::dot_prod_num(point_t *p)
{
    double result = 0;
    for(int i = 0; i < d_num; i++)
    {
        result += attr_num[i] * p->attr[i];
    }
    return result;
}


/**
 * @brief       Calculate the score of the tuple
 * @param v     The utility fucntion of the numerical part
 * @param categorical_value The score of the categorical value
 * @return      The utlity
 */
double tuple_t::score(double *v, std::map<std::string, double> &categorical_value)
{
    double score = dot_prod_num(v);
    for(int i = 0; i < d_cat; ++i)
        score += categorical_value[attr_cat[i]];
    return score;
}

/**
 * @brief Calculate the regret ratio if the tuple is the represented tuple
 * @param v                     The numerical utility vector
 * @param categorical_value     The score of the categorical value
 * @param maxvalue              The maximum utility
 * @return
 */
double tuple_t::regret(double *v, std::map<std::string, double> &categorical_value, double maxvalue)
{
    double val = score(v, categorical_value);
    return 1 - val / maxvalue;
}


/**
 * @brief   The user evaluation step
 * @return  The boredness given by user
 */
void tuple_t::final_result(std::string s, int numOfQuestion, double time, std::ofstream &fp)
{
    std::cout << "-----------------------------------------------------------------------\n";
    std::cout << std::setw(10) << "Result" << std::setw(10) << "Fuel" << std::setw(12) << "Price($)" << std::setw(12) << "Year" << std::setw(12) << "PowerPS" << std::setw(12) << "Used KM\n";
    std::cout << "-----------------------------------------------------------------------\n";
    std::cout << std::setw(10) << "Car" << std::setw(10) << attr_cat[0] << std::setw(12) << std::setw(12) << attr_num[0] << std::setw(12) << attr_num[1] << std::setw(12) << attr_num[2] << std::setw(12) << attr_num[3] <<"\n";
    std::cout << "-----------------------------------------------------------------------\n";
    printf("Please give a number from 1 to 10 (i.e., 1, 2, .., 10) to indicate how \n"
           "bored you feel when you are asked with %d questions in this round in \n"
           "order to obtain your favorite car (Note: 10 denotes that you feel the \n"
           "most bored and 1 denotes that you feel the least bored.): ", numOfQuestion);
    int bor = 0;
    while (bor > 10 || bor < 1)
        std::cin >> bor;
    fp << "Algorithm: " << s << "   Question: " << numOfQuestion << "\n";
    fp<< "----------------------------------------------------------------------------\n";
    fp << std::setw(10) << "Result" << std::setw(10) << "Fuel" << std::setw(12) << "Price($)" << std::setw(12) << "Year" << std::setw(12) << "PowerPS" << std::setw(12) << "Used KM\n";
    fp<< "----------------------------------------------------------------------------\n";
    fp << std::setw(10) << "Car" << std::setw(10) << attr_cat[0] << std::setw(12) << attr_num[0] << std::setw(12) << attr_num[1] << std::setw(12) << attr_num[2] << std::setw(12) << attr_num[3] <<"\n";
    fp<< "----------------------------------------------------------------------------\n";
    fp << "Boredness: " << bor << "\n\n\n";
}







