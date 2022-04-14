#include "cluster_t.h"

/**
 * @Constructor
 */
cluster_t::cluster_t(){}

/**
 * @brief         Constructor
 * @param p_set   The tuple set
 */
cluster_t::cluster_t(tuple_set *t_set)
{
    int size = t_set->tuples.size();
    int d_cat = t_set->tuples[0]->d_cat;
    int count = 0;
    tuple_t * tp;
    for(int i = 0; i < size; i++)//tuple
    {
        tp = new tuple_t(t_set->tuples[i]);
        bool exist = false;
        int j;
        for(j = 0; j < count; j++)//*string
        {
            exist = true;
            for(int k = 0; k < d_cat; k++)//string
            {
                if (label[j][k] != tp->attr_cat[k])
                {
                    exist = false;
                    break;
                }
            }
            if(exist)
                break;
        }
        if(exist)//tuple has a cluster
        {
            clusters[j]->tuples.push_back(tp);
        }
        else//create a new cluster
        {
            tuple_set *ts = new tuple_set();
            ts->tuples.push_back(tp);
            clusters.push_back(ts);
            std::string *s = new std::string[d_cat];
            for(int k = 0; k < d_cat; k++)//string
            {
                s[k] = tp->attr_cat[k];
            }
            label.push_back(s);
            ++count;
        }
    }
    size = clusters.size();
    for(int i = 0; i < size; ++i)
    {
        clusters[i]->ID = i;
        clusters[i]->bettupleset = new between_tupleset[size];
        clusters[i]->relation_exist = new int[size];
        for(int j = 0; j < size; ++j)
            clusters[i]->relation_exist[j] = 0;
    }
}

cluster_t::cluster_t(cluster_t *clu_t)
{
    int size = clu_t->clusters.size();
    int d_cat = clu_t->clusters[0]->tuples[0]->d_cat;
    tuple_set *ts;
    std::string *s;
    for(int i = 0; i < size; i++)
    {
        ts = new tuple_set();//tuple
        for(int j = 0; j < clu_t->clusters[i]->tuples.size(); j++)
        {
            ts->tuples.push_back(new tuple_t(clu_t->clusters[i]->tuples[j]));
        }
        clusters.push_back(ts);
        s = new std::string[d_cat];//label
        for(int j = 0; j < d_cat; j++)//string
        {
            s[j] = clu_t->label[i][j];
        }
        label.push_back(s);
    }
}

cluster_t::~cluster_t()
{
    int size = clusters.size();
    for(int i = 0; i < size; i++)
    {
        delete clusters[i];
        delete[] label[i];
    }
}

void cluster_t::prune(std::vector<point_t*> &R)
{
    int size = clusters.size();
    for(int i = 0; i < size; ++i)
    {
        if(clusters[i]->tuples.size() > 1)
            clusters[i]->prune_same_cat(R);
    }
}

void cluster_t::prune(hyperplane_set *R)
{
    int size = clusters.size();
    for(int i = 0; i < size; ++i)
    {
        if(clusters[i]->tuples.size() > 1)
            R->prune_same_cat(clusters[i]);
    }
}

/**
 * @brief   Print all the clusters
 */
void cluster_t::print()
{
    int size = clusters.size(), d_cat = clusters[0]->tuples[0]->d_cat;
    for(int i = 0; i < size; i++)
    {
        //print label
        for(int j = 0; j < d_cat; j++)
            std::cout<<label[i][j]<<" ";
        std::cout<<"\n";
        //print tuples
        for(int j = 0; j < clusters[i]->tuples.size(); j++)
        {
            clusters[i]->tuples[j]->p->print();
            //clusters[i]->tuples[j]->print();

        }
        std::cout<<"\n";
    }
}

/**
 * @brief   Print all the tuples
 */
