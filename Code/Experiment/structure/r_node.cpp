#include "r_node.h"
#include "../Others/lp.h"
#include <sys/time.h>

/**
 * @brief Constructor
 */
node_relation::node_relation(){}

/**
 * @brief Constructor
 * @param n1    R_node
 * @param n2    R_node
 * @param ty    Relation type
 */
node_relation::node_relation(r_node *n1, r_node *n2, r_node* nodesum, int ty1, int ty2)
{
    node_1 = n1;
    node_2 = n2;
    node_sum = nodesum;
    //A +/- B = +/- 1/2 C
    type[0] = ty1; //The coefficient of C
    type[1] = ty2; //The coefficient of B
    updated_count = 0;
    expired = false;
}

/**
 * @brief Print the relation
 */
void node_relation::print()
{
    cout<<"Relation: \n ";
    int d = node_1->len;
    for(int j = 0; j < d; j++)
        cout<<setw(10)<<node_1->s_1[j]<<" ";
    cout<<"--";
    for(int j = 0; j < d; j++)
        std::cout<<setw(10)<<node_1->s_2[j]<<" ";

    cout<<"\n ";
    for(int j = 0; j < d; j++)
        cout<<setw(10)<<node_2->s_1[j]<<" ";
    cout<<"--";
    for(int j = 0; j < d; j++)
        std::cout<<setw(10)<<node_2->s_2[j]<<" ";

    cout<<"\n=";

    for(int j = 0; j < d; j++)
        cout<<setw(10)<<node_sum->s_1[j]<<" ";
    cout<<"--";
    for(int j = 0; j < d; j++)
        cout<<setw(10)<<node_sum->s_2[j]<<" ";
    cout<<"\n";

    cout<<"Type: \n ";
    cout<<type[0]<<"  "<<type[1]<<"\n";

    node_1->print_cat();
    node_2->print_cat();
    node_sum->print_cat();

}

/**
 * @brief Constructor
 */
r_node::r_node(){}

/**
 * @brief Constructor
 * @param s1    Categorical values
 * @param s2    Categorical values
 */
r_node::r_node(std::string *s1, std::string *s2, int d_num, int d_cat)
{
    //find the different categorical value pair
    dim = d_num;
    len = d_cat;
    s_1 = new std::string[len];
    s_2 = new std::string[len];
    for(int i = 0; i < d_cat; i++)
    {
        if(s1[i]!=s2[i])//if they are different, record
        {
            s_1[i] = s1[i];
            s_2[i] = s2[i];

        }
        else
        {
            s_1[i] = "\0";
            s_2[i] = "\0";
        }
    }
}

/**
 * @brief Constructor
 * @param t_1   The tuple set containing tuples with the same categorical values
 * @param t_2   The tuple set containing tuples with the same categorical values
 */
r_node::r_node(tuple_set *t_1, tuple_set *t_2)
{
    //find the different categorical value pair
    if(t_1->tuples.empty() || t_2->tuples.empty())
    {
        std::cout<<"Error: Tuple set is empty.";
        return;
    }
    dim = t_1->tuples[0]->d_num;
    len = t_1->tuples[0]->d_cat;
    s_1 = new std::string[len];
    s_2 = new std::string[len];
    std::string *a1 = t_1->tuples[0]->attr_cat;
    std::string *a2 = t_2->tuples[0]->attr_cat;
    for(int i = 0; i < len; i++)
    {
        if(a1[i]!=a2[i])
        {
            s_1[i] = a1[i];
            s_2[i] = a2[i];
        }
        else
        {
            s_1[i] = "\0";
            s_2[i] = "\0";
        }
    }
    sets.emplace_back(t_1, t_2);
    is_modified = false;
    isRelationFound = false;
}

/**
 * @brief Constructor
 * @param t_1   The tuple set containing tuples with the same categorical values
 * @param t_2   The tuple set containing tuples with the same categorical values
 * @param index The index of the node in the list
 */
r_node::r_node(tuple_set *t_1, tuple_set *t_2, int index)
{
    ID = index;
    //find the different categorical value pair
    if(t_1->tuples.empty() || t_2->tuples.empty())
    {
        std::cout<<"Error: Tuple set is empty.";
        return;
    }
    dim = t_1->tuples[0]->d_num;
    len = t_1->tuples[0]->d_cat;
    s_1 = new std::string[len];
    s_2 = new std::string[len];
    std::string *a1 = t_1->tuples[0]->attr_cat;
    std::string *a2 = t_2->tuples[0]->attr_cat;
    for(int i = 0; i < len; i++)
    {
        if(a1[i]!=a2[i])
        {
            s_1[i] = a1[i];
            s_2[i] = a2[i];
        }
        else
        {
            s_1[i] = "\0";
            s_2[i] = "\0";
        }
    }
    sets.emplace_back(t_1, t_2);
    t_1->bettupleset[t_2->ID].node_index = index;
    t_1->bettupleset[t_2->ID].position = 1;
    t_2->bettupleset[t_1->ID].node_index = index;
    t_2->bettupleset[t_1->ID].position = 2;
    is_modified = false;
    isRelationFound = false;
}


/**
 * @brief Deconstructor
 */
r_node::~r_node()
{
    delete[] s_1;
    delete[] s_2;
    ineqleq.clear();
    ineqgeq.clear();
}

/**
 * @brief       Check whether the pair of the tuple sets belongs to the node
 *              If yes, update the node by the tuple sets. Otherwise, return false
 * @param t_1   The tuple set
 * @param t_2   The tuple set
 * @return      1 belong to & update the node
 *              0 not belong to
 */
bool r_node::fit(tuple_set *t_1, tuple_set *t_2, int index)
{
    //find the different categorical value pair
    if(t_1->tuples.empty() || t_2->tuples.empty())
    {
        std::cout<<"Error: Tuple set is empty.";
        return false;
    }
    std::string *a1 = t_1->tuples[0]->attr_cat;
    std::string *a2 = t_2->tuples[0]->attr_cat;
    int count1 =0, sig = 0;
    for(int i = 0; i < len; i++)
    {
        if(a1[i]!=a2[i])
        {
            if(count1 < 1)
            {
                if(s_1[i] != a1[i] || s_2[i] != a2[i])
                {
                    if (s_1[i] == a2[i] && s_2[i] == a1[i])
                    {
                        a1 = t_2->tuples[0]->attr_cat;
                        a2 = t_1->tuples[0]->attr_cat;
                        ++sig;
                    }
                    else
                        return false;
                }
            }
            else
            {
                if(s_1[i] != a1[i] || s_2[i] != a2[i])
                    return false;
            }
            ++count1;
        }
        else
        {
            if(s_1[i] != "\0" || s_2[i] != "\0")
                return false;
        }
    }
    if(!sig)
    {
        sets.emplace_back(t_1, t_2);
        t_1->bettupleset[t_2->ID].node_index = index;
        t_1->bettupleset[t_2->ID].position = 1;
        t_2->bettupleset[t_1->ID].node_index = index;
        t_2->bettupleset[t_1->ID].position = 2;
    }
    else
    {
        sets.emplace_back(t_2, t_1);
        t_1->bettupleset[t_2->ID].node_index = index;
        t_1->bettupleset[t_2->ID].position = 2;
        t_2->bettupleset[t_1->ID].node_index = index;
        t_2->bettupleset[t_1->ID].position = 1;
    }

    return true;
}

/**
 * @brief       Check whether the node containing the categorical values
 *              Only for checking, no updating operation on the node
 * @param s1    The string
 * @param s2    The string
 * @return      0 not belong to
 *              -1 belong to ineqleq
 *              1 belong to ineqgeq
 */
int r_node::fit(std::string *s1, std::string *s2)
{
    //find the different categorical value pair
    std::string *a1 = s1;
    std::string *a2 = s2;
    int count1 = 0, sig = -1;
    for(int i = 0; i < len; i++)
    {
        if(a1[i]!=a2[i])
        {
            if(count1 < 1)
            {
                if(s_1[i] != a1[i] || s_2[i] != a2[i])
                {
                    if (s_1[i] == a2[i] && s_2[i] == a1[i])
                    {
                        a1 = s2;
                        a2 = s1;
                        sig = 1;
                    }
                    else
                        return 0;
                }
            }
            else
            {
                if(s_1[i] != a1[i] || s_2[i] != a2[i])
                    return 0;
            }
            ++count1;
        }
        else
        {
            if(s_1[i] != "\0" || s_2[i] != "\0")
                return 0;
        }
    }
    return sig;
}

/**
 * @brief Check the sum of categorical values of n1, n2 is the current' s categorical value
 * @param n1    Relational node
 * @param n2    Relational node
 * @return      0 No relation
 *              2 A + B =  2C
 *              1 A + B =  C
 *              -1 A + B = -C
 *              -2 A + B = -2C
 */
int r_node::is_sum(r_node *n1, r_node *n2)
{
    int turn =1, count = 0, coef = 0;
    std::string s1, s2;
    std::string *x1 = s_1, *x2 = s_2;//current node categorical value
    for(int i = 0; i < len; i++)
    {
        if(n1->s_1[i] != n2->s_2[i]) //if they are different, record
        {
            s1 = n1->s_1[i]; s2 = n2->s_2[i];
        }
        else //if they are the same, delete
        {
            s1 = "\0"; s2 = "\0";
        }

        if(n2->s_1[i]!="\0" && n1->s_2[i]!="\0")//if both have values
        {
            if(n2->s_1[i] != n1->s_2[i])//if they are not the same
            {
                if(s1 == "\0" && s2 == "\0")
                {
                    if(coef != 2)
                    {
                        coef = 1;
                        s1 = n2->s_1[i];
                        s2 = n1->s_2[i];
                    }
                    else
                        return 0;
                }
                else if(s1 == n2->s_1[i] && s2 == n1->s_2[i])
                {
                    if(coef != 1)
                        coef = 2;
                    else
                        return 0;
                }
                else
                    return 0;
            }
            else //if they are the same
            {
                if(s1 != "\0" && s2 != "\0")
                {
                    if(coef == 2)
                        return 0;
                    else
                        coef = 1;
                }
            }
        }
        else if(n2->s_1[i]=="\0" && n1->s_2[i]!="\0")//if only one has value
        {
            if(s2 == "\0" && s1 != "\0")
            {
                if(coef != 2)
                {
                    coef = 1;
                    s2 = n1->s_2[i];
                }
                else
                    return 0;
            }
            else
                return 0;
        }
        else if(n2->s_1[i]!="\0" && n1->s_2[i]=="\0")//if only one has value
        {
            if(s1 == "\0" && s2 != "\0")
            {
                if(coef != 2)
                {
                    coef = 1;
                    s1 = n2->s_1[i];
                }
                else
                    return 0;
            }
            else
                return 0;
        }
        else //n2->s_2[i]=="\0" && n1->s_2[i]=="\0"
        {
            if(s1 != "\0" && s2 != "\0")
            {
                if(coef != 2)
                    coef = 1;
                else
                    return 0;
            }
        }
        //check
        if(x1[i] != s1 || x2[i] != s2)
        {
            if(count < 1)
            {
                if(x2[i] == s1 && x1[i] == s2)
                {
                    x1 = s_2;
                    x2 = s_1;
                    turn = -turn;
                    ++count;
                }
                else
                    return 0;
            }
            else
                return 0;
        }
        else if(x1[i] != "\0" && x2[i] != "\0")
            ++count;
    }
    return turn*coef;
}

/**
 * @brief Check the sum of categorical values of n1, n2 is the current' s categorical value
 * @param n1    Relational node
 * @param n2    Relational node
 * @return      0 No relation
 *              2 A - B =  2C
 *              1 A - B =  C
 *              -1 A - B = -C
 *              -2 A - B = -2C
 */
int r_node::is_difference(r_node *n1, r_node *n2)
{
    int turn = 1, count = 0, coef = 0;
    std::string s1, s2;
    std::string *x1 = s_1, *x2 = s_2;//current node categorical value
    for(int i = 0; i < len; i++)
    {
        if(n1->s_1[i] != n2->s_1[i]) //if they are different, record
        {
            s1 = n1->s_1[i]; s2 = n2->s_1[i];
        }
        else //if they are the same, delete
        {
            s1 = "\0"; s2 = "\0";
        }
        if(n2->s_2[i]!="\0" && n1->s_2[i]!="\0")//if both have values
        {
            if(n2->s_2[i] != n1->s_2[i])//if they are not the same
            {
                if(s1 == "\0" && s2 == "\0")
                {
                    if(coef != 2)
                    {
                        coef = 1;
                        s1 = n2->s_2[i];
                        s2 = n1->s_2[i];
                    }
                    else
                        return 0;
                }
                else if(s1 == n2->s_2[i] && s2 == n1->s_2[i])
                {
                    if(coef != 1)
                        coef = 2;
                    else
                        return 0;
                }
                else
                    return 0;
            }
            else //if they are the same
            {
                if(s1 != "\0" && s2 != "\0")
                {
                    if(coef == 2)
                        return 0;
                    else
                        coef = 1;
                }
            }
        }
        else if(n2->s_2[i]=="\0" && n1->s_2[i]!="\0")//if only one has value
        {
            if(s2 == "\0" && s1 != "\0")
            {
                if(coef != 2)
                {
                    coef = 1;
                    s2 = n1->s_2[i];
                }
                else
                    return 0;
            }
            else
                return 0;
        }
        else if(n2->s_2[i]!="\0" && n1->s_2[i]=="\0")//if only one has value
        {
            if(s1 == "\0" && s2 != "\0")
            {
                if(coef != 2)
                {
                    coef = 1;
                    s1 = n2->s_2[i];
                }
                else
                    return 0;
            }
            else
                return 0;
        }
        else //n2->s_2[i]=="\0" && n1->s_2[i]=="\0"
        {
            if(s1 != "\0" && s2 != "\0")
            {
                if(coef != 2)
                    coef = 1;
                else
                    return 0;
            }
        }
        //check
        if(x1[i] != s1 || x2[i] != s2)
        {
            if(count < 1)
            {
                if(x2[i] == s1 && x1[i] == s2)
                {
                    x1 = s_2;
                    x2 = s_1;
                    turn = -turn;
                    ++count;
                }
                else
                    return 0;
            }
            else
                return 0;
        }
        else if(x1[i] != "\0" && x2[i] != "\0")
            ++count;
    }
    return turn*coef;
}

/**
 * @brief Sum the categorical values of two r_node A + B = 1/2 C
 * @param n     The r_node
 * @param str   The results
 * @return      --1 No result of C
 *              -1/-2 The coefficiency 1/2
 */
int r_node::add(r_node *n, string *str)
{
    int coef = 0;
    std::string s1, s2;
    for(int i = 0; i < len; i++)
    {
        if(s_1[i] != n->s_2[i]) //if they are different, record
        {
            s1 = s_1[i]; s2 = n->s_2[i];
        }
        else //if they are the same, delete
        {
            s1 = "\0"; s2 = "\0";
        }

        if(n->s_1[i] != "\0" && s_2[i] != "\0")//if both have values
        {
            if(n->s_1[i] != s_2[i])//if they are not the same
            {
                if(s1 == "\0" && s2 == "\0")
                {
                    if(coef != 2)
                    {
                        coef = 1;
                        s1 = n->s_1[i];
                        s2 = s_2[i];
                    }
                    else
                        return -1;
                }
                else if(s1 == n->s_1[i] && s2 == s_2[i])
                {
                    if(coef != 1)
                        coef = 2;
                    else
                        return -1;
                }
                else
                    return -1;
            }
            else //if they are the same
            {
                if(s1 != "\0" && s2 != "\0")
                {
                    if(coef == 2)
                        return -1;
                    else
                        coef = 1;
                }
            }
        }
        else if(n->s_1[i]=="\0" && s_2[i] != "\0")//if only one has value
        {
            if(s2 == "\0" && s1 != "\0")
            {
                if(coef != 2)
                {
                    coef = 1;
                    s2 = s_2[i];
                }
                else
                    return -1;
            }
            else
                return -1;
        }
        else if(n->s_1[i] != "\0" && s_2[i] == "\0")//if only one has value
        {
            if(s1 == "\0" && s2 != "\0")
            {
                if(coef != 2)
                {
                    coef = 1;
                    s1 = n->s_1[i];
                }
                else
                    return -1;
            }
            else
                return -1;
        }
        else //n2->s_2[i]=="\0" && n1->s_2[i]=="\0"
        {
            if(s1 != "\0" && s2 != "\0")
            {
                if(coef != 2)
                    coef = 1;
                else
                    return -1;
            }
        }
        str[i] = s1;
        str[len + i] = s2;
    }
    return coef;
}

