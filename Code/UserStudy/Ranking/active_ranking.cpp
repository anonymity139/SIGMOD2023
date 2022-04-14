#include "active_ranking.h"
#include <sys/time.h>

/**
 * @brief Ask user questions and obtain a ranking
 * @param pset 		The point set
 * @param u 		The utility vector
 * @param Qcount 	The number of question asked
 * @param max_value The largest utility w.r.t. u
 */
void Active_Ranking(tuple_set *car_set, point_set *pset, point_t *u, int &Qcount, int &TID, std::ofstream &fp)
{
    timeval t1, t2;
    gettimeofday(&t1, 0);

    double totalTime = 0, time_cost = 0;
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
                    std::cout << Qcount <<"\n";
                    //double v1 = pset->points[i]->dot_product(u);
                    //double v2 = current_use[j]->dot_product(u);

                    gettimeofday(&t2, 0);
                    time_cost = (double) t2.tv_sec + (double) t2.tv_usec / 1000000 - (double) t1.tv_sec - (double) t1.tv_usec / 1000000;
                    totalTime += time_cost;

                    int option = car_set->show_to_user(pset->points[i]->id, current_use[j]->id);
                    gettimeofday(&t1, 0);
                    if (option == 1) //v1 > v2)
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

                    if(Qcount >= 60)
                    {
                        gettimeofday(&t2, 0);
                        time_cost = (double) t2.tv_sec + (double) t2.tv_usec / 1000000 - (double) t1.tv_sec - (double) t1.tv_usec / 1000000;
                        totalTime += time_cost;
                        point_t* apu = new point_t(dim); R->average_point(apu);
                        int maxIdx = 0, size = pset->points.size();
                        double maxValue = 0;
                        for (int i = 0; i < size; i++)
                        {
                            double value = pset->points[i]->dot_product(apu); //utility of the points
                            if (value > maxValue)
                            {
                                maxValue = value;
                                maxIdx = i;
                            }
                        }
                        std::cout << "-----------------------------------------------------------------------\n";
                        printf("%28s: %5d \n", "Number of questions asked", Qcount);
                        std::cout << "-----------------------------------------------------------------------\n";
                        //out_cp << Qcount << "    " << time_cost << "\n";

                        car_set->tuples[pset->points[maxIdx]->id]->final_result("ActiveRanking", Qcount, totalTime, fp);
                        TID = pset->points[maxIdx]->id;
                        return;
                    }
                }
                else if (relation == -1)
                    place = j + 1;
            }
            current_use.insert(current_use.begin() + place, pset->points[i]);
        }
    }
    gettimeofday(&t2, 0);
    time_cost = (double) t2.tv_sec + (double) t2.tv_usec / 1000000 - (double) t1.tv_sec - (double) t1.tv_usec / 1000000;
    totalTime += time_cost;
    std::cout << "-----------------------------------------------------------------------\n";
    printf("%28s: %5d \n", "Number of questions asked", Qcount);
    std::cout << "-----------------------------------------------------------------------\n";
    //out_cp << Qcount << "    " << time_cost << "\n";
    car_set->tuples[current_use[0]->id]->final_result("ActiveRanking", Qcount, totalTime, fp);
    TID = current_use[0]->id;
}
