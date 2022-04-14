#include <list>
#include "relational_graph.h"

/**
 * @brief Constructor
 */
relational_graph::relational_graph(){};

/**
 * @brief Constructor
 *        Build the graph based on the tuple sets
 * @param clu   The tuple sets
 */
relational_graph::relational_graph(cluster_t *clu)
{
    int size = clu->clusters.size();
    for(int i = 0; i < size - 1; i++)
    {
        for(int j = i + 1; j < size; j++)
        {
            bool is_add = false;
            for(int k = 0; k < list.size(); k++)
            {
                if(list[k]->fit(clu->clusters[i], clu->clusters[j], k))
                {
                    is_add = true;
                    break;
                }
            }
            if(!is_add)
            {
                list.emplace_back(new r_node(clu->clusters[i], clu->clusters[j], list.size()));
            }
        }
    }
}


/**
 * @brief Constructor
 *        Build the graph based on the tuple sets
 * @param clu   The tuple sets
 * @param tree  The rnode_tree
 */
relational_graph::relational_graph(cluster_t *clu, rnode_tree *tree)
{
    //initialize
    tree->root = new rnode_node();
    rnode_node *cnode;
    int str_index, index;
    int size = clu->clusters.size(), len = clu->clusters[0]->tuples[0]->d_cat, direction;
    string *str = new string[2 * len];
    tuple_set *t_1, *t_2;

    for(int i = 0; i < size - 1; i++)
    {
        t_1 = clu->clusters[i];
        for(int j = i + 1; j < size; j++)
        {
            t_2 = clu->clusters[j];
            std::string *s1 = t_1->tuples[0]->attr_cat;
            std::string *s2 = t_2->tuples[0]->attr_cat;
            for(int l = 0; l < len; ++l)
            {
                if(s1[l] != s2[l])
                {
                    str[l] = s1[l]; str[len + l] = s2[l];
                }
                else
                {
                    str[l] = "\0"; str[len + l] = "\0";
                }
            }

            int k = tree->search(str, len, direction);
            k = tree->search(str, len, direction);
            if(k == -1)
            {
                list.emplace_back(new r_node(clu->clusters[i], clu->clusters[j], list.size()));
                cnode = tree->root->add(list[list.size() - 1], str_index, index);
                if(cnode != NULL)
                    cnode->build(list[list.size() - 1], str_index, index);
            }
            else
            {
                if (direction == 1)
                {
                    list[k]->sets.emplace_back(t_1, t_2);
                    t_1->bettupleset[t_2->ID].node_index = index;
                    t_1->bettupleset[t_2->ID].position = 1;
                    t_2->bettupleset[t_1->ID].node_index = index;
                    t_2->bettupleset[t_1->ID].position = 2;
                }
                else
                {
                    list[k]->sets.emplace_back(t_2, t_1);
                    t_1->bettupleset[t_2->ID].node_index = index;
                    t_1->bettupleset[t_2->ID].position = 2;
                    t_2->bettupleset[t_1->ID].node_index = index;
                    t_2->bettupleset[t_1->ID].position = 1;
                }
            }





        }
    }
}


/**
 * @brief Print all the nodes
 */
void relational_graph::print()
{
    int num = 0;
    for(auto & i : list)
    {
        std::cout<<"node: "<<num<<"\n";
        num++;
        std::cout<<"categorical value: ";
        for(int j = 0; j < i->len; j++)
        {
            std::cout<<i->s_1[j]<<" ";
        }
        std::cout<<" --vs-- ";
        for(int j = 0; j < i->len; j++)
        {
            std::cout<<i->s_2[j]<<" ";
        }
        std::cout<<"\n";
        std::cout<<"Inequalities(<):\n";
        for(int j = 0; j < i->ineqleq.size(); j++)
        {
            std::cout<<i->ineqleq[j].norm<<"\n";
        }
        std::cout<<"Inequalities(>):\n";
        for(int j = 0; j < i->ineqgeq.size(); j++)
        {
            std::cout<<i->ineqgeq[j].norm<<"\n";
        }
        std::cout<<"tuple set:\n";
        for(int j = 0; j < i->sets.size(); j++)
        {
            std::cout<<"pair "<< j + 1<<"\n";
            i->sets[j].first->print();
            i->sets[j].second->print();
        }
        std::cout<<"Relation:\n";
        for(int j = 0; j < i->relation.size(); j++)
        {
            i->relation[j].first->print();
        }
        std::cout<<"\n";
    }
}