/**
 * @brief The difference of the categorical values of two r_node A - B = 1/2 C
 * @param n     The r_node
 * @param str   The categorical values
 * @return      -1 No result of C
 *              -1/-2 The coefficiency 1/2
 */
int r_node::subtract(r_node *n, string *str)
{
    int coef = 0;
    std::string s1, s2;
    for(int i = 0; i < len; i++)
    {
        if (s_1[i] != n->s_1[i]) //if they are different, record
        {
            s1 = s_1[i];
            s2 = n->s_1[i];
        }
        else //if they are the same, delete
        {
            s1 = "\0";
            s2 = "\0";
        }

        if (n->s_2[i] != "\0" && s_2[i] != "\0")//if both have values
        {
            if (n->s_2[i] != s_2[i])//if they are not the same
            {
                if (s1 == "\0" && s2 == "\0")
                {
                    if (coef != 2)
                    {
                        coef = 1;
                        s1 = n->s_2[i];
                        s2 = s_2[i];
                    }
                    else
                        return -1;
                }
                else if (s1 == n->s_2[i] && s2 == s_2[i])
                {
                    if (coef != 1)
                        coef = 2;
                    else
                        return -1;
                }
                else
                    return -1;
            }
            else //if they are the same
            {
                if (s1 != "\0" && s2 != "\0")
                {
                    if (coef == 2)
                        return -1;
                    else
                        coef = 1;
                }
            }
        }
        else if (n->s_2[i] == "\0" && s_2[i] != "\0")//if only one has value
        {
            if (s2 == "\0" && s1 != "\0")
            {
                if (coef != 2)
                {
                    coef = 1;
                    s2 = s_2[i];
                }
                else
                    return -1;
            }
            else
                return -1;
        }
        else if (n->s_2[i] != "\0" && s_2[i] == "\0")//if only one has value
        {
            if (s1 == "\0" && s2 != "\0")
            {
                if (coef != 2)
                {
                    coef = 1;
                    s1 = n->s_2[i];
                }
                else
                    return -1;
            }
            else
                return -1;
        }
        else //n2->s_2[i]=="\0" && n1->s_2[i]=="\0"
        {
            if (s1 != "\0" && s2 != "\0")
            {
                if (coef != 2)
                    coef = 1;
                else
                    return -1;
            }
        }
        str[i] = s1;
        str[len + i] = s2;
    }
    return coef;
}



/**
 * @brief Sum the categorical values of two r_node C = 2A + B
 * @param n     The r_node
 * @param str   The results
 * @return      -1 No result of C
 *              1/ There exists a result
 */
int r_node::add2(r_node *n, string *str)
{
    std::string s1, s2;
    for(int i = 0; i < len; i++)
    {
        if(s_1[i] != s_2[i]) //if they are different, record
        {
            s1 = s_1[i]; s2 = s_2[i];
            if(s1 != n->s_2[i] || s2 != n->s_1[i])
            {
                return -1;
            }
        }
        else //if they are the same, delete
        {
            if(n->s_1[i] != n->s_2[i])//if both have values
            {
                s1 = n->s_1[i];
                s2 = n->s_2[i];
            }
            else
            {
                s1 = "\0"; s2 = "\0";
            }
        }
        str[i] = s1;
        str[len + i] = s2;
    }
    return 1;
}


/**
 * @brief The difference of the categorical values of two r_node C = 2A - B
 * @param n     The r_node
 * @param str   The categorical values
 * @return      -1 No result of C
 *              1 There exists a result
 */
int r_node::subtract2(r_node *n, string *str)
{
    std::string s1, s2;
    for(int i = 0; i < len; i++)
    {
        if (s_1[i] != s_2[i]) //if they are different, record
        {
            s1 = s_1[i]; s2 = s_2[i];
            if (s1 != n->s_1[i] || s2 != n->s_2[i])
            {
                return -1;
            }
        }
        else //if they are the same, delete
        {
            if(n->s_1[i] != n->s_2[i])//if both have values
            {
                s1 = n->s_2[i];
                s2 = n->s_1[i];
            }
            else
            {
                s1 = "\0"; s2 = "\0";
            }
        }
        str[i] = s1;
        str[len + i] = s2;
    }
    return 1;
}



/**
 * @brief Check the sum of categorical values of n1, n2 is the current' s categorical value
 * @param n1    Relational node
 * @param n2    Relational node
 * @return      0 No relation
 *              1 A + B =  C
 */
int r_node::is_sum_posi(r_node *n1, r_node *n2)
{
    int turn = 1, count = 0;
    std::string s1, s2;
    for(int i = 0; i < len; i++)
    {
        //handle n1_1, n2_2
        if(n1->s_1[i] != n2->s_2[i])//if they are different, record
        {
            s1 = n1->s_1[i];
            s2 = n2->s_2[i];
        }
        else
        {
            s1 = "\0";
            s2 = "\0";
        }
        //handle n1_2, n2_1
        if(n2->s_1[i]!="\0" && n1->s_2[i]!="\0")//if both have values
        {
            if(n2->s_1[i] != n1->s_2[i])//if they are not the same
            {
                if(s1 == "\0" && s2 == "\0")
                {
                    s1 = n2->s_1[i];
                    s2 = n1->s_2[i];
                }
                else
                    return false;
            }
        }
        else if(n2->s_1[i]=="\0" && n1->s_2[i]!="\0")//if only one has value
        {
            if(s2 == "\0")
                s2 = n1->s_2[i];
            else
                return false;
        }
        else if(n2->s_1[i]!="\0" && n1->s_2[i]=="\0")//if only one has value
        {
            if(s1 == "\0")
                s1 = n2->s_1[i];
            else
                return false;
        }
        //check
        if(s_1[i] != s1 || s_2[i] != s2)
            return false;
    }
    return true;
}

/**
 * @brief Based on R, delete unnecessary inequalities (which are R-dominated by other inequalities (vectors))
 * @param R The numerical utility range
 */
void r_node::update(std::vector<point_t *> &R)
{
    //update ineqleq
    int size = ineqleq.size(), m, cover, index = 0, *sl;
    double *pt;
    if(size > 0)
    {
        sl = new int[size];
        for (int i = 0; i < size; ++i)
        {
            cover = 0;
            for (int j = 0; j < index && !cover; ++j)
                if (R_cover(ineqleq[i].norm, ineqleq[sl[j]].norm, R))
                    cover = 1;
            if (!cover)
            {
                // eliminate those that it covers
                m = index;
                index = 0;
                for (int j = 0; j < m; ++j)
                    if (!R_cover(ineqleq[sl[j]].norm, ineqleq[i].norm, R))
                        sl[index++] = sl[j];
                // add this inequality as well
                sl[index++] = i;
            }
        }
        //record
        cover = 0;
        for (int i = 0; i < size && cover < index; ++i)
        {
            if (i < sl[cover])
                ineqleq.erase(ineqleq.begin() + cover);
            else
                cover++;
        }
        size = ineqleq.size();
        for (int i = index; i < size; ++i)
            ineqleq.erase(ineqleq.begin() + index);
        delete[] sl;
    }

    //update ineqgeq
    size = ineqgeq.size();
    if (size <= 0)
        return;
    index = 0;
    sl = new int[size];
    for (int i = 0; i < size; ++i)
    {
        cover = 0;
        // check if pt is covered by others so far
        for (int j = 0; j < index && !cover; ++j)
        {
            if (R_cover(ineqgeq[sl[j]].norm, ineqgeq[i].norm, R))
                cover = 1;
        }
        if (!cover)
        {
            // eliminate those that it covers
            m = index;//number of current points
            index = 0;
            for (int j = 0; j < m; ++j)
                if (!R_cover(ineqgeq[i].norm, ineqgeq[sl[j]].norm, R))
                    sl[index++] = sl[j];
            // add this inequality as well
            sl[index++] = i;
        }
    }
    //record
    for (int i = 0, j = 0; i < size && j < index; ++i)
    {
        if (i < sl[j])
            ineqgeq.erase(ineqgeq.begin() + j);
        else
            j++;
    }
    size = ineqgeq.size();
    for (int i = index; i < size; ++i)
        ineqgeq.erase(ineqgeq.begin() + index);

    delete[] sl;
}

/**
 * @brief   Based on R, delete inequalities (which could not be the top-1 vector w.r.t. any utility vector in R)
 * @param R The numerical utility range
 */
void r_node::update(hyperplane_set *R)
{
    std::vector<double*> arr;
    //update the ineqleq
    int count = 0;
    while(count < ineqleq.size())
    {
        std::vector<double *> arr;
        for (int j = 0; j < ineqleq.size(); ++j)
        {
            if (j != count)
                arr.push_back(ineqleq[j].norm);
        }
        if (!R->istop_leq(arr, ineqleq[count].norm))
            ineqleq.erase(ineqleq.begin() + count);
        else
            ++count;
    }
    //update the ineqgeq
    count = 0;
    while(count < ineqgeq.size())
    {
        std::vector<double *> arr;
        for (int j = 0; j < ineqgeq.size(); ++j)
        {
            if (j != count)
                arr.push_back(ineqgeq[j].norm);
        }
        if (!R->istop_geq(arr, ineqgeq[count].norm))
            ineqgeq.erase(ineqgeq.begin() + count);
        else
            ++count;
    }
}

/**
 * @brief Based on R, add a new inequality to the node
 *        Delete unnecessary inequalities
 * @param v         The added inequality
 * @param direction The inequality direction(< or >)
 * @param R         The utility range
 * @return          Which part is updated (ineqleq, ineqgeq)
 *                  -1  ineqleq
 *                  0   not updated
 *                  1   ineqgeq
 */
int r_node::update(double *v, int direction, hyperplane_set *R, int round, std::vector<double> &c)
{
    double updated_sig;
    if(direction == -1)
    {
        int size = ineqleq.size();
        if(size <=0)
        {
            ineqleq.emplace_back(round, v, c);
            update_tupleset_relation(-1);
        }
        else
        {
            for(int i = 0; i < ineqleq.size(); i++)
            {
                if(R->R_dominate(v, ineqleq[i].norm))
                    return 0;
            }
            //if(is_constructed(ineqleq, v, R->ext_pts[0]->d))
            //    return 0;
            updated_sig = R->top_leq_check(ineqleq, v);
            if (updated_sig <= EQN2)
                return 0;
            // eliminate those that it covers
            int count = 0;
            for (int i = 0; i < size; ++i)
            {
                if (R_cover(ineqleq[count].norm, v, R->ext_pts))
                    ineqleq.erase(ineqleq.begin() + count);
                else
                    ++count;
            }
            std::vector<double*> arr;
            count = 0;
            while(count < ineqleq.size())
            {
                std::vector<double *> arr;
                for (int j = 0; j < ineqleq.size(); ++j)
                {
                    if (j != count)
                        arr.push_back(ineqleq[j].norm);
                }
                arr.push_back(v);
                if (!R->istop_leq(arr, ineqleq[count].norm))
                    ineqleq.erase(ineqleq.begin() + count);
                else
                    ++count;
            }
            // add v
            ineqleq.emplace_back(round, v, c);
        }
        if(updated_sig > UP_EPSIlON)
            updated = -1;
        return -1;
    }
    else if(direction == 1)
    {
        int size = ineqgeq.size();
        if(size <= 0)
        {
            ineqgeq.emplace_back(round, v, c);
            update_tupleset_relation(1);
        }
        else
        {
            for(int i = 0; i < ineqgeq.size(); i++)
            {
                if(R->R_dominate(ineqgeq[i].norm, v))
                    return 0;
            }
            //if(is_constructed(ineqgeq, v, R->ext_pts[0]->d))
            //    return 0;
            updated_sig = R->top_geq_check(ineqgeq, v);
            if (updated_sig <= EQN2)
                return 0;
            // eliminate those that it covers
            int count = 0;
            for (int i = 0; i < size; ++i)
            {
                if (R_cover(v, ineqgeq[count].norm, R->ext_pts))
                    ineqgeq.erase(ineqgeq.begin() + count);
                else
                    ++count;
            }
            std::vector<double*> arr;
            count = 0;
            while(count < ineqgeq.size())
            {
                std::vector<double *> arr;
                for (int j = 0; j < ineqgeq.size(); ++j)
                {
                    if (j != count)
                        arr.push_back(ineqgeq[j].norm);
                }
                arr.push_back(v);
                if (!R->istop_geq(arr, ineqgeq[count].norm))
                    ineqgeq.erase(ineqgeq.begin() + count);
                else
                    ++count;
            }
            // add v
            ineqgeq.emplace_back(round, v, c);

        }
        if(updated_sig > UP_EPSIlON)
            updated = 1;
        return 1;
    }
    return 0;
}


/**
 * @brief Based on R, add a new inequality to the node
 *        Delete unnecessary inequalities
 * @param v         The added inequality
 * @param direction The inequality direction(< or >)
 * @param R         The utility range
 * @return          Which part is updated (ineqleq, ineqgeq)
 *                  -1  ineqleq
 *                  0   not updated
 *                  1   ineqgeq
 */
int r_node::update_round(double *v, int direction, hyperplane_set *R, int round, std::vector<double> &c, double u_range)
{
    update_numerical_utilityrange(v, direction, R, u_range);
    double updated_sig;
    if(direction == -1)
    {
        int size = ineqleq.size();
        if(size <=0)
        {
            ineqleq.emplace_back(round, v, c);
            update_tupleset_relation(-1);
        }
        else
        {
            for(int i = 0; i < ineqleq.size(); i++)
            {
                if(R_cover(v, ineqleq[i].norm, R->ext_pts))
                    return 0;
            }
            //if(is_constructed(ineqleq, v, R->ext_pts[0]->d))
            //    return 0;
            updated_sig = R->top_leq_check(ineqleq, v);
            if (updated_sig <= EQN2)
                return 0;
            // eliminate those that it covers
            int count = 0;
            for (int i = 0; i < size; ++i)
            {
                if (R_cover(ineqleq[count].norm, v, R->ext_pts))
                    ineqleq.erase(ineqleq.begin() + count);
                else
                    ++count;
            }
            count = 0;
            while(count < ineqleq.size())
            {
                std::vector<double *> arr;
                for (int j = 0; j < ineqleq.size(); ++j)
                {
                    if (j != count)
                        arr.push_back(ineqleq[j].norm);
                }
                arr.push_back(v);
                if (!R->istop_leq(arr, ineqleq[count].norm))
                    ineqleq.erase(ineqleq.begin() + count);
                else
                    ++count;
            }
            // add v
            ineqleq.emplace_back(round, v, c);
        }
        updated = -1;
        return -1;
    }
    else if(direction == 1)
    {
        int size = ineqgeq.size();
        if(size <= 0)
        {
            ineqgeq.emplace_back(round, v, c);
            update_tupleset_relation(1);
        }
        else
        {
            for(int i = 0; i < ineqgeq.size(); i++)
            {
                if(R_cover(ineqgeq[i].norm, v, R->ext_pts))
                    return 0;
            }
            //if(is_constructed(ineqgeq, v, R->ext_pts[0]->d))
            //    return 0;
            updated_sig = R->top_geq_check(ineqgeq, v);
            if (updated_sig <= EQN2)
                return 0;
            // eliminate those that it covers
            int count = 0;
            for (int i = 0; i < size; ++i)
            {
                if (R_cover(v, ineqgeq[count].norm, R->ext_pts))
                    ineqgeq.erase(ineqgeq.begin() + count);
                else
                    ++count;
            }
            count = 0;
            while(count < ineqgeq.size())
            {
                std::vector<double *> arr;
                for (int j = 0; j < ineqgeq.size(); ++j)
                {
                    if (j != count)
                        arr.push_back(ineqgeq[j].norm);
                }
                arr.push_back(v);
                if (!R->istop_geq(arr, ineqgeq[count].norm))
                    ineqgeq.erase(ineqgeq.begin() + count);
                else
                    ++count;
            }
            // add v
            ineqgeq.emplace_back(round, v, c);
        }
        updated = 1;
        return 1;
    }
    return 0;
}

