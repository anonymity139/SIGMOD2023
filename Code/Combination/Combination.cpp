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
void combination(tuple_set *t_set, double *u, std::map<std::string, double> &categorical_value, double u_range,
                       int &Qcount, int ROUND, int mode)
{
    timeval t1; gettimeofday(&t1, 0);
    ofstream out_cp("../result.txt");
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
    //rnode_tree *tree = new rnode_tree(RG->list);
    //RG->set_relation_fast(tree);

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

        //update information
        if(!tp1->is_same_cat(tp2))
        {
            if(score1 >= score2)//update the relational graph
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
            if(score1 >= score2)
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
        c->prune(R);
        for(int i = 0; i < RG->list.size(); ++i)
        {
            //RG->list[i]->tuple_prune_all_rnode(R);
            RG->list[i]->tuple_prune_all_rnode_aggre(R);
            RG->list[i]->clean();
        }
        t_count = c->count_tuple();//count the tuples
        printMidResult(out_cp, t_count/totalSize * 100, Qcount, t1, mode);
    }
    c->clusters[0]->tuples[0]->p->printResult(out_cp, "Combination", Qcount, t1, mode);
    out_cp.close();
}


/**
 * @brief                       Algorithm Combination
 * @param t_set                 The tuple set
 * @param u                     The utility vector
 * @param categorical_value     The numerical value of each categorical value
 * @param Qcount                Number of questions
 */
void combination_confidence(tuple_set *t_set, double *u, std::map<std::string, double> &categorical_value, double u_range,
                       int &Qcount, int ROUND, double diffRatio, int mode)
{
    timeval t1; gettimeofday(&t1, 0);
    ofstream out_cp("../result.txt");
    double totalSize = t_set->tuples.size();
    if(t_set->tuples.size() <= 0)
        return;
    int d_num = t_set->tuples[0]->d_num, x = 0;
    double difficultQcount = 0; Qcount = 0;
    cluster_t *c = new cluster_t(t_set);
    hyperplane *h;
    hyperplane_set *R = new hyperplane_set(d_num, u_range);
    double t_count = INF;
    rnode_tree *tree = new rnode_tree();
    relational_graph *RG = new relational_graph(c, tree);
    //rnode_tree *tree = new rnode_tree(RG->list);
    //RG->set_relation_fast(tree);

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

        if(differenceRatio(score1, score2) < diffRatio)
            ++difficultQcount;
        if(differenceRatio(score1, score2) > diffRatio || rand() % 10 >= 5)//judge whether the user will answer the question
        {
            //update information
            if (!tp1->is_same_cat(tp2))
            {
                if (score1 >= score2)//update the relational graph
                {
                    RG->update_all_round(tp2, tp1, R, u_range, x++, tree, ROUND);
                    c->clusters[cid_2]->tuples.erase(c->clusters[cid_2]->tuples.begin() + tid_2);
                } else
                {
                    RG->update_all_round(tp1, tp2, R, u_range, x++, tree, ROUND);
                    c->clusters[cid_1]->tuples.erase(c->clusters[cid_1]->tuples.begin() + tid_1);
                }
                //RG->print_list();
            } else
            {
                if (score1 >= score2)
                {
                    R->hyperplanes.push_back(new hyperplane(tp2, tp1, 0));
                    c->clusters[cid_2]->tuples.erase(c->clusters[cid_2]->tuples.begin() + tid_2);
                } else
                {
                    R->hyperplanes.push_back(new hyperplane(tp1, tp2, 0));
                    c->clusters[cid_1]->tuples.erase(c->clusters[cid_1]->tuples.begin() + tid_1);
                }
                R->set_ext_pts(u_range);
            }
            if (d_num > 1)
                RG->update_basedR(R);

            //prune tuples
            c->prune(R);
            for (int i = 0; i < RG->list.size(); ++i)
            {
                //RG->list[i]->tuple_prune_all_rnode(R);
                RG->list[i]->tuple_prune_all_rnode_aggre(R);
                RG->list[i]->clean();
            }
        }
        t_count = c->count_tuple();//count the tuples
        printMidResult(out_cp, t_count/totalSize * 100, Qcount, t1, mode);
    }
    c->clusters[0]->tuples[0]->p->printResult(out_cp, "CombinationCf", Qcount, t1, mode);
    out_cp.close();
}