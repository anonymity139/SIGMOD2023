#include "active_ranking.h"
#include <sys/time.h>

/**
 * @brief Ask user questions and obtain a ranking
 * @param pset 		The point set
 * @param u 		The utility vector
 * @param Qcount 	The number of question asked
 * @param max_value The largest utility w.r.t. u
 */
void Active_Ranking(point_set *pset, point_t *u, int &Qcount, int mode)
{
    timeval t1, t2;
    std::ofstream out_cp("../result.txt");
    gettimeofday(&t1, 0);

    int dim = pset->points[0]->d;
    Qcount = 0;
    pset->random(0.5);

    //initial
    hyperplane_set *R = new hyperplane_set(dim, U_RANGE);
    int M = pset->points.size();
    std::vector<point_t *> current_use;
    current_use.push_back(pset->points[0]); //store all the points in order

    //interaction
    for (int i = 1; i < M; ++i) //compare: p_set contains all the points
    {
        bool same_exist = false;
        for (int j = 0; j < current_use.size(); j++)
        {
            if (pset->points[i]->is_same(current_use[j])) //if there exist a same point in the current_use, insert p_set[i]
            {
                same_exist = true;
                current_use.insert(current_use.begin() + j, pset->points[i]);
                break;
            }
        }
        if (!same_exist)
        {
            int num = current_use.size(), place = 0; //the place of the point inserted into the current_use
            for (int j = 0; j < num; j++) //find the question asked user
            {
                hyperplane *h = new hyperplane(pset->points[i], current_use[j], 0);
                int relation = R->check_relation(h);
                if (relation == 0) //if intersect, calculate the distance
                {
                    Qcount++;
                    double v1 = pset->points[i]->dot_product(u);
                    double v2 = current_use[j]->dot_product(u);
                    if (v1 > v2)
                    {
                        R->hyperplanes.push_back(new hyperplane(current_use[j], pset->points[i], 0));
                        R->set_ext_pts(U_RANGE);
                    }
                    else
                    {
                        R->hyperplanes.push_back(new hyperplane(pset->points[i], current_use[j], 0));
                        R->set_ext_pts(U_RANGE);
                        place = j + 1;
                    }

                    printMidResult(out_cp, 100, Qcount, t1, mode);
                    if(Qcount >= 11)
                    {
                        current_use[0]->printResult(out_cp, "ActiveRanking", Qcount, t1, mode);
                        out_cp.close();
                        return;
                    }
                }
                else if (relation == -1)
                    place = j + 1;
            }
            current_use.insert(current_use.begin() + place, pset->points[i]);
        }
    }
    current_use[0]->printResult(out_cp, "ActiveRanking", Qcount, t1, mode);
    out_cp.close();
}