/**
 * @brief Based on R, add a new inequality to the node
 *        Delete unnecessary inequalities
 * @param v         The added inequality
 * @param direction The inequality direction(< or >)
 * @param R         The utility range
 * @return          Which part is updated (ineqleq, ineqgeq)
 *                  -1  ineqleq
 *                  0   not updated
 *                  1   ineqgeq
 */
int r_node::update_threshold(double *v, int direction, hyperplane_set *R, int round, std::vector<double> &c, double u_range, double THRESHOLD)
{
    update_numerical_utilityrange(v, direction, R, u_range);
    double updated_sig;
    if(direction == -1)
    {
        double utility;
        int size = ineqleq.size();
        if(size <=0)
            update_tupleset_relation(-1);
        else
        {
            for(int i = 0; i < ineqleq.size(); i++)
            {
                if(R_cover(v, ineqleq[i].norm, R->ext_pts))
                    return 0;
            }
            //if(is_constructed(ineqleq, v, R->ext_pts[0]->d))
            //    return 0;
            updated_sig = R->top_leq_check_threshold(ineqleq, v, utility);
            if (updated_sig <= EQN2)
                return 0;
            // eliminate those that it covers
            int count = 0;
            for (int i = 0; i < size; ++i)
            {
                if (R_cover(ineqleq[count].norm, v, R->ext_pts))
                    ineqleq.erase(ineqleq.begin() + count);
                else
                    ++count;
            }
            count = 0;
            while(count < ineqleq.size())
            {
                std::vector<double *> arr;
                for (int j = 0; j < ineqleq.size(); ++j)
                {
                    if (j != count)
                        arr.push_back(ineqleq[j].norm);
                }
                arr.push_back(v);
                if (!R->istop_leq(arr, ineqleq[count].norm))
                    ineqleq.erase(ineqleq.begin() + count);
                else
                    ++count;
            }
        }
        if(abs(updated_sig/(updated_sig + utility)) > THRESHOLD)
            updated = -1;
        ineqleq.emplace_back(round, v, c); //add v
        return -1;
    }
    else if(direction == 1)
    {
        double utility;
        int size = ineqgeq.size();
        if(size <= 0)
            update_tupleset_relation(1);
        else
        {
            for(int i = 0; i < ineqgeq.size(); i++)
            {
                if(R_cover(ineqgeq[i].norm, v, R->ext_pts))
                    return 0;
            }
            //if(is_constructed(ineqgeq, v, R->ext_pts[0]->d))
            //    return 0;
            updated_sig = R->top_geq_check_threshold(ineqgeq, v, utility);
            if (updated_sig <= EQN2)
                return 0;
            // eliminate those that it covers
            int count = 0;
            for (int i = 0; i < size; ++i)
            {
                if (R_cover(v, ineqgeq[count].norm, R->ext_pts))
                    ineqgeq.erase(ineqgeq.begin() + count);
                else
                    ++count;
            }
            count = 0;
            while(count < ineqgeq.size())
            {
                std::vector<double *> arr;
                for (int j = 0; j < ineqgeq.size(); ++j)
                {
                    if (j != count)
                        arr.push_back(ineqgeq[j].norm);
                }
                arr.push_back(v);
                if (!R->istop_geq(arr, ineqgeq[count].norm))
                    ineqgeq.erase(ineqgeq.begin() + count);
                else
                    ++count;
            }
        }
        if(abs(updated_sig/(utility - updated_sig)) > THRESHOLD)
            updated = 1;
        ineqgeq.emplace_back(round, v, c); //add v
        return 1;
    }
    return 0;
}


/**
 * @brief Based on R, add a new inequality to the node
 *        Delete unnecessary inequalities
 * @param v         The added inequality
 * @param direction The inequality direction(< or >)
 * @param R         The utility range
 * @return          Which part is updated (ineqleq, ineqgeq)
 *                  -1  ineqleq
 *                  0   not updated
 *                  1   ineqgeq
 */
int r_node::update_without_check(double *v, int direction, hyperplane_set *R, std::vector<double> &c)
{
    if(direction == -1)
    {
        int size = ineqleq.size();
        if(size <= 0)
        {
            ineqleq.emplace_back(true, v, c);
            update_tupleset_relation(-1);
        }
        else
        {
            // eliminate those that it covers
            int count = 0;
            for (int i = 0; i < size; ++i)
            {
                if (R_cover(ineqleq[count].norm, v, R->ext_pts))
                    ineqleq.erase(ineqleq.begin() + count);
                else
                    ++count;
            }
            std::vector<double*> arr;
            count = 0;
            while(count < ineqleq.size())
            {
                std::vector<double *> arr;
                for (int j = 0; j < ineqleq.size(); ++j)
                {
                    if (j != count)
                        arr.push_back(ineqleq[j].norm);
                }
                arr.push_back(v);
                if (!R->istop_leq(arr, ineqleq[count].norm))
                    ineqleq.erase(ineqleq.begin() + count);
                else
                    ++count;
            }
            // add v
            ineqleq.emplace_back(true, v, c);
        }
        updated = -1;
        return -1;
    }
    else if(direction == 1)
    {
        int size = ineqgeq.size();
        if(size <=0)
        {
            ineqgeq.emplace_back(true, v, c);
            update_tupleset_relation(1);
        }
        else
        {
            // eliminate those that it covers
            int count = 0;
            for (int i = 0; i < size; ++i)
            {
                if (R_cover(v, ineqgeq[count].norm, R->ext_pts))
                    ineqgeq.erase(ineqgeq.begin() + count);
                else
                    ++count;
            }
            std::vector<double*> arr;
            count = 0;
            while(count < ineqgeq.size())
            {
                std::vector<double *> arr;
                for (int j = 0; j < ineqgeq.size(); ++j)
                {
                    if (j != count)
                        arr.push_back(ineqgeq[j].norm);
                }
                arr.push_back(v);
                if (!R->istop_geq(arr, ineqgeq[count].norm))
                    ineqgeq.erase(ineqgeq.begin() + count);
                else
                    ++count;
            }
            // add v
            ineqgeq.emplace_back(true, v, c);

        }
        updated = 1;
        return 1;
    }
    return 0;
}

/**
 * @brief Based on R, add a new inequality to the node
 *        Delete unnecessary inequalities
 * @param v         The added inequality
 * @param direction The inequality direction(< or >)
 * @param R         The utility range
 * @return          Which part is updated (ineqleq, ineqgeq)
 *                  -1  ineqleq
 *                  0   not updated
 *                  1   ineqgeq
 */
int r_node::update_without_check_round(double *v, int direction, hyperplane_set *R, std::vector<double> &c, double u_range)
{
    update_numerical_utilityrange(v, direction, R, u_range);
    if(direction == -1)
    {
        int size = ineqleq.size();
        if(size <= 0)
        {
            ineqleq.emplace_back(true, v, c);
            update_tupleset_relation(-1);
        }
        else
        {
            // eliminate those that it covers
            int count = 0;
            for (int i = 0; i < size; ++i)
            {
                if (R_cover(ineqleq[count].norm, v, R->ext_pts))
                    ineqleq.erase(ineqleq.begin() + count);
                else
                    ++count;
            }
            count = 0;
            while(count < ineqleq.size())
            {
                std::vector<double *> arr;
                for (int j = 0; j < ineqleq.size(); ++j)
                {
                    if (j != count)
                        arr.push_back(ineqleq[j].norm);
                }
                arr.push_back(v);
                if (!R->istop_leq(arr, ineqleq[count].norm))
                    ineqleq.erase(ineqleq.begin() + count);
                else
                    ++count;
                arr.clear();
            }
            // add v
            ineqleq.emplace_back(true, v, c);
        }
        updated = -1;
        return -1;
    }
    else if(direction == 1)
    {
        int size = ineqgeq.size();
        if(size <=0)
        {
            ineqgeq.emplace_back(true, v, c);
            update_tupleset_relation(1);
        }
        else
        {
            // eliminate those that it covers
            int count = 0;
            for (int i = 0; i < size; ++i)
            {
                if (R_cover(v, ineqgeq[count].norm, R->ext_pts))
                    ineqgeq.erase(ineqgeq.begin() + count);
                else
                    ++count;
            }
            count = 0;
            while(count < ineqgeq.size())
            {
                std::vector<double *> arr;
                for (int j = 0; j < ineqgeq.size(); ++j)
                {
                    if (j != count)
                        arr.push_back(ineqgeq[j].norm);
                }
                arr.push_back(v);
                if (!R->istop_geq(arr, ineqgeq[count].norm))
                    ineqgeq.erase(ineqgeq.begin() + count);
                else
                    ++count;
                arr.clear();
            }
            // add v
            ineqgeq.emplace_back(true, v, c);

        }
        updated = 1;
        return 1;
    }
    return 0;
}

/**
 * @brief Based on R, add a new inequality to the node
 *        Delete unnecessary inequalities
 * @param v         The added inequality
 * @param direction The inequality direction(< or >)
 * @param R         The utility range
 * @return          Which part is updated (ineqleq, ineqgeq)
 *                  -1  ineqleq
 *                  0   not updated
 *                  1   ineqgeq
 */
int r_node::update_without_check_threshold(double *v, int direction, hyperplane_set *R, std::vector<double> &c, double u_range, double THRESHOLD)
{
    update_numerical_utilityrange(v, direction, R, u_range);
    if(direction == -1)
    {
        int size = ineqleq.size();
        if(size <= 0)
            update_tupleset_relation(-1);//update the dominance relationship between tuple sets
        else
        {
            // eliminate those that it covers
            int count = 0;
            for (int i = 0; i < size; ++i)
            {
                if (R_cover(ineqleq[count].norm, v, R->ext_pts))
                    ineqleq.erase(ineqleq.begin() + count);
                else
                    ++count;
            }
            count = 0;
            while(count < ineqleq.size())
            {
                std::vector<double *> arr;
                for (int j = 0; j < ineqleq.size(); ++j)
                {
                    if (j != count)
                        arr.push_back(ineqleq[j].norm);
                }
                arr.push_back(v);
                if (!R->istop_leq(arr, ineqleq[count].norm))
                    ineqleq.erase(ineqleq.begin() + count);
                else
                    ++count;
                arr.clear();
            }
        }
        double updated_sig, utility;
        updated_sig = R->top_leq_check_threshold(ineqleq, v, utility);
        if(abs(updated_sig/(updated_sig + utility)) > THRESHOLD)
            updated = -1;
        else
            updated = 0;
        ineqleq.emplace_back(true, v, c); //add v
        return -1;
    }
    else if(direction == 1)
    {
        int size = ineqgeq.size();
        if(size <=0)
            update_tupleset_relation(1);
        else
        {
            // eliminate those that it covers
            int count = 0;
            for (int i = 0; i < size; ++i)
            {
                if (R_cover(v, ineqgeq[count].norm, R->ext_pts))
                    ineqgeq.erase(ineqgeq.begin() + count);
                else
                    ++count;
            }
            count = 0;
            while(count < ineqgeq.size())
            {
                std::vector<double *> arr;
                for (int j = 0; j < ineqgeq.size(); ++j)
                {
                    if (j != count)
                        arr.push_back(ineqgeq[j].norm);
                }
                arr.push_back(v);
                if (!R->istop_geq(arr, ineqgeq[count].norm))
                    ineqgeq.erase(ineqgeq.begin() + count);
                else
                    ++count;
                arr.clear();
            }
        }
        double updated_sig, utility;
        updated_sig = R->top_geq_check_threshold(ineqgeq, v, utility);
        if(abs(updated_sig/(utility - updated_sig)) > THRESHOLD)
            updated = 1;
        else
            updated = 0;
        ineqgeq.emplace_back(true, v, c); //add v
        return 1;
    }
    return 0;
}

/**
 * @brief Update the numerical utility range based on the information in the node and new added inequality
 * @param v             The new added inequality
 * @param direction     The direction
 * @param R             The numerical utility range
 */
void r_node::update_numerical_utilityrange(double *v, int direction, hyperplane_set *R, double urange)
{
    int dim = R->ext_pts[0]->d;
    if(dim > 1)
    {
        if (direction == -1)
        {

            for (int i = 0; i < ineqgeq.size(); ++i)
            {
                double *hy = new double[dim];
                for (int j = 0; j < dim; ++j)
                    hy[j] = ineqgeq[i].norm[j] - v[j];
                R->hyperplanes.emplace_back(new hyperplane(dim, hy, 0));
            }
        }
        else
        {
            for (int i = 0; i < ineqleq.size(); ++i)
            {
                double *hy = new double[dim];
                for (int j = 0; j < dim; ++j)
                    hy[j] = v[j] - ineqleq[i].norm[j];
                R->hyperplanes.emplace_back(new hyperplane(dim, hy, 0));
            }
        }
        R->set_ext_pts(urange);
    }
}

/**
 * @brief Based on R and the node, update the n1 and n2
 * @param n_1       Node 1
 * @param n_2       Node 1
 * @param direction Which part the current node is updated (-1: ineqleq or 1: ineqgeq)
 * @param R         The utility range extreme points
 */
