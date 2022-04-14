#include "structure/relational_graph.h"
#include "Tree.h"
#include "L.h"


void special(tuple_set *t_set, double *u, std::map<std::string, double> &categorical_value, int &Qcount)
{
    timeval t1, t2;
    ofstream out_cp("../result.txt");
    gettimeofday(&t1, 0);

    if (t_set->tuples.size() <= 0)
        return;
    double totalSize = t_set->tuples.size(), t_count = t_set->tuples.size();
    int dimCat = t_set->tuples[0]->d_cat; Qcount = 0;
    ctree *ctr = new ctree(dimCat);
    for (int i = 0; i < totalSize; ++i)
        ctr->add(t_set->tuples[i]);
    tuple_t *tp1, *tp2;

    for (int i = dimCat; i > 0; --i)//each categorical attribute corresponds to a round
    {
        //build a index for the i-th level
        RI R; int length = dimCat - i + 1;
        for (int j = 0; j < ctr->Tlist[i].size(); ++j)
        {
            bool is_founded = false;
            for(int k = 0; k < R.list.size(); ++k)
            {
                if(R.list[k]->is_same(ctr->Tlist[i][j], length))
                {
                    is_founded = true;
                    R.list[k]->nd.emplace_back(ctr->Tlist[i][j]);
                }
            }
            if(!is_founded)
            {
                L *l = new L(ctr->Tlist[i][j], length, R.list.size());
                R.list.emplace_back(l);
            }
        }

        valueArray vA(R.list.size());
        std::vector<L*> LOrderAll;
        for(int j = 0; j < R.list.size(); ++j)
        {
            for(int k = 0; k < LOrderAll.size(); ++k)
            {
                if(R.list[j]->find_tuple(LOrderAll[k], tp1, tp2))
                {
                    ++Qcount;
                    double score1 = tp1->score(u, categorical_value);
                    double score2 = tp2->score(u, categorical_value);
                    string *s1 = new string[length], *s2 = new string[length];
                    for(int l = 0; l < length; ++l)
                    {
                        if(R.list[j]->catevalue[l] != LOrderAll[k]->catevalue[l])
                        {
                            s1[l] = R.list[j]->catevalue[l];
                            s2[l] = LOrderAll[k]->catevalue[l];
                        }
                        else
                        {
                            s1[l] = "\0";
                            s2[l] = "\0";
                        }
                    }
                    if(score1 > score2)
                        R.update(vA, s1, s2);
                    else
                        R.update(vA, s2, s1);
                    //vA.print();
                    R.pruneTuple(vA, ctr, i);
                    t_count = ctr->count_tuple(i - 1);

                    //Delete unnecessary L
                    bool *isDelete = new bool[LOrderAll.size()];
                    for (int m = 0; m < LOrderAll.size(); ++m)
                    {
                        if (ctr->updateString(i - 1, LOrderAll[m]->catevalue))
                            isDelete[m] = false;
                        else
                            isDelete[m] = true;
                    }
                    for (int m = LOrderAll.size() - 1; m >= 0; --m)
                    {
                        if (isDelete[m])
                            LOrderAll.erase(LOrderAll.begin() + m);
                    }

                    /*
                    gettimeofday(&t2, 0);
                    double time_cost = (double) t2.tv_sec + (double) t2.tv_usec / 1000000 - (double) t1.tv_sec - (double) t1.tv_usec / 1000000;
                    std::cout << Qcount << "    " << t_count/totalSize << "    " << time_cost << "\n";
                    */

                }
            }
            //Delete unnecessary L
            if (ctr->updateString(i - 1, R.list[j]->catevalue))
                LOrderAll.push_back(R.list[j]);
        }
    }

    /*
    tp1 = ctr->Tlist[1][0]->goforTuple();
    for(int i = 1; i < ctr->Tlist[1].size(); ++i)
    {
        tp2 = ctr->Tlist[1][i]->goforTuple();
        ++Qcount;
        double score1 = tp1->score(u, categorical_value);
        double score2 = tp2->score(u, categorical_value);
        if(score1 < score2)
            tp1 = tp2;

        --t_count;
        gettimeofday(&t2, 0);
        double time_cost = (double) t2.tv_sec + (double) t2.tv_usec / 1000000 - (double) t1.tv_sec - (double) t1.tv_usec / 1000000;
        std::cout << Qcount << "    " << t_count/totalSize << "    " << time_cost << "\n";
        out_cp << Qcount << "    " << t_count/totalSize << "    " << time_cost << "\n";
    }
    */

    gettimeofday(&t2, 0);
    double time_cost = (double) t2.tv_sec + (double) t2.tv_usec / 1000000 - (double) t1.tv_sec - (double) t1.tv_usec / 1000000;
    printf("-----------------------------------------------------------------\n");
    printf("|%15s |%15d |%15lf |%10d |\n", "Special", Qcount, time_cost, ctr->root->goforTuple()->id);
    printf("-----------------------------------------------------------------\n");
    out_cp.close();


}


