#ifndef REGRET_INEQUALITY_H
#define REGRET_INEQUALITY_H

#include "define.h"

class inequality
{
public:
    int version;
    double *norm;
    std::vector<double> coeff;

    inequality();
    inequality(int ver, double *v, std::vector<double> &c);
    ~inequality();
};


#endif //REGRET_INEQUALITY_H