void r_node::update(std::pair<node_relation*, int> nr, int direction, hyperplane_set *R, int round)
{
    //initialize
    nr.first->node_1->updated = 0;
    nr.first->node_2->updated = 0;
    nr.first->node_sum->updated = 0;
    updated = direction;
    r_node *n_1, *n_2;
    int dim = R->ext_pts[0]->d;

    //update the node
    if(direction == -1)//<=
    {
        int size = ineqleq.size();
        for(int i = 0; i < size; ++i)
        {
            if(ineqleq[i].version >= round - 1)
            {
                //A/B A + B = 1/2C
                if (nr.first->type[0] > 0 && nr.first->type[1] == 1 && nr.second <= 2)
                {
                    if(nr.second == 1)
                        n_1 = nr.first->node_2;
                    else
                        n_1 = nr.first->node_1;
                    n_2 = nr.first->node_sum;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update C, where A + B = 1/2C
                    for (int k = 0; k < n_1->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = (ineqleq[i].norm[j] + n_1->ineqleq[k].norm[j])/nr.first->type[0];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = (ineqleq[i].coeff[a] + n_1->ineqleq[k].coeff[a])/nr.first->type[0];
                            c.push_back(x);
                        }
                        //add to node n_2
                        if(n_2->update(rd, -1, R, round, c) == 0)
                            delete []rd;
                    }
                    //update A/B
                    for (int k = 0; k < n_2->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = nr.first->type[0]*n_2->ineqgeq[k].norm[j] - ineqleq[i].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = nr.first->type[0]*n_2->ineqgeq[k].coeff[a] - ineqleq[i].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update(rd, 1, R, round, c) == 0)
                            delete []rd;
                    }
                }

                //A/B A + B = -1/2C
                if (nr.first->type[0] < 0 && nr.first->type[1] == 1 && nr.second <= 2)
                {
                    if(nr.second == 1)
                        n_1 = nr.first->node_2;
                    else
                        n_1 = nr.first->node_1;
                    n_2 = nr.first->node_sum;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update C
                    for (int k = 0; k < n_1->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = (ineqleq[i].norm[j] + n_1->ineqleq[k].norm[j])/nr.first->type[0];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = (ineqleq[i].coeff[a] + n_1->ineqleq[k].coeff[a])/nr.first->type[0];
                            c.push_back(x);
                        }
                        if(n_2->update(rd, 1, R, round, c) == 0)
                            delete []rd;
                    }
                    //update A/B
                    for (int k = 0; k < n_2->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = nr.first->type[0]*n_2->ineqleq[k].norm[j] - ineqleq[i].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = nr.first->type[0]*n_2->ineqleq[k].coeff[a] - ineqleq[i].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update(rd, 1, R, round, c) == 0)
                            delete []rd;
                    }
                }

                //C A + B = 1/2C
                if (nr.first->type[0] > 0 && nr.first->type[1] == 1 && nr.second == 3)
                {
                    n_1 = nr.first->node_1;
                    n_2 = nr.first->node_2;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update A
                    for (int k = 0; k < n_1->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = nr.first->type[0]*ineqleq[i].norm[j] - n_1->ineqgeq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = nr.first->type[0]*ineqleq[i].coeff[a] - n_1->ineqgeq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_2->update(rd, -1, R, round, c) == 0)
                            delete []rd;
                    }
                    //update B
                    for (int k = 0; k < n_2->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = nr.first->type[0]*ineqleq[i].norm[j] - n_2->ineqgeq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = nr.first->type[0]*ineqleq[i].coeff[a] - n_2->ineqgeq[k].coeff[a];
                            c.push_back(x);
                        }
                        //if not, added to C
                        if(n_1->update(rd, -1, R, round, c) == 0)
                            delete []rd;
                    }
                }

                //C A + B = -1/2C
                if (nr.first->type[0] < 0 && nr.first->type[1] == 1 && nr.second == 3)
                {
                    n_1 = nr.first->node_1;
                    n_2 = nr.first->node_2;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update B
                    for (int k = 0; k < n_1->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = nr.first->type[0]*ineqleq[i].norm[j] - n_1->ineqleq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = nr.first->type[0]*ineqleq[i].coeff[a] - n_1->ineqleq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_2->update(rd, 1, R, round, c) == 0)
                            delete []rd;
                    }
                    //update A
                    for (int k = 0; k < n_2->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = nr.first->type[0]*ineqleq[i].norm[j] - n_2->ineqleq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = nr.first->type[0]*ineqleq[i].coeff[a] - n_2->ineqleq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update(rd, 1, R, round, c) == 0)
                            delete []rd;
                    }
                }

                //A A - B = 1/2C
                if (nr.first->type[0] > 0 && nr.first->type[1] == -1 && nr.second == 1)
                {
                    n_1 = nr.first->node_2;
                    n_2 = nr.first->node_sum;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update C
                    for (int k = 0; k < n_1->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = (ineqleq[i].norm[j] - n_1->ineqgeq[k].norm[j])/nr.first->type[0];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = (ineqleq[i].coeff[a] - n_1->ineqgeq[k].coeff[a])/nr.first->type[0];
                            c.push_back(x);
                        }
                        if (n_2->update(rd, -1, R, round, c) == 0)
                            delete[]rd;
                    }
                    //update B
                    for (int k = 0; k < n_2->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = ineqleq[i].norm[j] - nr.first->type[0]*n_2->ineqgeq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = ineqleq[i].coeff[a] - nr.first->type[0]*n_2->ineqgeq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update(rd, -1, R, round, c) == 0)
                            delete []rd;
                    }
                }

                //A A - B = -1/2C
                if (nr.first->type[0] < 0 && nr.first->type[1] == -1 && nr.second == 1)
                {
                    n_1 = nr.first->node_2;
                    n_2 = nr.first->node_sum;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update C
                    for (int k = 0; k < n_1->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = (ineqleq[i].norm[j] - n_1->ineqgeq[k].norm[j])/nr.first->type[0];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = (ineqleq[i].coeff[a] - n_1->ineqgeq[k].coeff[a])/nr.first->type[0];
                            c.push_back(x);
                        }
                        if(n_2->update(rd, 1, R, round, c) == 0)
                            delete []rd;
                    }
                    //update B
                    for (int k = 0; k < n_2->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = ineqleq[i].norm[j] - nr.first->type[0]*n_2->ineqleq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = ineqleq[i].coeff[a] - nr.first->type[0]*n_2->ineqleq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update(rd, -1, R, round, c) == 0)
                            delete []rd;
                    }
                }

                //B A - B = 1/2C
                if (nr.first->type[0] > 0 && nr.first->type[1] == -1 && nr.second == 2)
                {
                    n_1 = nr.first->node_1;
                    n_2 = nr.first->node_sum;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update C
                    for (int k = 0; k < n_1->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = (n_1->ineqgeq[k].norm[j] - ineqleq[i].norm[j])/nr.first->type[0];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = (n_1->ineqgeq[k].coeff[a] - ineqleq[i].coeff[a])/nr.first->type[0];
                            c.push_back(x);
                        }
                        if(n_2->update(rd, 1, R, round, c) == 0)
                            delete []rd;
                    }
                    //update A
                    for (int k = 0; k < n_2->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = ineqleq[i].norm[j] + nr.first->type[0]*n_2->ineqleq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = ineqleq[i].coeff[a] + nr.first->type[0]*n_2->ineqleq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update(rd, -1, R, round, c) == 0)
                            delete []rd;
                    }
                }

                //B A - B = -1/2C
                if (nr.first->type[0] < 0 && nr.first->type[1] == -1 && nr.second == 2)
                {
                    n_1 = nr.first->node_1;
                    n_2 = nr.first->node_sum;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update C
                    for (int k = 0; k < n_1->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = (n_1->ineqgeq[k].norm[j] - ineqleq[i].norm[j])/nr.first->type[0];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = (n_1->ineqgeq[k].coeff[a] - ineqleq[i].coeff[a])/nr.first->type[0];
                            c.push_back(x);
                        }
                        if(n_2->update(rd, -1, R, round, c) == 0)
                            delete []rd;
                    }
                    //update A
                    for (int k = 0; k < n_2->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = ineqleq[i].norm[j] + nr.first->type[0]*n_2->ineqgeq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = ineqleq[i].coeff[a] + nr.first->type[0]*n_2->ineqgeq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update(rd, -1, R, round, c) == 0)
                            delete []rd;
                    }
                }

                //C A - B = 1/2C
                if (nr.first->type[0] > 0 && nr.first->type[1] == -1 && nr.second == 3)
                {
                    n_1 = nr.first->node_1;
                    n_2 = nr.first->node_2;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update B
                    for (int k = 0; k < n_1->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = n_1->ineqgeq[k].norm[j] - ineqleq[i].norm[j]*nr.first->type[0];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = n_1->ineqgeq[k].coeff[a] - ineqleq[i].coeff[a]*nr.first->type[0];
                            c.push_back(x);
                        }
                        if(n_2->update(rd, 1, R, round, c) == 0)
                            delete []rd;
                    }
                    //update A
                    for (int k = 0; k < n_2->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = nr.first->type[0]*ineqleq[i].norm[j] + n_2->ineqleq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = nr.first->type[0]*ineqleq[i].coeff[a] + n_2->ineqleq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update(rd, -1, R, round, c) == 0)
                            delete []rd;
                    }
                }

                //C A - B = -1/2C
                if (nr.first->type[0] < 0 && nr.first->type[1] == -1 && nr.second == 3)
                {
                    n_1 = nr.first->node_1;
                    n_2 = nr.first->node_2;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update B
                    for (int k = 0; k < n_1->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = -nr.first->type[0]*ineqleq[i].norm[j] + n_1->ineqleq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = -nr.first->type[0]*ineqleq[i].coeff[a] + n_1->ineqleq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_2->update(rd, -1, R, round, c) == 0)
                            delete []rd;
                    }
                    //update A
                    for (int k = 0; k < n_2->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = n_2->ineqgeq[k].norm[j] + nr.first->type[0]*ineqleq[i].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = n_2->ineqgeq[k].coeff[a] + nr.first->type[0]*ineqleq[i].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update(rd, 1, R, round, c) == 0)
                            delete []rd;
                    }
                }

            }
        }
    }
    else if(direction == 1)//>=
    {
        int size = ineqgeq.size();
        for(int i = 0; i < size; ++i)
        {
            if(ineqgeq[i].version >= round - 1)
            {
                //A/B A + B = 1/2C
                if (nr.first->type[0] > 0 && nr.first->type[1] == 1 && nr.second <= 2)
                {
                    if(nr.second == 1)
                        n_1 = nr.first->node_2;
                    else
                        n_1 = nr.first->node_1;
                    n_2 = nr.first->node_sum;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update C
                    for (int k = 0; k < n_1->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = (ineqgeq[i].norm[j] + n_1->ineqgeq[k].norm[j])/nr.first->type[0];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = (ineqgeq[i].coeff[a] + n_1->ineqgeq[k].coeff[a])/nr.first->type[0];
                            c.push_back(x);
                        }
                        if(n_2->update(rd, 1, R, round, c) == 0)
                            delete []rd;
                    }
                    //update A/B
                    for (int k = 0; k < n_2->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = nr.first->type[0]*n_2->ineqleq[k].norm[j] - ineqgeq[i].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = nr.first->type[0]*n_2->ineqleq[k].coeff[a] - ineqgeq[i].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update(rd, -1, R, round, c) == 0)
                            delete []rd;
                    }
                }

                //A/B A + B = -1/2C
                if (nr.first->type[0] < 0 && nr.first->type[1] == 1 && nr.second <= 2)
                {
                    if(nr.second == 1)
                        n_1 = nr.first->node_2;
                    else
                        n_1 = nr.first->node_1;
                    n_2 = nr.first->node_sum;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update C
                    for (int k = 0; k < n_1->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = (ineqgeq[i].norm[j] + n_1->ineqgeq[k].norm[j])/nr.first->type[0];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = (ineqgeq[i].coeff[a] + n_1->ineqgeq[k].coeff[a])/nr.first->type[0];
                            c.push_back(x);
                        }
                        if(n_2->update(rd, -1, R, round, c) == 0)
                            delete []rd;
                    }
                    //update A/B
                    for (int k = 0; k < n_2->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = nr.first->type[0]*n_2->ineqgeq[k].norm[j] - ineqgeq[i].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = nr.first->type[0]*n_2->ineqgeq[k].coeff[a] - ineqgeq[i].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update(rd, -1, R, round, c) == 0)
                            delete []rd;
                    }
                }

                //C A + B = 1/2C
                if (nr.first->type[0] > 0 && nr.first->type[1] == 1 && nr.second == 3)
                {
                    n_1 = nr.first->node_1;
                    n_2 = nr.first->node_2;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update A
                    for (int k = 0; k < n_1->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = nr.first->type[0]*ineqgeq[i].norm[j] - n_1->ineqleq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = nr.first->type[0]*ineqgeq[i].coeff[a] - n_1->ineqleq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_2->update(rd, 1, R, round, c) == 0)
                            delete []rd;
                    }
                    //update B
                    for (int k = 0; k < n_2->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = nr.first->type[0]*ineqgeq[i].norm[j] - n_2->ineqleq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = nr.first->type[0]*ineqgeq[i].coeff[a] - n_2->ineqleq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update(rd, 1, R, round, c) == 0)
                            delete []rd;
                    }
                }

                //C A + B = -1/2C
                if (nr.first->type[0] < 0 && nr.first->type[1] == 1 && nr.second == 3)
                {
                    n_1 = nr.first->node_1;
                    n_2 = nr.first->node_2;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update A
                    for (int k = 0; k < n_1->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = nr.first->type[0]*ineqgeq[i].norm[j] - n_1->ineqgeq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = nr.first->type[0]*ineqgeq[i].coeff[a] - n_1->ineqgeq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_2->update(rd, -1, R, round, c) == 0)
                            delete []rd;
                    }
                    //update B
                    for (int k = 0; k < n_2->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = nr.first->type[0]*ineqgeq[i].norm[j] - n_2->ineqgeq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = nr.first->type[0]*ineqgeq[i].coeff[a] - n_2->ineqgeq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update(rd, -1, R, round, c) == 0)
                            delete []rd;
                    }
                }

                //A A - B = 1/2C
                if (nr.first->type[0] > 0 && nr.first->type[1] == -1 && nr.second == 1)
                {
                    n_1 = nr.first->node_2;
                    n_2 = nr.first->node_sum;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update C
                    for (int k = 0; k < n_1->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = (ineqgeq[i].norm[j] - n_1->ineqleq[k].norm[j])/nr.first->type[0];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = (ineqgeq[i].coeff[a] - n_1->ineqleq[k].coeff[a])/nr.first->type[0];
                            c.push_back(x);
                        }
                        if(n_2->update(rd, 1, R, round, c) == 0)
                            delete []rd;
                    }
                    //update B
                    for (int k = 0; k < n_2->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = ineqgeq[i].norm[j] - nr.first->type[0]*n_2->ineqleq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = ineqgeq[i].coeff[a] - nr.first->type[0]*n_2->ineqleq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update(rd, 1, R, round, c) == 0)
                            delete []rd;
                    }
                }

                //A A - B = -1/2C
                if (nr.first->type[0] < 0 && nr.first->type[1] == -1 && nr.second == 1)
                {
                    n_1 = nr.first->node_2;
                    n_2 = nr.first->node_sum;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update C
                    for (int k = 0; k < n_1->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = (ineqgeq[i].norm[j] - n_1->ineqleq[k].norm[j])/nr.first->type[0];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = (ineqgeq[i].coeff[a] - n_1->ineqleq[k].coeff[a])/nr.first->type[0];
                            c.push_back(x);
                        }
                        if(n_2->update(rd, -1, R, round, c) == 0)
                            delete []rd;
                    }
                    //update B
                    for (int k = 0; k < n_2->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = ineqgeq[i].norm[j] - nr.first->type[0]*n_2->ineqgeq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = ineqgeq[i].coeff[a] - nr.first->type[0]*n_2->ineqgeq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update(rd, 1, R, round, c) == 0)
                            delete []rd;
                    }
                }

                //B A - B = 1/2C
                if (nr.first->type[0] > 0 && nr.first->type[1] == -1 && nr.second == 2)
                {
                    n_1 = nr.first->node_1;
                    n_2 = nr.first->node_sum;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update C
                    for (int k = 0; k < n_1->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = (n_1->ineqleq[k].norm[j] - ineqgeq[i].norm[j])/nr.first->type[0];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = (n_1->ineqleq[k].coeff[a] - ineqgeq[i].coeff[a])/nr.first->type[0];
                            c.push_back(x);
                        }
                        if(n_2->update(rd, -1, R, round, c) == 0)
                            delete []rd;
                    }
                    //update A
                    for (int k = 0; k < n_2->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = ineqgeq[i].norm[j] + nr.first->type[0]*n_2->ineqgeq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = ineqgeq[i].coeff[a] + nr.first->type[0]*n_2->ineqgeq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update(rd, 1, R, round, c) == 0)
                            delete []rd;
                    }
                }

                //B A - B = -1/2C
                if (nr.first->type[0] < 0 && nr.first->type[1] == -1 && nr.second == 2)
                {
                    n_1 = nr.first->node_1;
                    n_2 = nr.first->node_sum;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update C
                    for (int k = 0; k < n_1->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = (n_1->ineqleq[k].norm[j] - ineqgeq[i].norm[j])/nr.first->type[0];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = (n_1->ineqleq[k].coeff[a] - ineqgeq[i].coeff[a])/nr.first->type[0];
                            c.push_back(x);
                        }
                        if(n_2->update(rd, 1, R, round, c) == 0)
                            delete []rd;
                    }
                    //update A
                    for (int k = 0; k < n_2->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = ineqgeq[i].norm[j] + nr.first->type[0]*n_2->ineqleq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = ineqgeq[i].coeff[a] + nr.first->type[0]*n_2->ineqleq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update(rd, 1, R, round, c) == 0)
                            delete []rd;
                    }
                }

                //C A - B = 1/2C
                if (nr.first->type[0] > 0 && nr.first->type[1] == -1 && nr.second == 3)
                {
                    n_1 = nr.first->node_1;
                    n_2 = nr.first->node_2;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update B
                    for (int k = 0; k < n_1->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = n_1->ineqleq[k].norm[j] - ineqgeq[i].norm[j]*nr.first->type[0];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = n_1->ineqleq[k].coeff[a] - ineqgeq[i].coeff[a]*nr.first->type[0];
                            c.push_back(x);
                        }
                        if(n_2->update(rd, -1, R, round, c) == 0)
                            delete []rd;
                    }
                    //update A
                    for (int k = 0; k < n_2->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = nr.first->type[0]*ineqgeq[i].norm[j] + n_2->ineqgeq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = nr.first->type[0]*ineqgeq[i].coeff[a] + n_2->ineqgeq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update(rd, 1, R, round, c) == 0)
                            delete []rd;
                    }
                }

                //C A - B = -1/2C
                if (nr.first->type[0] < 0 && nr.first->type[1] == -1 && nr.second == 3)
                {
                    n_1 = nr.first->node_1;
                    n_2 = nr.first->node_2;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update B
                    for (int k = 0; k < n_1->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = -nr.first->type[0]*ineqgeq[i].norm[j] + n_1->ineqgeq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = -nr.first->type[0]*ineqgeq[i].coeff[a] + n_1->ineqgeq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_2->update(rd, 1, R, round, c) == 0)
                            delete []rd;
                    }
                    //update A
                    for (int k = 0; k < n_2->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = n_2->ineqleq[k].norm[j] + nr.first->type[0]*ineqgeq[i].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = n_2->ineqleq[k].coeff[a] + nr.first->type[0]*ineqgeq[i].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update(rd, -1, R, round, c) == 0)
                            delete []rd;
                    }
                }

            }
        }
    }
}

