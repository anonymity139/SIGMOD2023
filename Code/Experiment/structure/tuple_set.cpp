#include "tuple_set.h"
#include <fstream>
#include <iostream>
/*
 * @brief Destructor
 *        Delete the tuples in the array
 */
tuple_set::~tuple_set()
{
    int i = tuples.size();
    tuple_t *t;
    while(i > 0)
    {
        t = tuples[i-1];
        tuples.pop_back();
        delete t;
        i--;
    }
    tuples.clear();
}

/**
 * @brief Constructor
 */
tuple_set::tuple_set()
{
    relation_exist = NULL;
    bettupleset = NULL;
}

/**
 * @brief Constructor
 *        Record all the tuples in the input file to the tuple set
 * @param input     Name of input file.
 */
tuple_set::tuple_set(const char *input)
{
    relation_exist = NULL;
    bettupleset = NULL;
    std::ifstream c_fp;
    char filename[MAX_FILENAME_LENG];
    sprintf(filename, "../input/%s", input);
    //printf("%s\n", filename);
    c_fp.open(filename, std::ios::in);

    int number_of_tuples, d_cat, d_num;
    c_fp >> number_of_tuples >> d_cat >> d_num;

    // read points line by line
    for (int i = 0; i < number_of_tuples; i++)
    {
        tuple_t *t =new tuple_t(d_cat, d_num, i);
        for (int j = 0; j < d_cat; j++)//categorical attributes
        {
            c_fp >> t->attr_cat[j];
        }
        for (int j = 0; j < d_num; j++)//numerical attributes
        {
            c_fp >> t->attr_num[j];
        }
        tuples.push_back(t);
        //tuples[i]->print();
    }

    c_fp.close();
}

/**
 * @brief   Transform all the tuples(categorical&&numerical) to points (numerical)
 *          Each categorical value is transformed to one attributes (value 0/1)
 * @param categorical   It records all the categorical values in order (values in 1st dimension, values in 2nd dimension, ..., values in dim_cat-th dimension)
 * @return  The transformed point set
 */
point_set* tuple_set::transform(std::vector<std::string> &categorical)
{
    //initialization
    int size = tuples.size();
    if(size <= 0)
        return NULL;
    int dim_cat = tuples[0]->d_cat, dim_num = tuples[0]->d_num;
    int index = 0;//the number of categorical values
    std::map<std::string, int> list;

    //search all the categorical values
    std::map<std::string, int>::iterator it;
    for(int j = 0; j < dim_cat; j++)
    {
        for(int i = 0; i < size; i++)
        {
            it = list.find(tuples[i]->attr_cat[j]);
            if(it == list.end())
            {
                list.insert(std::pair<std::string, int>(tuples[i]->attr_cat[j], index));
                categorical.push_back(tuples[i]->attr_cat[j]);
                ++index;
            }
        }
    }

    //build point set
    point_set* p_set = new point_set();
    point_t *p;
    for(int i = 0; i < size; i++)
    {
        p = new point_t(index + dim_num, i);
        for(int j = 0; j < dim_cat; j++)//categorical part transformation
        {
            it = list.find(tuples[i]->attr_cat[j]);
            p->attr[it->second] = 1;
        }
        for(int j = index; j < index + dim_num; j++)//numerical part
            p->attr[j] = tuples[i]->attr_num[j - index];

        p_set->points.push_back(p);
        tuples[i]->p = p;
    }
    return p_set;
}

/**
 * @brief       Find the skyline points based on the numerical attributes only
 * @return      The tuple set
 */
tuple_set *tuple_set::skyline_based_num()
{
    int size = tuples.size();
    if(size <=0)
    {
        std::cout<<"Error: dataset for producing skyline is empty.";
        return NULL;
    }
    int i, j, m, dominated, index = 0;
    int d_num = tuples[0]->d_num, d_cat = tuples[0]->d_cat;
    tuple_t* tp;
    int* sl = new int[size];

    for (i = 0; i < size; ++i)
    {
        dominated = 0;
        tp = tuples[i];
        // check if pt is dominated by the skyline so far
        for (j = 0; j < index && !dominated; ++j)
            if (tuples[sl[j]]->is_same_cat(tp) && tuples[sl[j]]->dominates_num(tp))
                dominated = 1;

        if (!dominated)
        {
            //eliminate any points in current skyline that it dominates
            m = index;//number of current tuples
            index = 0;
            for (j = 0; j < m; ++j)
                if (!tuples[sl[j]]->is_same_cat(tp) || !tp->dominates_num(tuples[sl[j]]))
                    sl[index++] = sl[j];

            // add this point as well
            sl[index++] = i;
        }
    }
    //record skyline tuples
    tuple_set* skyline = new tuple_set();
    tuple_t *p;
    for (int i = 0; i < index; ++i)
    {
        p = new tuple_t(tuples[sl[i]]);
        skyline->tuples.push_back(p);
    }

    delete[] sl;
    return skyline;
}

/**
 * @brief Print all the tuples
 */
void tuple_set::print()
{
    int size = tuples.size();
    for(int i = 0; i < size; i++)
    {
        tuples[i]->print();
    }
}

/**
 * @brief       Prune tuples based on the utility range
 *              Assume tuples have the same categorical values (consider numerical part only)
 * @param R     The utility range
 */
