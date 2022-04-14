#include "Combination.h"
#include "structure/r_node.h"
#include "structure/relational_graph.h"
#include <sys/time.h>

/**
 * @brief                       Algorithm Combination
 * @param t_set                 The tuple set
 * @param u                     The utility vector
 * @param categorical_value     The numerical value of each categorical value
 * @param Qcount                Number of questions
 */
void combination_round(tuple_set *car_set, tuple_set *t_set, double *u, std::map<std::string, double> &categorical_value, double u_range, int &Qcount, int ROUND, int &TID, ofstream &fp)
{
    timeval t1, t2;
    gettimeofday(&t1, 0);

    double totalTime = 0, time_cost = 0;
    double totalSize = t_set->tuples.size();
    if(t_set->tuples.size() <= 0)
        return;
    int d_num = t_set->tuples[0]->d_num, x = 0, sid; Qcount = 0;
    cluster_t *c = new cluster_t(t_set);
    hyperplane *h;
    hyperplane_set *R = new hyperplane_set(d_num, u_range);
    double t_count = 10;
    rnode_tree *tree = new rnode_tree();
    relational_graph *RG = new relational_graph(c, tree);
    tuple_t* rep_tuple;

    while(t_count > 1)
    {
        ++Qcount;
        //randomly select two tuples
        tuple_t *tp1, *tp2; int cid_1, cid_2, tid_1, tid_2;
        int tsize = c->count_cluster();
        if(Qcount % 2 == 1 && tsize < c->clusters.size())
            c->select_tuple_sameset(tp1, tp2, cid_1, cid_2, tid_1, tid_2);
        else
            c->select_tuple(tp1, tp2, cid_1, cid_2, tid_1, tid_2);

        //user's feedback
        double score1 = tp1->score(u, categorical_value);
        double score2 = tp2->score(u, categorical_value);
        //tp1->print(); tp2->print();
        //std::cout<<"score1: "<<score1<<"  score2: "<<score2<<"\n";

        gettimeofday(&t2, 0);
        time_cost = (double) t2.tv_sec + (double) t2.tv_usec / 1000000 - (double) t1.tv_sec - (double) t1.tv_usec / 1000000;
        totalTime += time_cost;
        int option = car_set->show_to_user(tp1->id, tp2->id);
        gettimeofday(&t1, 0);

        //update information
        if(!tp1->is_same_cat(tp2))
        {
            if(option == 1) // score1 >= score2
            {
                RG->update_all_round(tp2, tp1, R, u_range, x++, tree, ROUND);
                c->clusters[cid_2]->tuples.erase(c->clusters[cid_2]->tuples.begin() + tid_2);
            }
            else
            {
                RG->update_all_round(tp1, tp2, R, u_range, x++, tree, ROUND);
                c->clusters[cid_1]->tuples.erase(c->clusters[cid_1]->tuples.begin() + tid_1);
            }
            //RG->print_list();
        }
        else
        {
            if(option == 1)//score1 >= score2
            {
                R->hyperplanes.push_back(new hyperplane(tp2, tp1, 0));
                c->clusters[cid_2]->tuples.erase(c->clusters[cid_2]->tuples.begin() + tid_2);
            }
            else
            {
                R->hyperplanes.push_back(new hyperplane(tp1, tp2, 0));
                c->clusters[cid_1]->tuples.erase(c->clusters[cid_1]->tuples.begin() + tid_1);
            }
            R->set_ext_pts(u_range);
        }
        if(d_num > 1)
            RG->update_basedR(R);

        //prune tuples
        //c->prune(R->ext_pts);
        c->prune(R);
        for(int i = 0; i < RG->list.size(); ++i)
        {
            //RG->list[i]->tuple_prune_all_rnode(R);
            RG->list[i]->tuple_prune_all_rnode_aggre(R);
            RG->list[i]->clean();
        }

        t_count = c->count_tuple();//count the tuples

    }
    gettimeofday(&t2, 0);
    time_cost = (double) t2.tv_sec + (double) t2.tv_usec / 1000000 - (double) t1.tv_sec - (double) t1.tv_usec / 1000000;
    std::cout << "-----------------------------------------------------------------------\n";
    printf("%28s: %5d \n", "Number of questions asked", Qcount);
    std::cout << "-----------------------------------------------------------------------\n";
    //out_cp << Qcount << "    " << time_cost << "\n";
    car_set->tuples[c->clusters[0]->tuples[0]->id]->final_result("Combination", Qcount, totalTime, fp);
    TID = c->clusters[0]->tuples[0]->id;
}