/**
 * @brief Based on R and the node, update the n1 and n2
 * @param n_1       Node 1
 * @param n_2       Node 1
 * @param direction Which part the current node is updated (-1: ineqleq or 1: ineqgeq)
 * @param R         The utility range extreme points
 */
void r_node::update_round(std::pair<node_relation*, int> nr, int direction, hyperplane_set *R, int round, double u_range)
{
    //initialize
    nr.first->node_1->updated = 0;
    nr.first->node_2->updated = 0;
    nr.first->node_sum->updated = 0;
    updated = direction;
    r_node *n_1, *n_2;
    int dim = R->ext_pts[0]->d;

    //update the node
    if(direction == -1)//<=
    {
        int size = ineqleq.size();
        for(int i = 0; i < size; ++i)
        {
            if(ineqleq[i].version >= round - 1)
            {
                //A/B A + B = 1/2C
                if (nr.first->type[0] > 0 && nr.first->type[1] == 1 && nr.second <= 2)
                {
                    if(nr.second == 1)
                        n_1 = nr.first->node_2;
                    else
                        n_1 = nr.first->node_1;
                    n_2 = nr.first->node_sum;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update C, where A + B = 1/2C
                    for (int k = 0; k < n_1->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = (ineqleq[i].norm[j] + n_1->ineqleq[k].norm[j])/nr.first->type[0];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = (ineqleq[i].coeff[a] + n_1->ineqleq[k].coeff[a])/nr.first->type[0];
                            c.push_back(x);
                        }
                        //add to node n_2
                        if(n_2->update_round(rd, -1, R, round, c, u_range) == 0)
                            delete []rd;
                    }
                    //update A/B
                    for (int k = 0; k < n_2->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = nr.first->type[0]*n_2->ineqgeq[k].norm[j] - ineqleq[i].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = nr.first->type[0]*n_2->ineqgeq[k].coeff[a] - ineqleq[i].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update_round(rd, 1, R, round, c, u_range) == 0)
                            delete []rd;
                    }
                }

                //A/B A + B = -1/2C
                if (nr.first->type[0] < 0 && nr.first->type[1] == 1 && nr.second <= 2)
                {
                    if(nr.second == 1)
                        n_1 = nr.first->node_2;
                    else
                        n_1 = nr.first->node_1;
                    n_2 = nr.first->node_sum;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update C
                    for (int k = 0; k < n_1->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = (ineqleq[i].norm[j] + n_1->ineqleq[k].norm[j])/nr.first->type[0];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = (ineqleq[i].coeff[a] + n_1->ineqleq[k].coeff[a])/nr.first->type[0];
                            c.push_back(x);
                        }
                        if(n_2->update_round(rd, 1, R, round, c, u_range) == 0)
                            delete []rd;
                    }
                    //update A/B
                    for (int k = 0; k < n_2->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = nr.first->type[0]*n_2->ineqleq[k].norm[j] - ineqleq[i].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = nr.first->type[0]*n_2->ineqleq[k].coeff[a] - ineqleq[i].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update_round(rd, 1, R, round, c, u_range) == 0)
                            delete []rd;
                    }
                }

                //C A + B = 1/2C
                if (nr.first->type[0] > 0 && nr.first->type[1] == 1 && nr.second == 3)
                {
                    n_1 = nr.first->node_1;
                    n_2 = nr.first->node_2;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update A
                    for (int k = 0; k < n_1->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = nr.first->type[0]*ineqleq[i].norm[j] - n_1->ineqgeq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = nr.first->type[0]*ineqleq[i].coeff[a] - n_1->ineqgeq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_2->update_round(rd, -1, R, round, c, u_range) == 0)
                            delete []rd;
                    }
                    //update B
                    for (int k = 0; k < n_2->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = nr.first->type[0]*ineqleq[i].norm[j] - n_2->ineqgeq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = nr.first->type[0]*ineqleq[i].coeff[a] - n_2->ineqgeq[k].coeff[a];
                            c.push_back(x);
                        }
                        //if not, added to C
                        if(n_1->update_round(rd, -1, R, round, c, u_range) == 0)
                            delete []rd;
                    }
                }

                //C A + B = -1/2C
                if (nr.first->type[0] < 0 && nr.first->type[1] == 1 && nr.second == 3)
                {
                    n_1 = nr.first->node_1;
                    n_2 = nr.first->node_2;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update B
                    for (int k = 0; k < n_1->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = nr.first->type[0]*ineqleq[i].norm[j] - n_1->ineqleq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = nr.first->type[0]*ineqleq[i].coeff[a] - n_1->ineqleq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_2->update_round(rd, 1, R, round, c, u_range) == 0)
                            delete []rd;
                    }
                    //update A
                    for (int k = 0; k < n_2->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = nr.first->type[0]*ineqleq[i].norm[j] - n_2->ineqleq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = nr.first->type[0]*ineqleq[i].coeff[a] - n_2->ineqleq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update_round(rd, 1, R, round, c, u_range) == 0)
                            delete []rd;
                    }
                }

                //A A - B = 1/2C
                if (nr.first->type[0] > 0 && nr.first->type[1] == -1 && nr.second == 1)
                {
                    n_1 = nr.first->node_2;
                    n_2 = nr.first->node_sum;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update C
                    for (int k = 0; k < n_1->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = (ineqleq[i].norm[j] - n_1->ineqgeq[k].norm[j])/nr.first->type[0];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = (ineqleq[i].coeff[a] - n_1->ineqgeq[k].coeff[a])/nr.first->type[0];
                            c.push_back(x);
                        }
                        if (n_2->update_round(rd, -1, R, round, c, u_range) == 0)
                            delete[]rd;
                    }
                    //update B
                    for (int k = 0; k < n_2->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = ineqleq[i].norm[j] - nr.first->type[0]*n_2->ineqgeq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = ineqleq[i].coeff[a] - nr.first->type[0]*n_2->ineqgeq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update_round(rd, -1, R, round, c, u_range) == 0)
                            delete []rd;
                    }
                }

                //A A - B = -1/2C
                if (nr.first->type[0] < 0 && nr.first->type[1] == -1 && nr.second == 1)
                {
                    n_1 = nr.first->node_2;
                    n_2 = nr.first->node_sum;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update C
                    for (int k = 0; k < n_1->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = (ineqleq[i].norm[j] - n_1->ineqgeq[k].norm[j])/nr.first->type[0];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = (ineqleq[i].coeff[a] - n_1->ineqgeq[k].coeff[a])/nr.first->type[0];
                            c.push_back(x);
                        }
                        if(n_2->update_round(rd, 1, R, round, c, u_range) == 0)
                            delete []rd;
                    }
                    //update B
                    for (int k = 0; k < n_2->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = ineqleq[i].norm[j] - nr.first->type[0]*n_2->ineqleq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = ineqleq[i].coeff[a] - nr.first->type[0]*n_2->ineqleq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update_round(rd, -1, R, round, c, u_range) == 0)
                            delete []rd;
                    }
                }

                //B A - B = 1/2C
                if (nr.first->type[0] > 0 && nr.first->type[1] == -1 && nr.second == 2)
                {
                    n_1 = nr.first->node_1;
                    n_2 = nr.first->node_sum;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update C
                    for (int k = 0; k < n_1->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = (n_1->ineqgeq[k].norm[j] - ineqleq[i].norm[j])/nr.first->type[0];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = (n_1->ineqgeq[k].coeff[a] - ineqleq[i].coeff[a])/nr.first->type[0];
                            c.push_back(x);
                        }
                        if(n_2->update_round(rd, 1, R, round, c, u_range) == 0)
                            delete []rd;
                    }
                    //update A
                    for (int k = 0; k < n_2->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = ineqleq[i].norm[j] + nr.first->type[0]*n_2->ineqleq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = ineqleq[i].coeff[a] + nr.first->type[0]*n_2->ineqleq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update_round(rd, -1, R, round, c, u_range) == 0)
                            delete []rd;
                    }
                }

                //B A - B = -1/2C
                if (nr.first->type[0] < 0 && nr.first->type[1] == -1 && nr.second == 2)
                {
                    n_1 = nr.first->node_1;
                    n_2 = nr.first->node_sum;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update C
                    for (int k = 0; k < n_1->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = (n_1->ineqgeq[k].norm[j] - ineqleq[i].norm[j])/nr.first->type[0];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = (n_1->ineqgeq[k].coeff[a] - ineqleq[i].coeff[a])/nr.first->type[0];
                            c.push_back(x);
                        }
                        if(n_2->update_round(rd, -1, R, round, c, u_range) == 0)
                            delete []rd;
                    }
                    //update A
                    for (int k = 0; k < n_2->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = ineqleq[i].norm[j] + nr.first->type[0]*n_2->ineqgeq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = ineqleq[i].coeff[a] + nr.first->type[0]*n_2->ineqgeq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update_round(rd, -1, R, round, c, u_range) == 0)
                            delete []rd;
                    }
                }

                //C A - B = 1/2C
                if (nr.first->type[0] > 0 && nr.first->type[1] == -1 && nr.second == 3)
                {
                    n_1 = nr.first->node_1;
                    n_2 = nr.first->node_2;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update B
                    for (int k = 0; k < n_1->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = n_1->ineqgeq[k].norm[j] - ineqleq[i].norm[j]*nr.first->type[0];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = n_1->ineqgeq[k].coeff[a] - ineqleq[i].coeff[a]*nr.first->type[0];
                            c.push_back(x);
                        }
                        if(n_2->update_round(rd, 1, R, round, c, u_range) == 0)
                            delete []rd;
                    }
                    //update A
                    for (int k = 0; k < n_2->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = nr.first->type[0]*ineqleq[i].norm[j] + n_2->ineqleq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = nr.first->type[0]*ineqleq[i].coeff[a] + n_2->ineqleq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update_round(rd, -1, R, round, c, u_range) == 0)
                            delete []rd;
                    }
                }

                //C A - B = -1/2C
                if (nr.first->type[0] < 0 && nr.first->type[1] == -1 && nr.second == 3)
                {
                    n_1 = nr.first->node_1;
                    n_2 = nr.first->node_2;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update B
                    for (int k = 0; k < n_1->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = -nr.first->type[0]*ineqleq[i].norm[j] + n_1->ineqleq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = -nr.first->type[0]*ineqleq[i].coeff[a] + n_1->ineqleq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_2->update_round(rd, -1, R, round, c, u_range) == 0)
                            delete []rd;
                    }
                    //update A
                    for (int k = 0; k < n_2->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = n_2->ineqgeq[k].norm[j] + nr.first->type[0]*ineqleq[i].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = n_2->ineqgeq[k].coeff[a] + nr.first->type[0]*ineqleq[i].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update_round(rd, 1, R, round, c, u_range) == 0)
                            delete []rd;
                    }
                }

            }
        }
    }
    else if(direction == 1)//>=
    {
        int size = ineqgeq.size();
        for(int i = 0; i < size; ++i)
        {
            if(ineqgeq[i].version >= round - 1)
            {
                //A/B A + B = 1/2C
                if (nr.first->type[0] > 0 && nr.first->type[1] == 1 && nr.second <= 2)
                {
                    if(nr.second == 1)
                        n_1 = nr.first->node_2;
                    else
                        n_1 = nr.first->node_1;
                    n_2 = nr.first->node_sum;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update C
                    for (int k = 0; k < n_1->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = (ineqgeq[i].norm[j] + n_1->ineqgeq[k].norm[j])/nr.first->type[0];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = (ineqgeq[i].coeff[a] + n_1->ineqgeq[k].coeff[a])/nr.first->type[0];
                            c.push_back(x);
                        }
                        if(n_2->update_round(rd, 1, R, round, c, u_range) == 0)
                            delete []rd;
                    }
                    //update A/B
                    for (int k = 0; k < n_2->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = nr.first->type[0]*n_2->ineqleq[k].norm[j] - ineqgeq[i].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = nr.first->type[0]*n_2->ineqleq[k].coeff[a] - ineqgeq[i].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update_round(rd, -1, R, round, c, u_range) == 0)
                            delete []rd;
                    }
                }

                //A/B A + B = -1/2C
                if (nr.first->type[0] < 0 && nr.first->type[1] == 1 && nr.second <= 2)
                {
                    if(nr.second == 1)
                        n_1 = nr.first->node_2;
                    else
                        n_1 = nr.first->node_1;
                    n_2 = nr.first->node_sum;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update C
                    for (int k = 0; k < n_1->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = (ineqgeq[i].norm[j] + n_1->ineqgeq[k].norm[j])/nr.first->type[0];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = (ineqgeq[i].coeff[a] + n_1->ineqgeq[k].coeff[a])/nr.first->type[0];
                            c.push_back(x);
                        }
                        if(n_2->update_round(rd, -1, R, round, c, u_range) == 0)
                            delete []rd;
                    }
                    //update A/B
                    for (int k = 0; k < n_2->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = nr.first->type[0]*n_2->ineqgeq[k].norm[j] - ineqgeq[i].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = nr.first->type[0]*n_2->ineqgeq[k].coeff[a] - ineqgeq[i].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update_round(rd, -1, R, round, c, u_range) == 0)
                            delete []rd;
                    }
                }

                //C A + B = 1/2C
                if (nr.first->type[0] > 0 && nr.first->type[1] == 1 && nr.second == 3)
                {
                    n_1 = nr.first->node_1;
                    n_2 = nr.first->node_2;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update A
                    for (int k = 0; k < n_1->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = nr.first->type[0]*ineqgeq[i].norm[j] - n_1->ineqleq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = nr.first->type[0]*ineqgeq[i].coeff[a] - n_1->ineqleq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_2->update_round(rd, 1, R, round, c, u_range) == 0)
                            delete []rd;
                    }
                    //update B
                    for (int k = 0; k < n_2->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = nr.first->type[0]*ineqgeq[i].norm[j] - n_2->ineqleq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = nr.first->type[0]*ineqgeq[i].coeff[a] - n_2->ineqleq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update_round(rd, 1, R, round, c, u_range) == 0)
                            delete []rd;
                    }
                }

                //C A + B = -1/2C
                if (nr.first->type[0] < 0 && nr.first->type[1] == 1 && nr.second == 3)
                {
                    n_1 = nr.first->node_1;
                    n_2 = nr.first->node_2;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update A
                    for (int k = 0; k < n_1->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = nr.first->type[0]*ineqgeq[i].norm[j] - n_1->ineqgeq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = nr.first->type[0]*ineqgeq[i].coeff[a] - n_1->ineqgeq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_2->update_round(rd, -1, R, round, c, u_range) == 0)
                            delete []rd;
                    }
                    //update B
                    for (int k = 0; k < n_2->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = nr.first->type[0]*ineqgeq[i].norm[j] - n_2->ineqgeq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = nr.first->type[0]*ineqgeq[i].coeff[a] - n_2->ineqgeq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update_round(rd, -1, R, round, c, u_range) == 0)
                            delete []rd;
                    }
                }

                //A A - B = 1/2C
                if (nr.first->type[0] > 0 && nr.first->type[1] == -1 && nr.second == 1)
                {
                    n_1 = nr.first->node_2;
                    n_2 = nr.first->node_sum;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update C
                    for (int k = 0; k < n_1->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = (ineqgeq[i].norm[j] - n_1->ineqleq[k].norm[j])/nr.first->type[0];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = (ineqgeq[i].coeff[a] - n_1->ineqleq[k].coeff[a])/nr.first->type[0];
                            c.push_back(x);
                        }
                        if(n_2->update_round(rd, 1, R, round, c, u_range) == 0)
                            delete []rd;
                    }
                    //update B
                    for (int k = 0; k < n_2->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = ineqgeq[i].norm[j] - nr.first->type[0]*n_2->ineqleq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = ineqgeq[i].coeff[a] - nr.first->type[0]*n_2->ineqleq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update_round(rd, 1, R, round, c, u_range) == 0)
                            delete []rd;
                    }
                }

                //A A - B = -1/2C
                if (nr.first->type[0] < 0 && nr.first->type[1] == -1 && nr.second == 1)
                {
                    n_1 = nr.first->node_2;
                    n_2 = nr.first->node_sum;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update C
                    for (int k = 0; k < n_1->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = (ineqgeq[i].norm[j] - n_1->ineqleq[k].norm[j])/nr.first->type[0];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = (ineqgeq[i].coeff[a] - n_1->ineqleq[k].coeff[a])/nr.first->type[0];
                            c.push_back(x);
                        }
                        if(n_2->update_round(rd, -1, R, round, c, u_range) == 0)
                            delete []rd;
                    }
                    //update B
                    for (int k = 0; k < n_2->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = ineqgeq[i].norm[j] - nr.first->type[0]*n_2->ineqgeq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = ineqgeq[i].coeff[a] - nr.first->type[0]*n_2->ineqgeq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update_round(rd, 1, R, round, c, u_range) == 0)
                            delete []rd;
                    }
                }

                //B A - B = 1/2C
                if (nr.first->type[0] > 0 && nr.first->type[1] == -1 && nr.second == 2)
                {
                    n_1 = nr.first->node_1;
                    n_2 = nr.first->node_sum;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update C
                    for (int k = 0; k < n_1->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = (n_1->ineqleq[k].norm[j] - ineqgeq[i].norm[j])/nr.first->type[0];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = (n_1->ineqleq[k].coeff[a] - ineqgeq[i].coeff[a])/nr.first->type[0];
                            c.push_back(x);
                        }
                        if(n_2->update_round(rd, -1, R, round, c, u_range) == 0)
                            delete []rd;
                    }
                    //update A
                    for (int k = 0; k < n_2->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = ineqgeq[i].norm[j] + nr.first->type[0]*n_2->ineqgeq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = ineqgeq[i].coeff[a] + nr.first->type[0]*n_2->ineqgeq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update_round(rd, 1, R, round, c, u_range) == 0)
                            delete []rd;
                    }
                }

                //B A - B = -1/2C
                if (nr.first->type[0] < 0 && nr.first->type[1] == -1 && nr.second == 2)
                {
                    n_1 = nr.first->node_1;
                    n_2 = nr.first->node_sum;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update C
                    for (int k = 0; k < n_1->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = (n_1->ineqleq[k].norm[j] - ineqgeq[i].norm[j])/nr.first->type[0];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = (n_1->ineqleq[k].coeff[a] - ineqgeq[i].coeff[a])/nr.first->type[0];
                            c.push_back(x);
                        }
                        if(n_2->update_round(rd, 1, R, round, c, u_range) == 0)
                            delete []rd;
                    }
                    //update A
                    for (int k = 0; k < n_2->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = ineqgeq[i].norm[j] + nr.first->type[0]*n_2->ineqleq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = ineqgeq[i].coeff[a] + nr.first->type[0]*n_2->ineqleq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update_round(rd, 1, R, round, c, u_range) == 0)
                            delete []rd;
                    }
                }

                //C A - B = 1/2C
                if (nr.first->type[0] > 0 && nr.first->type[1] == -1 && nr.second == 3)
                {
                    n_1 = nr.first->node_1;
                    n_2 = nr.first->node_2;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update B
                    for (int k = 0; k < n_1->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = n_1->ineqleq[k].norm[j] - ineqgeq[i].norm[j]*nr.first->type[0];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = n_1->ineqleq[k].coeff[a] - ineqgeq[i].coeff[a]*nr.first->type[0];
                            c.push_back(x);
                        }
                        if(n_2->update_round(rd, -1, R, round, c, u_range) == 0)
                            delete []rd;
                    }
                    //update A
                    for (int k = 0; k < n_2->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = nr.first->type[0]*ineqgeq[i].norm[j] + n_2->ineqgeq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = nr.first->type[0]*ineqgeq[i].coeff[a] + n_2->ineqgeq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update_round(rd, 1, R, round, c, u_range) == 0)
                            delete []rd;
                    }
                }

                //C A - B = -1/2C
                if (nr.first->type[0] < 0 && nr.first->type[1] == -1 && nr.second == 3)
                {
                    n_1 = nr.first->node_1;
                    n_2 = nr.first->node_2;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update B
                    for (int k = 0; k < n_1->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = -nr.first->type[0]*ineqgeq[i].norm[j] + n_1->ineqgeq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = -nr.first->type[0]*ineqgeq[i].coeff[a] + n_1->ineqgeq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_2->update_round(rd, 1, R, round, c, u_range) == 0)
                            delete []rd;
                    }
                    //update A
                    for (int k = 0; k < n_2->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = n_2->ineqleq[k].norm[j] + nr.first->type[0]*ineqgeq[i].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = n_2->ineqleq[k].coeff[a] + nr.first->type[0]*ineqgeq[i].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update_round(rd, -1, R, round, c, u_range) == 0)
                            delete []rd;
                    }
                }

            }
        }
    }
}