void cluster_t::print_tuple()
{
    std::cout<<"point left\n";
    std::cout<<"======================\n";
    for(int i = 0; i < clusters.size(); i++)
        for(int j=0; j < clusters[i]->tuples.size(); j++)
            std::cout<< clusters[i]->tuples[j]->id <<"\n";
    std::cout<<"======================\n\n";
}

/**
 * @brief   Find the tuple set which has the relation with all other tuple set
 * @return  --1 There is no such tuple set
 *          -i  The index of the tuple set
 */
void cluster_t::find_tupleset_relationexist(std::vector<int> &sid)
{
    int num = clusters.size();
    int *index_cluster = new int[num];
    for(int i = 0; i < num; ++i)
        index_cluster[i] = clusters[i]->ID;

    for(int i = 0; i < num; ++i)
    {
        if(clusters[i]->is_relation_cover(i, num, index_cluster))
            sid.push_back(i);
    }
}

/**
 * @brief Select two tuples from the dataset
 * @param t1    The first tuple
 * @param t2    The second tuple
 */
void cluster_t::select_tuple(tuple_t *&t1, tuple_t *&t2, int &cid_1, int &cid_2, int &tid_1, int &tid_2)
{
    int numCat = INF, dim_cat = clusters[0]->tuples[0]->d_cat;
    cid_1 = rand() % clusters.size();//id of the cluster
    tid_1 = rand() % clusters[cid_1]->tuples.size();//id of the tuple1
    cid_2 = cid_1;

    for(int i = 0; i < clusters.size(); ++i)
    {
        if(i != cid_1)
        {
            int numc = 0;
            for(int k = 0; k < dim_cat; ++k)
            {
                if(clusters[cid_1]->tuples[0]->attr_cat[k] != clusters[i]->tuples[0]->attr_cat[k])
                    ++numc;
            }
            if(numc < numCat)
            {
                numCat = numc;
                cid_2 = i;
            }
            if(numCat == 1)
                break;
        }
    }

    tid_2 = rand() % clusters[cid_2]->tuples.size();//id of the tuple2
    while (cid_1 == cid_2 && tid_1 == tid_2)
        tid_2 = rand() % clusters[cid_2]->tuples.size();//id of the tuple2
    t1 = clusters[cid_1]->tuples[tid_1];
    t2 = clusters[cid_2]->tuples[tid_2];
}

/**
 * @brief Selction two tuples which has the same categorical values
 * @param t1    The first tuple
 * @param t2    The second tuple
 */
void cluster_t::select_tuple_sameset(tuple_t *&t1, tuple_t *&t2, int &cid_1, int &cid_2, int &tid_1, int &tid_2)
{
    //cluster
    int cid = rand() % clusters.size();
    while(clusters[cid]->tuples.size() < 2)
    {
        cid = (cid + 1) % clusters.size();
    }
    //tuples
    tid_1 = rand() % clusters[cid]->tuples.size(); //id of the tuple1
    tid_2 = rand() % clusters[cid]->tuples.size(); //id of the tuple2
    while (tid_1 == tid_2)
    {
        tid_2 = rand() % clusters[cid]->tuples.size(); //id of the tuple2
    }
    t1 = clusters[cid]->tuples[tid_1];
    t2 = clusters[cid]->tuples[tid_2];
    cid_1 = cid; cid_2 = cid;
}

/**
 * @brief   Count the number of clusters, delete clusters which do not contain any tuple
 * @return
 */
int cluster_t::count_tuple()
{
    int t_count = 0, csize = 0;
    int cs = clusters.size();
    for(int i = 0; i < cs; ++i)
    {
        if(clusters[csize]->tuples.size() <= 0)
            clusters.erase(clusters.begin() + csize);
        else
        {
            t_count += clusters[csize]->tuples.size();
            ++csize;
        }
    }
    return t_count;
}

/**
 * @brief   Count the number of tuple sets whose number of tuples are 1
 * @return  The number of tuple sets
 */
