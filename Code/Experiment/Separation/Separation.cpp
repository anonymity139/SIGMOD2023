#include "Separation.h"
#include "structure/relational_graph.h"

/**
 * @brief                       Algorithm Separation
 * @param t_set                 The tuple set
 * @param u                     The utility vector
 * @param categorical_value     The numerical value of each categorical value
 * @param Qcount                Number of questions asked
 * @param epsilon               The threshold for regret ratio
 */
void separation(tuple_set *t_set, double *u, std::map<std::string, double> &categorical_value, double u_range,
                      int &Qcount, int ROUND, int mode)
{
    timeval t1, t2;
    ofstream out_cp("../result.txt");
    gettimeofday(&t1, 0);

    double totalSize = t_set->tuples.size();
    if(t_set->tuples.size() <= 0)
        return;
    int d_num = t_set->tuples[0]->d_num, x = 0; Qcount = 0;
    cluster_t *c = new cluster_t(t_set);
    hyperplane_set *R = new hyperplane_set(d_num, u_range);
    double tsize = c->count_cluster();
    rnode_tree *tree = new rnode_tree();
    relational_graph *RG = new relational_graph(c, tree);
    tuple_t *tp1, *tp2;


    while(tsize < c->clusters.size() && d_num > 1) //numerical attributes
    {
        ++Qcount; int cid_1, cid_2, tid_1, tid_2;
        c->select_tuple_sameset(tp1, tp2, cid_1, cid_2, tid_1, tid_2);

        //user feedback
        double score1 = tp1->score(u, categorical_value);
        double score2 = tp2->score(u, categorical_value);
        //tp1->print(); tp2->print(); //p1->print(); p2->print();
        //std::cout<<"score1: "<<score1<<"  score2: "<<score2<<"\n";

        //construct hyperplane based on the user's feedback
        if (score1 >= score2)
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
        c->prune(R);
        tsize = c->count_cluster(); //count tuple sets whose size are 1
        printMidResult(out_cp, c->count_tuple()/totalSize * 100, Qcount, t1, mode);
    }

    tsize = c->count_tuple();
    //c->maintain_one(R, d_num);
    while(tsize > 1) //categorical attributes
    {
        ++Qcount; int cid_1, cid_2, tid_1, tid_2;
        c->select_tuple(tp1, tp2, cid_1, cid_2, tid_1, tid_2); //randomly select two tuples

        //user feedback
        double score1 = tp1->score(u, categorical_value);
        double score2 = tp2->score(u, categorical_value);
        //tp1->print(); tp2->print(); //p1->print(); p2->print();
        //std::cout<<"score1: "<<score1<<"  score2: "<<score2<<"\n";

        //update information
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
        //prune tuples
        for(int i = 0; i < RG->list.size(); ++i)
        {
            //RG->list[i]->tuple_prune_all_rnode(R);
            RG->list[i]->tuple_prune_all_rnode_aggre(R);
            RG->list[i]->clean();
        }

        tsize = c->count_tuple(); //count the tuples
        printMidResult(out_cp, tsize/totalSize * 100, Qcount, t1, mode);
    }
    c->clusters[0]->tuples[0]->p->printResult(out_cp, "Separation", Qcount, t1, mode);
    out_cp.close();

}


/**
 * @brief                       Algorithm Separation
 * @param t_set                 The tuple set
 * @param u                     The utility vector
 * @param categorical_value     The numerical value of each categorical value
 * @param Qcount                Number of questions asked
 * @param epsilon               The threshold for regret ratio
 */
void separation_confidence(tuple_set *t_set, double *u, std::map<std::string, double> &categorical_value, double u_range,
                           int &Qcount, int ROUND, double diffRatio, int mode)
{
    timeval t1, t2;
    ofstream out_cp("../result.txt");
    gettimeofday(&t1, 0);

    double totalSize = t_set->tuples.size();
    if(t_set->tuples.size() <= 0)
        return;
    int d_num = t_set->tuples[0]->d_num, x = 0; double difficultQcount = 0; Qcount = 0;
    cluster_t *c = new cluster_t(t_set);
    hyperplane_set *R = new hyperplane_set(d_num, u_range);
    double tsize = c->count_cluster();
    rnode_tree *tree = new rnode_tree();
    relational_graph *RG = new relational_graph(c, tree);
    tuple_t *tp1, *tp2;


    while(tsize < c->clusters.size() && d_num > 1) //numerical attributes
    {
        ++Qcount; int cid_1, cid_2, tid_1, tid_2;
        c->select_tuple_sameset(tp1, tp2, cid_1, cid_2, tid_1, tid_2);

        //user feedback
        double score1 = tp1->score(u, categorical_value);
        double score2 = tp2->score(u, categorical_value);
        //tp1->print(); tp2->print(); //p1->print(); p2->print();
        //std::cout<<"score1: "<<score1<<"  score2: "<<score2<<"\n";

        if(differenceRatio(score1, score2) < diffRatio)
            ++difficultQcount;
        if(differenceRatio(score1, score2) > diffRatio || rand() % 10 >= 5)//judge whether the user will answer the question
        {
            //construct hyperplane based on the user's feedback
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
            c->prune(R);
        }
        tsize = c->count_cluster(); //count tuple sets whose size are 1
        printMidResult(out_cp, c->count_tuple()/totalSize * 100, Qcount, t1, mode);
    }

    tsize = c->count_tuple();
    //c->maintain_one(R, d_num);
    while(tsize > 1) //categorical attributes
    {
        ++Qcount; int cid_1, cid_2, tid_1, tid_2;
        c->select_tuple(tp1, tp2, cid_1, cid_2, tid_1, tid_2); //randomly select two tuples

        //user feedback
        double score1 = tp1->score(u, categorical_value);
        double score2 = tp2->score(u, categorical_value);
        //tp1->print(); tp2->print(); //p1->print(); p2->print();
        //std::cout<<"score1: "<<score1<<"  score2: "<<score2<<"\n";

        if(differenceRatio(score1, score2) < diffRatio)
            ++difficultQcount;
        if(differenceRatio(score1, score2) > diffRatio || rand() % 10 >= 5)//judge whether the user will answer the question
        {
            //update information
            if (score1 >= score2)//update the relational graph
            {
                RG->update_all_round(tp2, tp1, R, u_range, x++, tree, ROUND);
                c->clusters[cid_2]->tuples.erase(c->clusters[cid_2]->tuples.begin() + tid_2);
            } else
            {
                RG->update_all_round(tp1, tp2, R, u_range, x++, tree, ROUND);
                c->clusters[cid_1]->tuples.erase(c->clusters[cid_1]->tuples.begin() + tid_1);
            }
            //prune tuples
            for (int i = 0; i < RG->list.size(); ++i)
            {
                //RG->list[i]->tuple_prune_all_rnode(R);
                RG->list[i]->tuple_prune_all_rnode_aggre(R);
                RG->list[i]->clean();
            }
        }
        tsize = c->count_tuple(); //count the tuples
        printMidResult(out_cp, tsize/totalSize * 100, Qcount, t1, mode);
    }
    c->clusters[0]->tuples[0]->p->printResult(out_cp, "SeparationCf", Qcount, t1, mode);
    out_cp.close();
}