/**
 * @brief Based on R and the node, update the n1 and n2
 * @param n_1       Node 1
 * @param n_2       Node 1
 * @param direction Which part the current node is updated (-1: ineqleq or 1: ineqgeq)
 * @param R         The utility range extreme points
 */
void r_node::update_threshold(std::pair<node_relation*, int> nr, int direction, hyperplane_set *R, int round, double u_range, double THRESHOLD)
{
    //initialize
    nr.first->node_1->updated = 0;
    nr.first->node_2->updated = 0;
    nr.first->node_sum->updated = 0;
    updated = direction;
    r_node *n_1, *n_2;
    int dim = R->ext_pts[0]->d;

    //update the node
    if(direction == -1)//<=
    {
        int size = ineqleq.size();
        for(int i = 0; i < size; ++i)
        {
            if(ineqleq[i].version >= round - 1)
            {
                //A/B A + B = 1/2C
                if (nr.first->type[0] > 0 && nr.first->type[1] == 1 && nr.second <= 2)
                {
                    if(nr.second == 1)
                        n_1 = nr.first->node_2;
                    else
                        n_1 = nr.first->node_1;
                    n_2 = nr.first->node_sum;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update C, where A + B = 1/2C
                    for (int k = 0; k < n_1->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = (ineqleq[i].norm[j] + n_1->ineqleq[k].norm[j])/nr.first->type[0];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = (ineqleq[i].coeff[a] + n_1->ineqleq[k].coeff[a])/nr.first->type[0];
                            c.push_back(x);
                        }
                        //add to node n_2
                        if(n_2->update_threshold(rd, -1, R, round, c, u_range, THRESHOLD) == 0)
                            delete []rd;
                    }
                    //update A/B
                    for (int k = 0; k < n_2->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = nr.first->type[0]*n_2->ineqgeq[k].norm[j] - ineqleq[i].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = nr.first->type[0]*n_2->ineqgeq[k].coeff[a] - ineqleq[i].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update_threshold(rd, 1, R, round, c, u_range, THRESHOLD) == 0)
                            delete []rd;
                    }
                }

                //A/B A + B = -1/2C
                if (nr.first->type[0] < 0 && nr.first->type[1] == 1 && nr.second <= 2)
                {
                    if(nr.second == 1)
                        n_1 = nr.first->node_2;
                    else
                        n_1 = nr.first->node_1;
                    n_2 = nr.first->node_sum;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update C
                    for (int k = 0; k < n_1->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = (ineqleq[i].norm[j] + n_1->ineqleq[k].norm[j])/nr.first->type[0];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = (ineqleq[i].coeff[a] + n_1->ineqleq[k].coeff[a])/nr.first->type[0];
                            c.push_back(x);
                        }
                        if(n_2->update_threshold(rd, 1, R, round, c, u_range, THRESHOLD) == 0)
                            delete []rd;
                    }
                    //update A/B
                    for (int k = 0; k < n_2->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = nr.first->type[0]*n_2->ineqleq[k].norm[j] - ineqleq[i].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = nr.first->type[0]*n_2->ineqleq[k].coeff[a] - ineqleq[i].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update_threshold(rd, 1, R, round, c, u_range, THRESHOLD) == 0)
                            delete []rd;
                    }
                }

                //C A + B = 1/2C
                if (nr.first->type[0] > 0 && nr.first->type[1] == 1 && nr.second == 3)
                {
                    n_1 = nr.first->node_1;
                    n_2 = nr.first->node_2;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update A
                    for (int k = 0; k < n_1->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = nr.first->type[0]*ineqleq[i].norm[j] - n_1->ineqgeq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = nr.first->type[0]*ineqleq[i].coeff[a] - n_1->ineqgeq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_2->update_threshold(rd, -1, R, round, c, u_range, THRESHOLD) == 0)
                            delete []rd;
                    }
                    //update B
                    for (int k = 0; k < n_2->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = nr.first->type[0]*ineqleq[i].norm[j] - n_2->ineqgeq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = nr.first->type[0]*ineqleq[i].coeff[a] - n_2->ineqgeq[k].coeff[a];
                            c.push_back(x);
                        }
                        //if not, added to C
                        if(n_1->update_threshold(rd, -1, R, round, c, u_range, THRESHOLD) == 0)
                            delete []rd;
                    }
                }

                //C A + B = -1/2C
                if (nr.first->type[0] < 0 && nr.first->type[1] == 1 && nr.second == 3)
                {
                    n_1 = nr.first->node_1;
                    n_2 = nr.first->node_2;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update B
                    for (int k = 0; k < n_1->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = nr.first->type[0]*ineqleq[i].norm[j] - n_1->ineqleq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = nr.first->type[0]*ineqleq[i].coeff[a] - n_1->ineqleq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_2->update_threshold(rd, 1, R, round, c, u_range, THRESHOLD) == 0)
                            delete []rd;
                    }
                    //update A
                    for (int k = 0; k < n_2->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = nr.first->type[0]*ineqleq[i].norm[j] - n_2->ineqleq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = nr.first->type[0]*ineqleq[i].coeff[a] - n_2->ineqleq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update_threshold(rd, 1, R, round, c, u_range, THRESHOLD) == 0)
                            delete []rd;
                    }
                }

                //A A - B = 1/2C
                if (nr.first->type[0] > 0 && nr.first->type[1] == -1 && nr.second == 1)
                {
                    n_1 = nr.first->node_2;
                    n_2 = nr.first->node_sum;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update C
                    for (int k = 0; k < n_1->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = (ineqleq[i].norm[j] - n_1->ineqgeq[k].norm[j])/nr.first->type[0];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = (ineqleq[i].coeff[a] - n_1->ineqgeq[k].coeff[a])/nr.first->type[0];
                            c.push_back(x);
                        }
                        if (n_2->update_threshold(rd, -1, R, round, c, u_range, THRESHOLD) == 0)
                            delete[]rd;
                    }
                    //update B
                    for (int k = 0; k < n_2->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = ineqleq[i].norm[j] - nr.first->type[0]*n_2->ineqgeq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = ineqleq[i].coeff[a] - nr.first->type[0]*n_2->ineqgeq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update_threshold(rd, -1, R, round, c, u_range, THRESHOLD) == 0)
                            delete []rd;
                    }
                }

                //A A - B = -1/2C
                if (nr.first->type[0] < 0 && nr.first->type[1] == -1 && nr.second == 1)
                {
                    n_1 = nr.first->node_2;
                    n_2 = nr.first->node_sum;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update C
                    for (int k = 0; k < n_1->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = (ineqleq[i].norm[j] - n_1->ineqgeq[k].norm[j])/nr.first->type[0];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = (ineqleq[i].coeff[a] - n_1->ineqgeq[k].coeff[a])/nr.first->type[0];
                            c.push_back(x);
                        }
                        if(n_2->update_threshold(rd, 1, R, round, c, u_range, THRESHOLD) == 0)
                            delete []rd;
                    }
                    //update B
                    for (int k = 0; k < n_2->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = ineqleq[i].norm[j] - nr.first->type[0]*n_2->ineqleq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = ineqleq[i].coeff[a] - nr.first->type[0]*n_2->ineqleq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update_threshold(rd, -1, R, round, c, u_range, THRESHOLD) == 0)
                            delete []rd;
                    }
                }

                //B A - B = 1/2C
                if (nr.first->type[0] > 0 && nr.first->type[1] == -1 && nr.second == 2)
                {
                    n_1 = nr.first->node_1;
                    n_2 = nr.first->node_sum;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update C
                    for (int k = 0; k < n_1->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = (n_1->ineqgeq[k].norm[j] - ineqleq[i].norm[j])/nr.first->type[0];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = (n_1->ineqgeq[k].coeff[a] - ineqleq[i].coeff[a])/nr.first->type[0];
                            c.push_back(x);
                        }
                        if(n_2->update_threshold(rd, 1, R, round, c, u_range, THRESHOLD) == 0)
                            delete []rd;
                    }
                    //update A
                    for (int k = 0; k < n_2->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = ineqleq[i].norm[j] + nr.first->type[0]*n_2->ineqleq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = ineqleq[i].coeff[a] + nr.first->type[0]*n_2->ineqleq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update_threshold(rd, -1, R, round, c, u_range, THRESHOLD) == 0)
                            delete []rd;
                    }
                }

                //B A - B = -1/2C
                if (nr.first->type[0] < 0 && nr.first->type[1] == -1 && nr.second == 2)
                {
                    n_1 = nr.first->node_1;
                    n_2 = nr.first->node_sum;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update C
                    for (int k = 0; k < n_1->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = (n_1->ineqgeq[k].norm[j] - ineqleq[i].norm[j])/nr.first->type[0];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = (n_1->ineqgeq[k].coeff[a] - ineqleq[i].coeff[a])/nr.first->type[0];
                            c.push_back(x);
                        }
                        if(n_2->update_threshold(rd, -1, R, round, c, u_range, THRESHOLD) == 0)
                            delete []rd;
                    }
                    //update A
                    for (int k = 0; k < n_2->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = ineqleq[i].norm[j] + nr.first->type[0]*n_2->ineqgeq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = ineqleq[i].coeff[a] + nr.first->type[0]*n_2->ineqgeq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update_threshold(rd, -1, R, round, c, u_range, THRESHOLD) == 0)
                            delete []rd;
                    }
                }

                //C A - B = 1/2C
                if (nr.first->type[0] > 0 && nr.first->type[1] == -1 && nr.second == 3)
                {
                    n_1 = nr.first->node_1;
                    n_2 = nr.first->node_2;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update B
                    for (int k = 0; k < n_1->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = n_1->ineqgeq[k].norm[j] - ineqleq[i].norm[j]*nr.first->type[0];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = n_1->ineqgeq[k].coeff[a] - ineqleq[i].coeff[a]*nr.first->type[0];
                            c.push_back(x);
                        }
                        if(n_2->update_threshold(rd, 1, R, round, c, u_range, THRESHOLD) == 0)
                            delete []rd;
                    }
                    //update A
                    for (int k = 0; k < n_2->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = nr.first->type[0]*ineqleq[i].norm[j] + n_2->ineqleq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = nr.first->type[0]*ineqleq[i].coeff[a] + n_2->ineqleq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update_threshold(rd, -1, R, round, c, u_range, THRESHOLD) == 0)
                            delete []rd;
                    }
                }

                //C A - B = -1/2C
                if (nr.first->type[0] < 0 && nr.first->type[1] == -1 && nr.second == 3)
                {
                    n_1 = nr.first->node_1;
                    n_2 = nr.first->node_2;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update B
                    for (int k = 0; k < n_1->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = -nr.first->type[0]*ineqleq[i].norm[j] + n_1->ineqleq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = -nr.first->type[0]*ineqleq[i].coeff[a] + n_1->ineqleq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_2->update_threshold(rd, -1, R, round, c, u_range, THRESHOLD) == 0)
                            delete []rd;
                    }
                    //update A
                    for (int k = 0; k < n_2->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = n_2->ineqgeq[k].norm[j] + nr.first->type[0]*ineqleq[i].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqleq[i].coeff.size(); a++)
                        {
                            double x = n_2->ineqgeq[k].coeff[a] + nr.first->type[0]*ineqleq[i].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update_threshold(rd, 1, R, round, c, u_range, THRESHOLD) == 0)
                            delete []rd;
                    }
                }

            }
        }
    }
    else if(direction == 1)//>=
    {
        int size = ineqgeq.size();
        for(int i = 0; i < size; ++i)
        {
            if(ineqgeq[i].version >= round - 1)
            {
                //A/B A + B = 1/2C
                if (nr.first->type[0] > 0 && nr.first->type[1] == 1 && nr.second <= 2)
                {
                    if(nr.second == 1)
                        n_1 = nr.first->node_2;
                    else
                        n_1 = nr.first->node_1;
                    n_2 = nr.first->node_sum;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update C
                    for (int k = 0; k < n_1->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = (ineqgeq[i].norm[j] + n_1->ineqgeq[k].norm[j])/nr.first->type[0];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = (ineqgeq[i].coeff[a] + n_1->ineqgeq[k].coeff[a])/nr.first->type[0];
                            c.push_back(x);
                        }
                        if(n_2->update_threshold(rd, 1, R, round, c, u_range, THRESHOLD) == 0)
                            delete []rd;
                    }
                    //update A/B
                    for (int k = 0; k < n_2->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = nr.first->type[0]*n_2->ineqleq[k].norm[j] - ineqgeq[i].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = nr.first->type[0]*n_2->ineqleq[k].coeff[a] - ineqgeq[i].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update_threshold(rd, -1, R, round, c, u_range, THRESHOLD) == 0)
                            delete []rd;
                    }
                }

                //A/B A + B = -1/2C
                if (nr.first->type[0] < 0 && nr.first->type[1] == 1 && nr.second <= 2)
                {
                    if(nr.second == 1)
                        n_1 = nr.first->node_2;
                    else
                        n_1 = nr.first->node_1;
                    n_2 = nr.first->node_sum;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update C
                    for (int k = 0; k < n_1->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = (ineqgeq[i].norm[j] + n_1->ineqgeq[k].norm[j])/nr.first->type[0];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = (ineqgeq[i].coeff[a] + n_1->ineqgeq[k].coeff[a])/nr.first->type[0];
                            c.push_back(x);
                        }
                        if(n_2->update_threshold(rd, -1, R, round, c, u_range, THRESHOLD) == 0)
                            delete []rd;
                    }
                    //update A/B
                    for (int k = 0; k < n_2->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = nr.first->type[0]*n_2->ineqgeq[k].norm[j] - ineqgeq[i].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = nr.first->type[0]*n_2->ineqgeq[k].coeff[a] - ineqgeq[i].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update_threshold(rd, -1, R, round, c, u_range, THRESHOLD) == 0)
                            delete []rd;
                    }
                }

                //C A + B = 1/2C
                if (nr.first->type[0] > 0 && nr.first->type[1] == 1 && nr.second == 3)
                {
                    n_1 = nr.first->node_1;
                    n_2 = nr.first->node_2;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update A
                    for (int k = 0; k < n_1->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = nr.first->type[0]*ineqgeq[i].norm[j] - n_1->ineqleq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = nr.first->type[0]*ineqgeq[i].coeff[a] - n_1->ineqleq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_2->update_threshold(rd, 1, R, round, c, u_range, THRESHOLD) == 0)
                            delete []rd;
                    }
                    //update B
                    for (int k = 0; k < n_2->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = nr.first->type[0]*ineqgeq[i].norm[j] - n_2->ineqleq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = nr.first->type[0]*ineqgeq[i].coeff[a] - n_2->ineqleq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update_threshold(rd, 1, R, round, c, u_range, THRESHOLD) == 0)
                            delete []rd;
                    }
                }

                //C A + B = -1/2C
                if (nr.first->type[0] < 0 && nr.first->type[1] == 1 && nr.second == 3)
                {
                    n_1 = nr.first->node_1;
                    n_2 = nr.first->node_2;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update A
                    for (int k = 0; k < n_1->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = nr.first->type[0]*ineqgeq[i].norm[j] - n_1->ineqgeq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = nr.first->type[0]*ineqgeq[i].coeff[a] - n_1->ineqgeq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_2->update_threshold(rd, -1, R, round, c, u_range, THRESHOLD) == 0)
                            delete []rd;
                    }
                    //update B
                    for (int k = 0; k < n_2->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = nr.first->type[0]*ineqgeq[i].norm[j] - n_2->ineqgeq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = nr.first->type[0]*ineqgeq[i].coeff[a] - n_2->ineqgeq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update_threshold(rd, -1, R, round, c, u_range, THRESHOLD) == 0)
                            delete []rd;
                    }
                }

                //A A - B = 1/2C
                if (nr.first->type[0] > 0 && nr.first->type[1] == -1 && nr.second == 1)
                {
                    n_1 = nr.first->node_2;
                    n_2 = nr.first->node_sum;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update C
                    for (int k = 0; k < n_1->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = (ineqgeq[i].norm[j] - n_1->ineqleq[k].norm[j])/nr.first->type[0];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = (ineqgeq[i].coeff[a] - n_1->ineqleq[k].coeff[a])/nr.first->type[0];
                            c.push_back(x);
                        }
                        if(n_2->update_threshold(rd, 1, R, round, c, u_range, THRESHOLD) == 0)
                            delete []rd;
                    }
                    //update B
                    for (int k = 0; k < n_2->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = ineqgeq[i].norm[j] - nr.first->type[0]*n_2->ineqleq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = ineqgeq[i].coeff[a] - nr.first->type[0]*n_2->ineqleq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update_threshold(rd, 1, R, round, c, u_range, THRESHOLD) == 0)
                            delete []rd;
                    }
                }

                //A A - B = -1/2C
                if (nr.first->type[0] < 0 && nr.first->type[1] == -1 && nr.second == 1)
                {
                    n_1 = nr.first->node_2;
                    n_2 = nr.first->node_sum;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update C
                    for (int k = 0; k < n_1->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = (ineqgeq[i].norm[j] - n_1->ineqleq[k].norm[j])/nr.first->type[0];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = (ineqgeq[i].coeff[a] - n_1->ineqleq[k].coeff[a])/nr.first->type[0];
                            c.push_back(x);
                        }
                        if(n_2->update_threshold(rd, -1, R, round, c, u_range, THRESHOLD) == 0)
                            delete []rd;
                    }
                    //update B
                    for (int k = 0; k < n_2->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = ineqgeq[i].norm[j] - nr.first->type[0]*n_2->ineqgeq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = ineqgeq[i].coeff[a] - nr.first->type[0]*n_2->ineqgeq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update_threshold(rd, 1, R, round, c, u_range, THRESHOLD) == 0)
                            delete []rd;
                    }
                }

                //B A - B = 1/2C
                if (nr.first->type[0] > 0 && nr.first->type[1] == -1 && nr.second == 2)
                {
                    n_1 = nr.first->node_1;
                    n_2 = nr.first->node_sum;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update C
                    for (int k = 0; k < n_1->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = (n_1->ineqleq[k].norm[j] - ineqgeq[i].norm[j])/nr.first->type[0];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = (n_1->ineqleq[k].coeff[a] - ineqgeq[i].coeff[a])/nr.first->type[0];
                            c.push_back(x);
                        }
                        if(n_2->update_threshold(rd, -1, R, round, c, u_range, THRESHOLD) == 0)
                            delete []rd;
                    }
                    //update A
                    for (int k = 0; k < n_2->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = ineqgeq[i].norm[j] + nr.first->type[0]*n_2->ineqgeq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = ineqgeq[i].coeff[a] + nr.first->type[0]*n_2->ineqgeq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update_threshold(rd, 1, R, round, c, u_range, THRESHOLD) == 0)
                            delete []rd;
                    }
                }

                //B A - B = -1/2C
                if (nr.first->type[0] < 0 && nr.first->type[1] == -1 && nr.second == 2)
                {
                    n_1 = nr.first->node_1;
                    n_2 = nr.first->node_sum;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update C
                    for (int k = 0; k < n_1->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = (n_1->ineqleq[k].norm[j] - ineqgeq[i].norm[j])/nr.first->type[0];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = (n_1->ineqleq[k].coeff[a] - ineqgeq[i].coeff[a])/nr.first->type[0];
                            c.push_back(x);
                        }
                        if(n_2->update_threshold(rd, 1, R, round, c, u_range, THRESHOLD) == 0)
                            delete []rd;
                    }
                    //update A
                    for (int k = 0; k < n_2->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = ineqgeq[i].norm[j] + nr.first->type[0]*n_2->ineqleq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = ineqgeq[i].coeff[a] + nr.first->type[0]*n_2->ineqleq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update_threshold(rd, 1, R, round, c, u_range, THRESHOLD) == 0)
                            delete []rd;
                    }
                }

                //C A - B = 1/2C
                if (nr.first->type[0] > 0 && nr.first->type[1] == -1 && nr.second == 3)
                {
                    n_1 = nr.first->node_1;
                    n_2 = nr.first->node_2;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update B
                    for (int k = 0; k < n_1->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = n_1->ineqleq[k].norm[j] - ineqgeq[i].norm[j]*nr.first->type[0];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = n_1->ineqleq[k].coeff[a] - ineqgeq[i].coeff[a]*nr.first->type[0];
                            c.push_back(x);
                        }
                        if(n_2->update_threshold(rd, -1, R, round, c, u_range, THRESHOLD) == 0)
                            delete []rd;
                    }
                    //update A
                    for (int k = 0; k < n_2->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = nr.first->type[0]*ineqgeq[i].norm[j] + n_2->ineqgeq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = nr.first->type[0]*ineqgeq[i].coeff[a] + n_2->ineqgeq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update_threshold(rd, 1, R, round, c, u_range, THRESHOLD) == 0)
                            delete []rd;
                    }
                }

                //C A - B = -1/2C
                if (nr.first->type[0] < 0 && nr.first->type[1] == -1 && nr.second == 3)
                {
                    n_1 = nr.first->node_1;
                    n_2 = nr.first->node_2;
                    //this->print_cat(); n_1->print_cat(); n_2->print_cat();
                    //update B
                    for (int k = 0; k < n_1->ineqgeq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = -nr.first->type[0]*ineqgeq[i].norm[j] + n_1->ineqgeq[k].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = -nr.first->type[0]*ineqgeq[i].coeff[a] + n_1->ineqgeq[k].coeff[a];
                            c.push_back(x);
                        }
                        if(n_2->update_threshold(rd, 1, R, round, c, u_range, THRESHOLD) == 0)
                            delete []rd;
                    }
                    //update A
                    for (int k = 0; k < n_2->ineqleq.size(); ++k)
                    {
                        //new inequality
                        double *rd = new double[dim];
                        for (int j = 0; j < dim; ++j)
                            rd[j] = n_2->ineqleq[k].norm[j] + nr.first->type[0]*ineqgeq[i].norm[j];
                        //new coefficient for checking
                        std::vector<double> c;
                        for(int a = 0; a < ineqgeq[i].coeff.size(); a++)
                        {
                            double x = n_2->ineqleq[k].coeff[a] + nr.first->type[0]*ineqgeq[i].coeff[a];
                            c.push_back(x);
                        }
                        if(n_1->update_threshold(rd, -1, R, round, c, u_range, THRESHOLD) == 0)
                            delete []rd;
                    }
                }

            }
        }
    }
}


