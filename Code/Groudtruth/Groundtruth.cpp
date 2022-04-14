#include "Groundtruth.h"

/**
 * @brief Find the largest tuple
 * @param p_skyline The tuple dataset
 * @param up        The utility vector
 * @return          The utility
 */
double ground_truth(point_set *p_skyline, point_t *up)
{
    int maxIdx = 0, size = p_skyline->points.size();
    double maxValue = 0;
    for (int i = 0; i < size; i++)
    {
        double value = p_skyline->points[i]->dot_product(up);//utility of the points
        if (value > maxValue)
        {
            maxValue = value;
            maxIdx = i;
        }
    }
    printf("-----------------------------------------------------------------\n");
    printf("|%15s |%15s |%15s |%10d |\n", "Ground Truth", "-", "-", p_skyline->points[maxIdx]->id);
    printf("-----------------------------------------------------------------\n");

    return maxValue;



}