void tuple_set::prune_same_cat(std::vector<point_t*> &R)
{
    int size = tuples.size();
    bool *Cid = new bool[size];//false means delete the tuple
    for(int i = 0; i < size; i++)
        Cid[i] = true;

    //check dominate relation
    for(int i = 0; i < size; ++i)
    {
        if(Cid[i])
        {
            for(int j = 0; j < size; j++)
            {
                if(i!=j && Cid[j])
                {
                    if(tuples[i]->R_dominates_num(tuples[j], R))
                    {
                        //std::cout<<tuples[i]->id<<"dominates"<<tuples[j]->id<<"\n";
                        Cid[j] = false;
                    }
                }
            }
        }
    }
    //prune tuples based on Cid
    for(int i = size - 1; i >= 0; i--)
    {
        if(!Cid[i])
            tuples.erase(tuples.begin() + i);
    }
}



/**
 * @brief       Find all the categorical value
 * @param list  Containing all the categorical values
 */
void tuple_set::find_categorical(std::map<std::string, double> &list)
{
    int size = tuples.size();
    int dim_cat = tuples[0]->d_cat;
    int index = 0;

    //search all the categorical values
    std::map<std::string, double>::iterator it;
    for(int j = 0; j < dim_cat; j++)
        for(int i = 0; i < size; i++)
        {
            it = list.find(tuples[i]->attr_cat[j]);
            if (it == list.end())
            {
                list.insert(std::pair<std::string, int>(tuples[i]->attr_cat[j], index));
                ++index;
            }
        }
}

/**
 * @brief       Find all the categorical value, classified them in different attributes
 * @param list  Containing all the categorical values
 */
void tuple_set::find_categorical(std::map<std::string, double> *list, double *num_c)
{
    int size = tuples.size();
    int dim_cat = tuples[0]->d_cat;
    //list = new std::map<std::string, double>[dim_cat];
    //num_c = new double[dim_cat];

    //search all the categorical values
    std::map<std::string, double>::iterator it;
    for(int j = 0; j < dim_cat; j++)
    {
        int index = 0;
        for (int i = 0; i < size; i++)
        {
            it = list[j].find(tuples[i]->attr_cat[j]);
            if (it == list[j].end())
            {
                list[j].insert(std::pair<std::string, int>(tuples[i]->attr_cat[j], index));
                ++index;
            }
        }
        num_c[j] = index;
    }
}

/**
 * @brief   Check whether the tuple set has relation with all the other tuple set
 * @return  -true Its relation covers all tuple set
 *          -false Its relation does not cover all tuple set
 */
bool tuple_set::is_relation_cover(int index, int num, int *index_cluster)
{
    for(int i = 0; i < num; ++i)
    {
        if(i != index && relation_exist[index_cluster[i]] == 0)
            return false;
    }
    return true;
}

/**
 * @brief   Check the regret ratio among tuples in the same set
 * @return  The regret ratio
 */
double tuple_set::regret_sameset(std::vector<point_t*> &R)
{
    int size = tuples.size(), ext_size = R.size(), dim = tuples[0]->d_num;
    if(size <= 1)
        return 0;
    double regret = 0, numerator, denominator, value;
    for(int i = 1; i < size; ++i)
    {
        //denominator
        denominator = tuples[i]->dot_prod_num(R[0]);
        for (int j = 1; j < ext_size; ++j)
        {
            value = tuples[i]->dot_prod_num(R[j]);
            if (value < denominator)
                denominator = value;
        }
        //numerator
        double *v = new double[dim];
        for (int d = 0; d < dim; d++)
            v[d] = tuples[i]->attr_num[d] - tuples[0]->attr_num[d];
        numerator = R[0]->dot_product(v);
        for (int j = 1; j < ext_size; ++j)
        {
            value = R[j]->dot_product(v);
            if (value > numerator)
                numerator = value;
        }
        //regret
        value = numerator / denominator;
        if (value > regret)
            regret = value;
    }
    return regret;
}

/**
 * @brief   Check the regret ratio among tuples in the same set
 * @param   The numerical utility range
 * @param   The represented point ID
 * @return  The regret ratio
 */
double tuple_set::regret_sameset(std::vector<point_t*> &R, int ID)
{
    int size = tuples.size(), ext_size = R.size(), dim = tuples[0]->d_num;
    if(size <= 1)
        return 0;
    double regret = 0, numerator, denominator, value;
    for(int i = 0; i < size; ++i)
    {
        if(i != ID)
        {
            //denominator
            denominator = tuples[i]->dot_prod_num(R[0]);
            for (int j = 1; j < ext_size; ++j)
            {
                value = tuples[i]->dot_prod_num(R[j]);
                if (value < denominator)
                    denominator = value;
            }
            //numerator
            double *v = new double[dim];
            for (int d = 0; d < dim; d++)
                v[d] = tuples[i]->attr_num[d] - tuples[ID]->attr_num[d];
            numerator = R[0]->dot_product(v);
            for (int j = 1; j < ext_size; ++j)
            {
                value = R[j]->dot_product(v);
                if (value > numerator)
                    numerator = value;
            }
            //regret
            value = numerator / denominator;
            if (value > regret)
                regret = value;
        }
    }
    return regret;
}

/**
 * @brief Find the largest tuple in the tuple set w.r.t. u
 * @param u                     The utility vector
 * @param categorical_value     The mapping from categorical value to numerical value
 * @return                      The point ID
 */
int tuple_set::max_uvalue(point_t *u, std::map<std::string, double> &categorical_value)
{
    int maxIdx = 0, size = tuples.size();
    double maxValue = 0;
    for (int i = 0; i < size; i++)
    {
        double value = tuples[i]->dot_prod_num(u);//utility of the points
        if (value > maxValue)
        {
            maxValue = value;
            maxIdx = i;
        }
    }
    return maxIdx;
}




























