/**
 * @brief       Prune the tuple unable to be top-1
 *              Only consider the newly added inequality
 * @param R     The utility range
 */
void r_node::tuple_prune(hyperplane_set *R)
{
    int size = sets.size(), dim = sets[0].first->tuples[0]->d_num;
    int i = 0;
    for(int a = 0; a < size; ++a)
    {
        if(sets[i].first == NULL || sets[i].second == NULL || sets[i].first->tuples.size() == 0 || sets[i].second->tuples.size() == 0)
            sets.erase(sets.begin() + i);
        else
        {
            bool delete1, delete2;
            int count1 = 0, size1 = sets[i].first->tuples.size();
            for (int j = 0; j < size1; ++j)
            {
                delete1 = false;
                //sets[i].first->tuples[count1]->print();
                int count2 = 0, size2 = sets[i].second->tuples.size();
                for (int k = 0; k < size2; ++k)
                {
                    delete2 = false;
                    //sets[i].second->tuples[count2]->print();
                    double *v = new double[dim];
                    for (int d = 0; d < dim; ++d)
                        v[d] = sets[i].second->tuples[count2]->attr_num[d] - sets[i].first->tuples[count1]->attr_num[d];
                    for (int l = 0; l < ineqgeq.size(); ++l)
                    {
                        if (ineqgeq[l].version)//only check new relation
                        {
                            if (R_cover(ineqgeq[l].norm, v, R->ext_pts))
                            {
                                delete2 = true;
                                break;
                            }
                        }
                    }
                    if (delete2)
                        sets[i].second->tuples.erase(sets[i].second->tuples.begin() + count2);
                    else
                    {
                        for (int l = 0; l < ineqleq.size(); ++l)
                        {
                            if (ineqleq[l].version)//only check new relation
                            {
                                if (R_cover(v, ineqleq[l].norm, R->ext_pts))
                                {
                                    delete1 = true;
                                    break;
                                }
                            }
                        }
                        ++count2;
                    }
                    if (delete1)
                        break;
                }
                if (delete1)
                    sets[i].first->tuples.erase(sets[i].first->tuples.begin() + count1);
                else
                    ++count1;
            }
            ++i;
        }
    }
    is_modified = 0;
    for (int i = 0; i < ineqleq.size(); ++i)
        ineqleq[i].version = 0;
    for (int i = 0; i < ineqgeq.size(); ++i)
        ineqgeq[i].version = 0;
}

