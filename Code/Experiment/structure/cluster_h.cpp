#include "cluster_h.h"

cluster_h::cluster_h()
{
    center = NULL;
}

cluster_h::~cluster_h()
{

    int i = h_set.size();
    while(i>0)
    {
        delete[] h_set[i-1];
        i--;
    }
    h_set.clear();
}
