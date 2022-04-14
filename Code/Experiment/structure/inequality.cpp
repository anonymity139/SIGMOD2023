#include "inequality.h"


/**
 * @brief   Constructor
 */
inequality::inequality(){}

/**
 * @brief   Constructor
 */
inequality::inequality(int ver, double *v, std::vector<double> &c)
{
    version = ver;
    norm = v;
    coeff = c;
}

/**
 * @brief   Deconstructor
 */
inequality::~inequality()
{
    //delete[] norm;
    //std::vector<double>().swap(coeff);
    //coeff.swap(std::vector<double>());
}