/**
 * @brief       Prune the tuple unable to be top-1, based on the information in the relational graph
 * @param R     The utility range
 */
void r_node::tuple_prune_all_rnode(hyperplane_set *R)
{
    //obtain the utility range Ri of each xi, where xi is the top-1 vector w.r.t. any u in Ri
    int sizeleq = ineqleq.size(), sizegeq = ineqgeq.size();
    ext_leq = new vector<point_t*>[sizeleq];
    ext_geq = new vector<point_t*>[sizegeq];
    for(int i = 0; i < sizeleq; ++i)
        R->find_ext_leq(ineqleq, i, ext_leq[i]);
    for(int i = 0; i < sizegeq; ++i)
        R->find_ext_geq(ineqgeq, i, ext_geq[i]);

    //prune points
    int size = sets.size(), dim = sets[0].first->tuples[0]->d_num, i = 0;
    for(int a = 0; a < size; ++a)//sets
    {
        if(sets[i].first == NULL || sets[i].second == NULL || sets[i].first->tuples.size() == 0 || sets[i].second->tuples.size() == 0)
            sets.erase(sets.begin() + i);
        else
        {
            bool delete1;
            int count1 = 0, size1 = sets[i].first->tuples.size();
            for (int j = 0; j < size1; ++j)//tuple 1
            {
                delete1 = false;
                int count2 = 0, size2 = sets[i].second->tuples.size();
                for (int k = 0; k < size2; ++k) //tuple 2
                {
                    //sets[i].first->tuples[count1]->print();
                    //sets[i].second->tuples[count2]->print();
                    double *v = new double[dim];
                    for (int d = 0; d < dim; ++d)
                        v[d] = sets[i].first->tuples[count1]->attr_num[d] - sets[i].second->tuples[count2]->attr_num[d];

                    if(ineqgeq.size() > 0 && R->dominate(ineqgeq, ext_geq, v, 1))//R->R_dominate_geq(ineqgeq, v))
                    {
                        sets[i].second->tuples.erase(sets[i].second->tuples.begin() + count2);
                    }
                    else
                    {
                        if(ineqleq.size() > 0 && R->dominate(ineqleq, ext_leq, v, -1))//R->R_dominate_leq(ineqleq, v))
                        {
                            delete1 = true;
                            break;
                        }
                        ++count2;
                    }
                }
                if (delete1)
                    sets[i].first->tuples.erase(sets[i].first->tuples.begin() + count1);
                else
                    ++count1;
            }
            ++i;
        }
    }
}



/**
 * @brief       Prune the tuple unable to be top-1, based on the information in the relational graph
 * @param R     The utility range
 */
void r_node::tuple_prune_all_rnode_aggre(hyperplane_set *R)
{
    //obtain the utility range Ri of each xi, where xi is the top-1 vector w.r.t. any u in Ri
    int sizeleq = ineqleq.size(), sizegeq = ineqgeq.size();
    ext_leq = new vector<point_t*>[sizeleq];
    ext_geq = new vector<point_t*>[sizegeq];
    for(int i = 0; i < sizeleq; ++i)
        R->find_ext_leq(ineqleq, i, ext_leq[i]);
    for(int i = 0; i < sizegeq; ++i)
        R->find_ext_geq(ineqgeq, i, ext_geq[i]);

    //prune points
    int size = sets.size(), dim = sets[0].first->tuples[0]->d_num, i = 0;
    for(int a = 0; a < size; ++a)//sets
    {
        if(sets[i].first == NULL || sets[i].second == NULL || sets[i].first->tuples.size() == 0 || sets[i].second->tuples.size() == 0)
            sets.erase(sets.begin() + i);
        else
        {
            int count1 = 0, size1 = sets[i].first->tuples.size(), *dom1 = new int[sizeleq];
            for (int j = 0; j < size1; ++j) //tuple 1
            {
                for (int z = 0; z < sizeleq; ++z)
                    dom1[z] = 0;
                for (int k = 0; k < sets[i].second->tuples.size(); ++k) //tuple 2
                {
                    //sets[i].first->tuples[count1]->print();
                    //sets[i].second->tuples[count2]->print();
                    double *v = new double[dim];
                    for (int d = 0; d < dim; ++d)
                        v[d] = sets[i].first->tuples[count1]->attr_num[d] - sets[i].second->tuples[k]->attr_num[d];

                    if (ineqleq.size() > 0 && R->dominated(ineqleq, ext_leq, v, -1, dom1))//R->R_dominate_leq(ineqleq, v))
                    {
                        sets[i].first->tuples.erase(sets[i].first->tuples.begin() + count1);
                        --count1;
                        break;
                    }
                }
                ++count1;
            }

            int count2 = 0, size2 = sets[i].second->tuples.size(), *dom2 = new int[sizegeq];
            for (int j = 0; j < size2; ++j) //tuple 1
            {
                for (int z = 0; z < sizegeq; ++z)
                    dom2[z] = 0;
                for (int k = 0; k < sets[i].first->tuples.size(); ++k) //tuple 2
                {
                    //sets[i].first->tuples[count1]->print();
                    //sets[i].second->tuples[count2]->print();
                    double *v = new double[dim];
                    for (int d = 0; d < dim; ++d)
                        v[d] = sets[i].first->tuples[k]->attr_num[d] - sets[i].second->tuples[count2]->attr_num[d];

                    if (ineqgeq.size() > 0 && R->dominated(ineqgeq, ext_geq, v, 1, dom2))//R->R_dominate_leq(ineqleq, v))
                    {
                        sets[i].second->tuples.erase(sets[i].second->tuples.begin() + count2);
                        --count2;
                        break;
                    }
                }
                ++count2;
            }

            ++i;
        }

    }
}



/**
 * @brief       Prune the tuple unable to be top-1
 * @param R     The utility range
 * @return      Return the largest difference between two tuples
 */
double r_node::tuple_prune_all_rnode_distance(hyperplane_set *R)
{
    //obtain the utility range Ri of each xi, where xi is the top-1 vector w.r.t. any u in Ri
    int sizeleq = ineqleq.size(), sizegeq = ineqgeq.size();
    vector<point_t*> *ext_leq = new vector<point_t*>[sizeleq];
    vector<point_t*> *ext_geq = new vector<point_t*>[sizegeq];
    //R->print();
    for(int i = 0; i < sizeleq; ++i)
        R->find_ext_leq(ineqleq, i, ext_leq[i]);
    for(int i = 0; i < sizegeq; ++i)
        R->find_ext_geq(ineqgeq, i, ext_geq[i]);

    /*
    for(int i = 0; i < sizeleq; ++i)
    {
        for(int j = 0; j < ext_leq[i].size(); ++j)
            ext_leq[i][j]->print();
        cout<<"\n";
    }
    */

    //prune points
    int size = sets.size(), dim = sets[0].first->tuples[0]->d_num, i = 0;
    double max_diff = 1, diff;
    for(int a = 0; a < size; ++a)//sets
    {
        if(sets[i].first == NULL || sets[i].second == NULL || sets[i].first->tuples.size() == 0 || sets[i].second->tuples.size() == 0)
            sets.erase(sets.begin() + i);
        else
        {
            bool delete1;
            int count1 = 0, size1 = sets[i].first->tuples.size();
            for (int j = 0; j < size1; ++j)//tuple 1
            {
                delete1 = false;
                //sets[i].first->tuples[count1]->print();
                int count2 = 0, size2 = sets[i].second->tuples.size();
                for (int k = 0; k < size2; ++k) //tuple 2
                {
                    //sets[i].second->tuples[count2]->print();
                    double *v = new double[dim];
                    for (int d = 0; d < dim; d++)
                        v[d] = sets[i].first->tuples[count1]->attr_num[d] - sets[i].second->tuples[count2]->attr_num[d];

                    diff = R->dominate_distance(ineqgeq, ext_geq, v, 1);
                    if (diff > max_diff)
                        max_diff = diff;
                    if(ineqgeq.size() > 0 && diff == -1)//R->R_dominate_geq(ineqgeq, v))
                    {
                        //cout<<sets[i].second->tuples[count2]->id<<" is dominated by "<<sets[i].first->tuples[count1]->id<<"\n";
                        sets[i].second->tuples.erase(sets[i].second->tuples.begin() + count2);
                    }
                    else
                    {
                        diff = R->dominate_distance(ineqleq, ext_leq, v, -1);
                        if (diff > max_diff)
                            max_diff = diff;
                        if(ineqleq.size() > 0 && diff == -1)//R->R_dominate_leq(ineqleq, v))
                        {
                            //cout<<sets[i].first->tuples[count1]->id<<" is dominated by "<<sets[i].second->tuples[count2]->id<<"\n";
                            delete1 = true;
                            break;
                        }
                        ++count2;
                    }
                }
                if (delete1)
                    sets[i].first->tuples.erase(sets[i].first->tuples.begin() + count1);
                else
                    ++count1;
            }
            ++i;
        }
    }
    return max_diff;
}

/**
 * @brief   Print the node information
 */
void r_node::print()
{
    std::cout<<"categorical value: \n";
    for(int j = 0; j < len; j++)
    {
        std::cout<<setw(10)<<s_1[j]<<" ";
    }
    std::cout<<" --vs-- ";
    for(int j = 0; j < len; j++)
    {
        std::cout<<setw(10)<<s_2[j]<<" ";
    }
    std::cout<<"\n";

    std::cout<<"leq\n";
    for(int i=0; i< ineqleq.size(); ++i)
    {
        std::cout<<ineqleq[i].version<<"     ";
        for(int j=0; j < dim; ++j)
            std::cout<<setw(15)<<ineqleq[i].norm[j]<<" ";
        for(int j =0; j < ineqleq[i].coeff.size(); ++j)
            std::cout<<" "<<setw(5)<<ineqleq[i].coeff[j];
        std::cout<<"\n";
    }
    std::cout<<"geq\n";
    for(int i = 0; i< ineqgeq.size(); ++i)
    {
        std::cout<<ineqgeq[i].version<<"     ";
        for(int j = 0; j < dim; ++j)
            std::cout<<setw(15)<<ineqgeq[i].norm[j]<<" ";
        for(int j =0; j < ineqgeq[i].coeff.size(); ++j)
            std::cout<<" "<<setw(5)<<ineqgeq[i].coeff[j];
        std::cout<<"\n";
    }
    /*
     for(int i=0; i < sets.size(); i++)
    {
        std::cout<<"tuple: ";
        sets[i].first->tuples[0]->print();
        sets[i].second->tuples[0]->print();
        std::cout<<"\n";
    }
     */
}

/**
 * @brief   Print the node information (only categorical attributes)
 */
void r_node::print_cat()
{
    std::cout<<"categorical value: ";
    for(int j = 0; j < len; j++)
    {
        std::cout<<setw(10)<<s_1[j]<<" ";
    }
    std::cout<<" --vs-- ";
    for(int j = 0; j < len; j++)
    {
        std::cout<<setw(10)<<s_2[j]<<" ";
    }
    std::cout<<"\n";
}

/**
 * @param   Set ineqgeq/ineqleq as updated already
 * @param direction
 */
void r_node::clean(int direction)
{
    if(direction == -1)
    {
        for(int i = 0; i < ineqleq.size(); ++i)
            ineqleq[i].version = 0;
    }
    else
    {
        for(int i = 0; i < ineqgeq.size(); ++i)
            ineqgeq[i].version = 0;
    }
}

/**
 * @param   Set ineqgeq & ineqleq as updated already
 */
void r_node::clean()
{
    is_modified = 0;
    for (int i = 0; i < ineqleq.size(); ++i)
        ineqleq[i].version = 0;
    for (int i = 0; i < ineqgeq.size(); ++i)
        ineqgeq[i].version = 0;
}

void r_node::update_coeff()
{
    //add an index for checking
    for(int i = 0; i < ineqleq.size(); ++i)
        ineqleq[i].coeff.push_back(0);
    for(int i = 0; i < ineqgeq.size(); ++i)
        ineqgeq[i].coeff.push_back(0);
}

/**
 * @brief   Record that there exists a categorical relation which can be represented by numerical attributes between two tuple sets
 * @param   direction   -1 (leq)/ 1 (geq)
 */
void r_node::update_tupleset_relation(int direction)
{
    if(direction == -1)
    {
        for(int i = 0; i < sets.size(); ++i)
            sets[i].second->relation_exist[sets[i].first->ID] = 1;
    }
    else
    {
        for(int i = 0; i < sets.size(); ++i)
            sets[i].first->relation_exist[sets[i].second->ID] = 1;
    }
}

/**
 * @brief The regret ratio of a tuple and a tuple set
 * @param t         The tuple
 * @param tuples     The tuple set
 * @param position  The set containing the tuple in the relation
 * @return          The regret
 */
double r_node::regret_check(tuple_t *t, vector<tuple_t*> &tuples, int position, std::vector<point_t*> &R)
{
    int size = tuples.size(), ext_size = R.size(), dim = t->d_num;
    double regret = 0, numerator = -1, denominator = INF, value;
    for(int i = 0; i < size; ++i)
    {
        double *vd = new double[dim];//for denominator
        double *vn = new double[dim];//for numerator
        if(position == 1)
        {
            for(int j = 0; j < ineqgeq.size(); ++j)
            {
                for (int d = 0; d < dim; d++)
                {
                    vd[d] = tuples[i]->attr_num[d]; //- ineqgeq[j].norm[d];
                    vn[d] = tuples[i]->attr_num[d] - t->attr_num[d] - ineqgeq[j].norm[d];
                }
                for (int l = 0; l < ext_geq[j].size(); ++l)
                {
                    value = ext_geq[j][l]->dot_product(vd);//for denominator
                    if (value < denominator)
                        denominator = value;
                    value = ext_geq[j][l]->dot_product(vn);//for numerator
                    if (value > numerator)
                        numerator = value;
                }
                //regret
                value = numerator / denominator;
                if (value > regret)
                    regret = value;
            }

        }
        else //position == 2
        {
            for(int j = 0; j < ineqleq.size(); ++j)
            {
                for (int d = 0; d < dim; d++)
                {
                    vd[d] = tuples[i]->attr_num[d] + ineqleq[j].norm[d];
                    vn[d] = tuples[i]->attr_num[d] - t->attr_num[d] + ineqleq[j].norm[d];
                }
                for (int l = 0; l < ext_leq[j].size(); ++l)
                {
                    value = ext_leq[j][l]->dot_product(vd);
                    if (value < denominator)
                        denominator = value;
                    value = ext_leq[j][l]->dot_product(vn);
                    if (value > numerator)
                        numerator = value;
                }
                //regret
                value = numerator / denominator;
                if (value > regret)
                    regret = value;
            }
        }
    }
    return regret;
}



bool r_node::order_check(tuple_t *tp1, tuple_t *tp2, int position, point_t* ut)
{
    int dim = ut->d;
    double *v = new double[dim];

    if(position == 1)
    {
        for(int j = 0; j < ineqgeq.size(); ++j)
        {
            for (int d = 0; d < dim; d++)
                v[d] = tp1->attr_num[d] - tp2->attr_num[d] + ineqgeq[j].norm[d];
            if(ut->dot_product(v) > 0)
                return true;
        }
    }
    else //position == 2
    {
        for (int j = 0; j < ineqleq.size(); ++j)
        {
            for (int d = 0; d < dim; d++)
                v[d] = tp2->attr_num[d] - tp1->attr_num[d] + ineqleq[j].norm[d];
            if (ut->dot_product(v) < 0)
                return true;
        }
    }
    return false;
}







