int cluster_t::count_cluster()
{
    int sizeone = 0;
    for(int i = 0; i < clusters.size(); ++i)
    {
        if (clusters[i]->tuples.size() == 1)
            sizeone++;
    }
    return sizeone;

}

/**
 * @brief   Calculate the regret ratio in each tuple set (with different categorical values)
 * @return
 */
double cluster_t::regret_ratio(hyperplane_set *R, int dim)
{
    point_t *ap = new point_t(dim); int maxp;
    R->average_point(ap);
    double regret = 0, value;

    for(int i = 0; i < clusters.size(); ++i)
    {
        //find the point with the largest utility w.r.t. ap
        double maxvalue = -1;
        for(int j = 0; j < clusters[i]->tuples.size(); ++j)
        {
            value = clusters[i]->tuples[j]->dot_prod_num(ap);
            if (value > maxvalue)
            {
                maxvalue = value;
                maxp = j;
            }
        }

        //calculate the regret
        value = clusters[i]->regret_sameset(R->ext_pts, maxp);
        if(value > regret)
            regret = value;
    }
    return regret;
}

/**
 * @brief Delete the tuples in each tuple set, only maintain the first tuple
 */
void cluster_t::maintain_one(hyperplane_set *R, int dim)
{
    point_t *ap = new point_t(dim); tuple_t *maxp;
    R->average_point(ap);

    for(int i = 0; i < clusters.size(); ++i)
    {
        double maxvalue = -1;
        for(int j = 0; j < clusters[i]->tuples.size(); ++j)
        {
            double value = clusters[i]->tuples[j]->dot_prod_num(ap);
            if (value > maxvalue)
            {
                maxvalue = value;
                maxp = clusters[i]->tuples[j];
            }
        }

        //only remain tuple maxp
        while(clusters[i]->tuples.size() > 0)
            clusters[i]->tuples.pop_back();
        clusters[i]->tuples.push_back(maxp);
    }
}

/**
 * @brief Calculate the real regret of the selected point
 * @param R                     The numerical utility range
 * @param u                     The utility vector
 * @param categorical_value     The mapping from categorical value to numerical value
 * @param max_value             The utility of the largest point
 * @param maxp                  The selected represented tuple
 * @return
 */
double cluster_t::regret_ratio(hyperplane_set *R, double *u, std::map<std::string, double> &categorical_value, double max_value, tuple_t *&maxp)
{
    double maxvalue = -1;
    int dim = clusters[0]->tuples[0]->d_num;
    point_t* ap = new point_t(dim);
    R->average_point(ap);

    for(int i = 0; i < clusters.size(); ++i)
    {
        for(int j = 0; j < clusters[i]->tuples.size(); ++j)
        {
            double value = clusters[i]->tuples[j]->dot_prod_num(ap);
            if (value > maxvalue)
            {
                maxvalue = value;
                maxp = clusters[i]->tuples[j];
            }
        }
    }

    return 1;
}


double cluster_t::regret_bound(hyperplane_set *R, tuple_t *tpl)
{
    double regret = 0;
    for(int i = 0; i < clusters.size(); ++i)
    {
        for(int j = 0; j < clusters[i]->tuples.size(); ++j)
        {
            double value = R->regret_bound(clusters[i]->tuples[j]->p, tpl->p);
            if(value > regret)
                regret = value;
        }
    }

    return regret;
}

/**
 * @brief Find the tuple which has the maximum utility
 * @param expU The utility function
 * @return The tuple
 */
tuple_t *cluster_t::findBest(point_t *expU)
{
    double maxValue = 0;
    tuple_t *maxT;
    for(int i = 0; i < clusters.size(); ++i)
    {
        for(int j = 0; j < clusters[i]->tuples.size(); ++j)
        {
            double value = clusters[i]->tuples[j]->p->dot_product(expU);
            if (value > maxValue)
            {
                maxValue = value;
                maxT = clusters[i]->tuples[j];
            }
        }
    }
    return maxT;
}






