/**
 * @brief Based on the node, find all the relation
 */
void relational_graph::set_relation()
{
    int size = list.size();
    for(int i = 0; i < size - 2; ++i)
    {
        cout<<i<<"\n";
        for(int j = i + 1; j < size - 1; ++j)
        {
            for(int k = j + 1; k < size; ++k)
            {
                //list[i]->print_cat();list[j]->print_cat();list[k]->print_cat();std::cout<<"\n";
                int turn = list[i]->is_sum(list[j], list[k]);
                if(turn != 0)//A + B = +/- 1/2 C
                {
                    rlist.emplace_back(new node_relation(list[j], list[k], list[i], turn, 1));
                    int rsize = rlist.size() - 1;
                    list[j]->relation.emplace_back(rlist[rsize], 1);
                    list[k]->relation.emplace_back(rlist[rsize], 2);
                    list[i]->relation.emplace_back(rlist[rsize], 3);
                }
                else
                {
                    turn = list[i]->is_difference(list[j], list[k]);
                    if(turn != 0)//A - B = (+/- 1/2) C
                    {
                        rlist.emplace_back(new node_relation(list[j], list[k], list[i], turn, -1));
                        int rsize = rlist.size() - 1;
                        list[j]->relation.emplace_back(rlist[rsize], 1);
                        list[k]->relation.emplace_back(rlist[rsize], 2);
                        list[i]->relation.emplace_back(rlist[rsize], 3);
                    }
                    else
                    {
                        turn = list[j]->is_sum(list[i], list[k]);
                        if(turn != 0)//C + B = +/- 1/2 A
                        {
                            rlist.emplace_back(new node_relation(list[i], list[k], list[j], turn, 1));
                            int rsize = rlist.size() - 1;
                            list[j]->relation.emplace_back(rlist[rsize], 3);
                            list[k]->relation.emplace_back(rlist[rsize], 2);
                            list[i]->relation.emplace_back(rlist[rsize], 1);
                        }
                        else //C + A = +/- 1/2 B
                        {
                            turn = list[j]->is_difference(list[i], list[k]);
                            if(turn != 0)//C - B = +/- 1/2 A
                            {
                                rlist.emplace_back(new node_relation(list[i], list[k], list[j], turn, -1));
                                int rsize = rlist.size() - 1;
                                list[j]->relation.emplace_back(rlist[rsize], 3);
                                list[k]->relation.emplace_back(rlist[rsize], 2);
                                list[i]->relation.emplace_back(rlist[rsize], 1);
                            }
                            else
                            {
                                turn = list[k]->is_sum(list[i], list[j]);
                                if(turn != 0)
                                {
                                    rlist.emplace_back(new node_relation(list[i], list[j], list[k], turn, 1));
                                    int rsize = rlist.size() - 1;
                                    list[i]->relation.emplace_back(rlist[rsize], 1);
                                    list[j]->relation.emplace_back(rlist[rsize], 2);
                                    list[k]->relation.emplace_back(rlist[rsize], 3);
                                }
                                else
                                {
                                    turn = list[k]->is_difference(list[i], list[j]);
                                    if(turn != 0)
                                    {
                                        rlist.emplace_back(new node_relation(list[i], list[j], list[k], turn, -1));
                                        int rsize = rlist.size() - 1;
                                        list[i]->relation.emplace_back(rlist[rsize], 1);
                                        list[j]->relation.emplace_back(rlist[rsize], 2);
                                        list[k]->relation.emplace_back(rlist[rsize], 3);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

/**
 * @brief Based on the node, find all the relation
 */
void relational_graph::set_relation_fast(rnode_tree *tree)
{
    int size = list.size(), len = list[0]->len;

    for(int i = 0; i < size - 1; ++i)
    {
        cout<<i<<"\n";
        for(int j = i + 1; j < size; ++j)
        {
            //list[i]->print_cat();list[j]->print_cat();
            string *str = new string[2 * len];
            int coeff = list[i]->add(list[j], str), direction;//A + B = +/- 1/2 C
            if(coeff != -1)
            {
                int k = tree->search(str, len, direction);
                if(k != -1)
                {
                    //list[k]->print_cat();
                    rlist.emplace_back(new node_relation(list[i], list[j], list[k], direction * coeff, 1));
                    int rsize = rlist.size() - 1;
                    list[i]->relation.emplace_back(rlist[rsize], 1);
                    list[j]->relation.emplace_back(rlist[rsize], 2);
                    list[k]->relation.emplace_back(rlist[rsize], 3);
                }
            }
            str = new string[2 * len]; direction = 0;
            coeff = list[i]->subtract(list[j], str);//A - B = (+/- 1/2) C
            if(coeff != -1)
            {
                int k = tree->search(str, len, direction);
                if(k != -1)
                {
                    //list[k]->print_cat();
                    rlist.emplace_back(new node_relation(list[i], list[j], list[k], direction * coeff, -1));
                    int rsize = rlist.size() - 1;
                    list[i]->relation.emplace_back(rlist[rsize], 1);
                    list[j]->relation.emplace_back(rlist[rsize], 2);
                    list[k]->relation.emplace_back(rlist[rsize], 3);
                }
            }
        }
    }
}

/**
 * @brief               Find the node containing the categorical values s1, s2 in the graph
 * @param s1            String
 * @param s2            String
 * @param direction     The direction of the matching
 * @return              The index of the node
 *                      -1 does not have that node
 */
int relational_graph::find_node(std::string *s1, std::string *s2, int &direction)
{
    int size = list.size();
    for(int i = 0; i < size; ++i)
    {
          direction = list[i]->fit(s1, s2);
          if(direction != 0)
              return i;
    }
    return -1;
}

/**
 * @brief       Based on the utility range R, simplify the inequalities of each node
 * @param R
 * @return
 */
void relational_graph::update_basedR(hyperplane_set* R)
{
    int size = list.size();
    for(int i = 0; i < size; ++i)
        list[i]->update(R);
}

/**
 * @brief       Based on the inequality , update the relational graph
 *              g(s1) - g(s2) <= uv
 * @param s1    categorical values
 * @param s2    categorical values
 * @param v     inequalities
 */
void relational_graph::update_all(tuple_t *tp1, tuple_t *tp2, hyperplane_set *R, double u_range, int x)
{
    for(int i = 0; i < list.size(); ++i)
        list[i]->update_coeff();

    //initialize
    std::string *s1 = tp1->attr_cat;
    std::string *s2 = tp2->attr_cat;
    int d_num = tp1->d_num, round = 2;
    std::list<std::pair<r_node*, int>> n_index; //int: which part updated (ineqleq, ineqgeq)

    //update the point containing the inequality
    int direction, index = this->find_node(s1, s2, direction);
    if(index <0)
    {
        cout<<"The node is not found.\n";
        return;
    }
    r_node* rn1 = list[index];
    //tp1->print(); tp2->print(); rn1->print();
    //direction: x-y or y-x; updated part: ineqleq or ineqgeq
    double *v = new double[d_num];
    if(direction == -1)
    {
        for (int i = 0; i < d_num; ++i)
            v[i] = tp2->attr_num[i] - tp1->attr_num[i];
    }
    else if(direction == 1)
    {
        for (int i = 0; i < d_num; ++i)
            v[i] = tp1->attr_num[i] - tp2->attr_num[i];
    }
    std::vector<double> coeff;
    for(int i = 0 ; i < x; ++i)
        coeff.push_back(0);
    coeff.push_back(1);

    rn1->print_cat();
    for (int i = 0; i < d_num; ++i)
        cout<<v[i]<<" ";
    cout<<"\n";

    //print_list();

    //R->print();
    int update_part = rn1->update_without_check(v, direction, R, coeff);

    //update the nodes which have relation with node[index]
    for(int i = 0; i < rn1->relation.size(); ++i)
    {
        //rn1->relation[i].first->print();
        rn1->update(rn1->relation[i], update_part, R, round);
        //insert into queue
        if (rn1->relation[i].second == 1)
        {
            if (rn1->relation[i].first->node_2->updated != 0)
                n_index.emplace_back(rn1->relation[i].first->node_2, rn1->relation[i].first->node_2->updated);
            if (rn1->relation[i].first->node_sum->updated != 0)
                n_index.emplace_back(rn1->relation[i].first->node_sum, rn1->relation[i].first->node_sum->updated);
        }
        else if (rn1->relation[i].second == 2)
        {
            if (rn1->relation[i].first->node_1->updated != 0)
                n_index.emplace_back(rn1->relation[i].first->node_1, rn1->relation[i].first->node_1->updated);
            if (rn1->relation[i].first->node_sum->updated != 0)
                n_index.emplace_back(rn1->relation[i].first->node_sum, rn1->relation[i].first->node_sum->updated);
        }
        else{
            if (rn1->relation[i].first->node_1->updated != 0)
                n_index.emplace_back(rn1->relation[i].first->node_1, rn1->relation[i].first->node_1->updated);
            if (rn1->relation[i].first->node_2->updated != 0)
                n_index.emplace_back(rn1->relation[i].first->node_2, rn1->relation[i].first->node_2->updated);
        }
        //print_list();
    }


    std::list<std::pair<r_node*, int>>::iterator iter;
    for(iter = n_index.begin(); iter != n_index.end(); ++iter)
        iter->first->is_modified = 1;

    round++;
    //updated the rest node
     while(!n_index.empty()&&round < 3)
    {
        r_node* nd = n_index.front().first;
        update_part = n_index.front().second;
        if(!nd->is_modified)
        {
            for(iter = n_index.begin(); iter != n_index.end(); ++iter)
                iter->first->is_modified = 1;
            round++;
        }
        for(int i = 0; i < nd->relation.size(); ++i)
        {
            nd->print_cat();
            nd->relation[i].first->print();
            nd->update(nd->relation[i], update_part, R, round);
            //insert into queue
            if (nd->relation[i].second == 1)
            {
                if (nd->relation[i].first->node_2->updated != 0)
                    n_index.emplace_back(nd->relation[i].first->node_2, nd->relation[i].first->node_2->updated);
                if (nd->relation[i].first->node_sum->updated != 0)
                    n_index.emplace_back(nd->relation[i].first->node_sum, nd->relation[i].first->node_sum->updated);
            }
            else if (nd->relation[i].second == 2)
            {
                if (nd->relation[i].first->node_1->updated != 0)
                    n_index.emplace_back(nd->relation[i].first->node_1, nd->relation[i].first->node_1->updated);
                if (nd->relation[i].first->node_sum->updated != 0)
                    n_index.emplace_back(nd->relation[i].first->node_sum, nd->relation[i].first->node_sum->updated);
            }
            else{
                if (nd->relation[i].first->node_1->updated != 0)
                    n_index.emplace_back(nd->relation[i].first->node_1, nd->relation[i].first->node_1->updated);
                if (nd->relation[i].first->node_2->updated != 0)
                    n_index.emplace_back(nd->relation[i].first->node_2, nd->relation[i].first->node_2->updated);
            }
            //print_list();
        }
        std::pair<r_node*, int> x = n_index.front();
        x.first->is_modified = 0;
        n_index.remove(x);
    }
}

/**
 * @brief       Based on the inequality , update the relational graph
 *              g(s1) - g(s2) <= uv
 * @param s1    categorical values
 * @param s2    categorical values
 * @param v     inequalities
 */
void relational_graph::update_all_round(tuple_t *tp1, tuple_t *tp2, hyperplane_set *R, double u_range, int x, rnode_tree *tree, int ROUND)
{
    for(int i = 0; i < list.size(); ++i)
        list[i]->update_coeff();

    //initialize
    std::string *s1 = tp1->attr_cat;
    std::string *s2 = tp2->attr_cat;
    int d_num = tp1->d_num, round = 2;

    //find the node needed to update
    int direction;
    r_node* nd = list[this->find_node(s1, s2, direction)];

    //direction: x-y or y-x;
    double *v = new double[d_num];
    if(direction == -1)
    {
        for (int i = 0; i < d_num; ++i)
            v[i] = tp2->attr_num[i] - tp1->attr_num[i];
    }
    else if(direction == 1)
    {
        for (int i = 0; i < d_num; ++i)
            v[i] = tp1->attr_num[i] - tp2->attr_num[i];
    }
    std::vector<double> coeff;
    for(int i = 0 ; i < x; ++i)
        coeff.push_back(0);
    coeff.push_back(1);

    nd->update_without_check_round(v, direction, R, coeff, u_range);//update the point containing the inequality

    auto *n_index = new std::list<std::pair<r_node*, int>>[ROUND + 1]; //int: which part updated (ineqleq, ineqgeq)
    n_index[1].emplace_back(nd, direction);

    for(int round = 2; round <= ROUND; ++round)
    {
        while(!n_index[round - 1].empty())
        {
            if(!nd->isRelationFound)
                findRelation(nd, tree);
            nd = n_index[round - 1].front().first;
            direction = n_index[round - 1].front().second;
            for (int i = 0; i < nd->relation.size(); ++i)
            {
                nd->update_round(nd->relation[i], direction, R, round, u_range);
                //insert into queue
                if (nd->relation[i].second == 1)
                {
                    if (nd->relation[i].first->node_2->updated != 0)
                        n_index[round].emplace_back(nd->relation[i].first->node_2, nd->relation[i].first->node_2->updated);
                    if (nd->relation[i].first->node_sum->updated != 0)
                        n_index[round].emplace_back(nd->relation[i].first->node_sum, nd->relation[i].first->node_sum->updated);
                }
                else if (nd->relation[i].second == 2)
                {
                    if (nd->relation[i].first->node_1->updated != 0)
                        n_index[round].emplace_back(nd->relation[i].first->node_1, nd->relation[i].first->node_1->updated);
                    if (nd->relation[i].first->node_sum->updated != 0)
                        n_index[round].emplace_back(nd->relation[i].first->node_sum, nd->relation[i].first->node_sum->updated);
                }
                else
                {
                    if (nd->relation[i].first->node_1->updated != 0)
                        n_index[round].emplace_back(nd->relation[i].first->node_1, nd->relation[i].first->node_1->updated);
                    if (nd->relation[i].first->node_2->updated != 0)
                        n_index[round].emplace_back(nd->relation[i].first->node_2, nd->relation[i].first->node_2->updated);
                }
            }
            std::pair<r_node *, int> x = n_index[round - 1].front();
            n_index[round - 1].remove(x);
        }
    }
}


/**
 * @brief       Based on the inequality , update the relational graph
 *              g(s1) - g(s2) <= uv
 * @param s1    categorical values
 * @param s2    categorical values
 * @param v     inequalities
 */
void relational_graph::update_all_threshold(tuple_t *tp1, tuple_t *tp2, hyperplane_set *R, double u_range, int x, rnode_tree *tree, double THRESHOLD)
{
    for(int i = 0; i < list.size(); ++i)
        list[i]->update_coeff();

    //initialize
    std::string *s1 = tp1->attr_cat;
    std::string *s2 = tp2->attr_cat;
    int d_num = tp1->d_num, round = 2;

    //find the node needed to update
    int direction;
    r_node* nd = list[this->find_node(s1, s2, direction)];

    //direction: x-y or y-x;
    double *v = new double[d_num];
    if(direction == -1)
    {
        for (int i = 0; i < d_num; ++i)
            v[i] = tp2->attr_num[i] - tp1->attr_num[i];
    }
    else if(direction == 1)
    {
        for (int i = 0; i < d_num; ++i)
            v[i] = tp1->attr_num[i] - tp2->attr_num[i];
    }
    std::vector<double> coeff;
    for(int i = 0 ; i < x; ++i)
        coeff.push_back(0);
    coeff.push_back(1);

    nd->update_without_check_threshold(v, direction, R, coeff, u_range, THRESHOLD);//update the point containing the inequality
    auto *n_index = new std::list<std::pair<r_node*, int>>[100]; //int: which part updated (ineqleq, ineqgeq)
    if (nd->updated != 0)
        n_index[1].emplace_back(nd, direction);

    for(int round = 2; round < 100; ++round)
    {
        while(!n_index[round - 1].empty())
        {
            if(!nd->isRelationFound)
                findRelation(nd, tree);
            nd = n_index[round - 1].front().first;
            direction = n_index[round - 1].front().second;
            for (int i = 0; i < nd->relation.size(); ++i)
            {
                nd->update_threshold(nd->relation[i], direction, R, round, u_range, THRESHOLD);
                //insert into queue
                if (nd->relation[i].second == 1)
                {
                    if (nd->relation[i].first->node_2->updated != 0)
                        n_index[round].emplace_back(nd->relation[i].first->node_2, nd->relation[i].first->node_2->updated);
                    if (nd->relation[i].first->node_sum->updated != 0)
                        n_index[round].emplace_back(nd->relation[i].first->node_sum, nd->relation[i].first->node_sum->updated);
                }
                else if (nd->relation[i].second == 2)
                {
                    if (nd->relation[i].first->node_1->updated != 0)
                        n_index[round].emplace_back(nd->relation[i].first->node_1, nd->relation[i].first->node_1->updated);
                    if (nd->relation[i].first->node_sum->updated != 0)
                        n_index[round].emplace_back(nd->relation[i].first->node_sum, nd->relation[i].first->node_sum->updated);
                }
                else
                {
                    if (nd->relation[i].first->node_1->updated != 0)
                        n_index[round].emplace_back(nd->relation[i].first->node_1, nd->relation[i].first->node_1->updated);
                    if (nd->relation[i].first->node_2->updated != 0)
                        n_index[round].emplace_back(nd->relation[i].first->node_2, nd->relation[i].first->node_2->updated);
                }
            }
            std::pair<r_node *, int> x = n_index[round - 1].front();
            n_index[round - 1].remove(x);
        }
        if(n_index[round].empty())
            break;
    }
}


/**
 * @brief   Prune the tuples in the tuple set
 * @param t_set     Tuple set
 * @param R         The utility range
 */
void relational_graph::prune(tuple_set *t_set, std::vector<point_t *> &R)
{
    int size = t_set->tuples.size();
    for(int i = 0; i < size; ++i)
    {
        if(t_set->tuples[i] != NULL)
        {
            for(int j = 0; j < size; ++j)
            {
                //check if tuples[i] R-dominates tuples[j]
                if(i!=j && t_set->tuples[j] != NULL)
                {
                    if(t_set->tuples[i]->is_same_cat(t_set->tuples[j]))
                    {
                        if(t_set->tuples[i]->R_dominates_num(t_set->tuples[j], R))
                            t_set->tuples[j] = NULL;
                    }
                    else
                    {
                        int index, direction, dim = t_set->tuples[i]->d_num;
                        index = find_node(t_set->tuples[i]->attr_cat, t_set->tuples[j]->attr_cat, direction);
                        if(direction == -1)
                        {
                            double *v = new double[dim];
                            for(int l = 0; l < dim; ++l)
                                v[l] = t_set->tuples[j]->attr_num[l] - t_set->tuples[i]->attr_num[l];
                            for(int k = 0; k < list[index]->ineqgeq.size(); ++k)
                            {
                                if(R_cover(list[index]->ineqgeq[k].norm, v, R))
                                {
                                    t_set->tuples[j] = NULL;
                                    break;
                                }
                            }
                        }
                        else
                        {
                            double *v = new double[dim];
                            for(int l = 0; l < dim; ++l)
                                v[l] = t_set->tuples[i]->attr_num[l] - t_set->tuples[j]->attr_num[l];
                            for(int k = 0; k < list[index]->ineqleq.size(); ++k)
                            {
                                if(R_cover(v, list[index]->ineqleq[k].norm, R))
                                {
                                    t_set->tuples[j] = NULL;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

/**
 * @brief   Print all the relations of nodes
 */
void relational_graph::print_rlist()
{
    for(int i = 0; i < rlist.size(); ++i)
        rlist[i]->print();
}

/**
 * @brief   Print all the nodes
 */
void relational_graph::print_list()
{
    cout<<"Relational Graph \n";
    for(int j=0; j < list.size(); ++j)
        list[j]->print();
    cout<<"\n\n";
}


void relational_graph::findRelation(r_node *nd, rnode_tree *tree)
{
    nd->isRelationFound = true;
    int size = list.size(), len = list[0]->len;
    for(int i = 0; i < size - 1; ++i)
    {
        //list[i]->print_cat();list[j]->print_cat();

        //A + B = +/- 1/2 C
        string *str = new string[2 * len];
        int coeff = nd->add(list[i], str), direction;//A + B = +/- 1/2 C
        if(coeff != -1)
        {
            int k = tree->search(str, len, direction);
            if(k != -1)
            {
                //list[k]->print_cat();
                rlist.emplace_back(new node_relation(nd, list[i], list[k], direction * coeff, 1));
                int rsize = rlist.size() - 1;
                nd->relation.emplace_back(rlist[rsize], 1);
                list[i]->relation.emplace_back(rlist[rsize], 2);
                list[k]->relation.emplace_back(rlist[rsize], 3);
            }
        }

        //A - B = +/- 1/2 C
        str = new string[2 * len]; direction = 0;
        coeff = nd->subtract(list[i], str);//A - B = (+/- 1/2) C
        if(coeff != -1)
        {
            int k = tree->search(str, len, direction);
            if(k != -1)
            {
                //list[k]->print_cat();
                rlist.emplace_back(new node_relation(nd, list[i], list[k], direction * coeff, -1));
                int rsize = rlist.size() - 1;
                nd->relation.emplace_back(rlist[rsize], 1);
                list[i]->relation.emplace_back(rlist[rsize], 2);
                list[k]->relation.emplace_back(rlist[rsize], 3);
            }
        }

        //2A + B = +/- C
        str = new string[2 * len]; direction = 0;
        coeff = nd->add2(list[i], str);//A - B = (+/- 1/2) C
        if(coeff != -1)
        {
            int k = tree->search(str, len, direction);
            if(k != -1)
            {
                //list[k]->print_cat();
                if(direction == 1)
                    rlist.emplace_back(new node_relation(list[k], list[i], nd, 2, -1));
                else
                    rlist.emplace_back(new node_relation(list[k], list[i], nd, -2, 1));
                int rsize = rlist.size() - 1;
                list[k]->relation.emplace_back(rlist[rsize], 1);
                list[i]->relation.emplace_back(rlist[rsize], 2);
                nd->relation.emplace_back(rlist[rsize], 3);
            }
        }

        //2A - B = +/- C
        str = new string[2 * len]; direction = 0;
        coeff = nd->subtract2(list[i], str);//A - B = (+/- 1/2) C
        if(coeff != -1)
        {
            int k = tree->search(str, len, direction);
            if(k != -1)
            {
                //list[k]->print_cat();
                if(direction == 1)
                    rlist.emplace_back(new node_relation(list[k], list[i], nd, 2, 1));
                else
                    rlist.emplace_back(new node_relation(list[k], list[i], nd, -2, -1));
                int rsize = rlist.size() - 1;
                list[k]->relation.emplace_back(rlist[rsize], 1);
                list[i]->relation.emplace_back(rlist[rsize], 2);
                nd->relation.emplace_back(rlist[rsize], 3);
            }
        }
    }
}


/**
 * @brief Calculate the regret ratio
 * @param c The clusters containing tuples with different categorical value
 * @return  The regret ratio
 */
double relational_graph::regret_ratio(cluster_t *c, hyperplane_set *R, tuple_t *&maxp)
{
    std::vector<int> list_sid;
    c->find_tupleset_relationexist(list_sid);

    int dim = c->clusters[0]->tuples[0]->d_num;
    double maxvalue;
    point_t* ap = new point_t(dim);
    R->average_point(ap);

    if(list_sid.size() < 1)//consider all the clusters
    {
        return 1;
    }
    else
    {
        double regret = 1, minregret = 1;
        for(int s = 0; s < list_sid.size(); ++s)
        {
            //find the tuple with the largest numerical utility
            int tid; maxvalue = -1;
            tuple_set* clu = c->clusters[ list_sid[s] ];
            for(int j = 0; j < clu->tuples.size(); ++j)
            {
                double value = clu->tuples[j]->dot_prod_num(ap);
                if (value > maxvalue)
                {
                    maxvalue = value;
                    tid = j;
                }
            }

            //calculate the regret ratio of t
            double value = clu->regret_sameset(R->ext_pts, tid); //consider the regret ratio in the cluster
            if(value < 1)                                           //consider the regret ratio with other the cluster
            {
                regret = value;
                for (int i = 0; i < c->clusters.size(); ++i)
                {
                    if(i != list_sid[s])
                    {
                        tuple_set *tset1 = clu, *tset2 = c->clusters[i];
                        value = list[tset1->bettupleset[tset2->ID].node_index]->regret_check(tset1->tuples[tid], tset2->tuples, tset1->bettupleset[tset2->ID].position, R->ext_pts);
                        if (value >= 1)
                        {
                            regret = 1;
                            break;
                        }
                        else if (value > regret)
                            regret = value;
                    }
                }
            }

            if(regret < minregret)
            {
                minregret = regret;
                maxp = clu->tuples[tid];
            }
        }
        return minregret;
    }
}


/**
 * @brief Calculate the regret ratio
 * @param c The clusters containing tuples with different categorical value
 * @return  The regret ratio
 */
double relational_graph::regret_ratio2(cluster_t *c, hyperplane_set *R, point_t *ap, tuple_t *&maxp)
{
    std::vector<int> list_sid;
    c->find_tupleset_relationexist(list_sid);

    int dim = c->clusters[0]->tuples[0]->d_num;
    double maxvalue;
    ap->print();

    if(list_sid.size() < 1)//consider all the clusters
    {
        int maxIdx = 0; double maxValue = -1;
        for (int i = 0; i < c->clusters.size(); i++)
        {
            for(int j = 0; j < c->clusters[i]->tuples.size(); ++j)
            {
                double value = c->clusters[i]->tuples[j]->p->dot_product(ap);//utility of the points
                if (value > maxValue)
                {
                    maxValue = value;
                    maxp = c->clusters[i]->tuples[j];
                }
            }
        }
        return 1;
    }
    else
    {
        double regret = 1, minregret = 1;
        for(int s = 0; s < list_sid.size(); ++s)
        {
            //find the tuple with the largest numerical utility
            int tid; maxvalue = -1;
            tuple_set* clu = c->clusters[ list_sid[s] ];
            for(int j = 0; j < clu->tuples.size(); ++j)
            {
                double value = clu->tuples[j]->dot_prod_num(ap);
                if (value > maxvalue)
                {
                    maxvalue = value;
                    tid = j;
                }
            }

            //calculate the regret ratio of t
            double value = clu->regret_sameset(R->ext_pts, tid); //consider the regret ratio in the cluster
            if(value < 1)                                           //consider the regret ratio with other the cluster
            {
                regret = value;
                for (int i = 0; i < c->clusters.size(); ++i)
                {
                    if(i != list_sid[s])
                    {
                        tuple_set *tset1 = clu, *tset2 = c->clusters[i];
                        value = list[tset1->bettupleset[tset2->ID].node_index]->regret_check(tset1->tuples[tid], tset2->tuples, tset1->bettupleset[tset2->ID].position, R->ext_pts);
                        if (value >= 1)
                        {
                            regret = 1;
                            break;
                        }
                        else if (value > regret)
                            regret = value;
                    }
                }
            }

            if(regret < minregret)
            {
                minregret = regret;
                maxp = clu->tuples[tid];
            }
        }
        return minregret;
    }
}


/**
 * @brief Calculate the regret ratio
 * @param c The clusters containing tuples with different categorical value
 * @return  The regret ratio
 */
double relational_graph::regret_ratio(cluster_t *c, std::vector<point_t*> &R, int &maxsid)
{
    double regret = 1, minregret = 1;
    std::vector<int> list_sid;
    c->find_tupleset_relationexist(list_sid);
    for(int s = 0; s < list_sid.size(); ++s)
    {
        double value = c->clusters[list_sid[s]]->regret_sameset(R); //consider the regret ratio in the cluster
        if(value < 1) //consider the regret ratio with other the cluster
        {
            regret = value;
            for (int i = 0; i < c->clusters.size(); ++i)
            {
                if(i != list_sid[s])
                {
                    tuple_set *tset1 = c->clusters[list_sid[s]], *tset2 = c->clusters[i];
                    value = list[tset1->bettupleset[tset2->ID].node_index]->regret_check(tset1->tuples[0], tset2->tuples, tset1->bettupleset[tset2->ID].position, R);
                    if (value >= 1)
                    {
                        regret = 1;
                        break;
                    }
                    else if (value > regret)
                        regret = value;
                }
            }
        }
    }
    return minregret;
}













