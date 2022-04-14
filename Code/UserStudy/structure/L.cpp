#include "L.h"



/**
 * @brief Constructor
 * @param cnd   The node
 * @param idx   The length of the path (without counting the leaf)
 */
L::L(cnode *cnd, int length, int idx)
{
    index = idx;
    nd.emplace_back(cnd);
    cnode *c = cnd;
    for(int i = 0; i < length - 1; ++i)
    {
        catevalue.emplace_back(c->s);
        c = c->child[0];
    }
    catevalue.emplace_back(c->s);
}


/**
 * @brief Check whether L contains all the categorical value in the unique path of the node
 * @param cnd   The node
 * @param ind   The length of the path (without counting the leaf)
 */
bool L::is_same(cnode *cnd, int ind)
{
    cnode *c = cnd;
    for(int i = 0; i < ind; ++i)
    {
        if(c->s != catevalue[i])
            return false;
        if(i < ind - 1)
            c = c->child[0];
    }
    return true;
}

/**
 * @brief Find two tuples as a question
 * @param l2    The Ri element
 * @param t1    The first tuple
 * @param t2    The second tuple
 * @return      Whether there are two tuples exist
 */
bool L::find_tuple(L *l2, tuple_t *&t1, tuple_t *&t2)
{
    for(int i = 0; i < nd.size(); ++i)
    {
        for(int j = 0; j < l2->nd.size(); ++j)
        {
            if(nd[i]->parent == l2->nd[j]->parent)
            {
                t1 = nd[i]->goforTuple();
                t2 = l2->nd[j]->goforTuple();
                return true;
            }
        }
    }
    return false;
}


void RI::update(valueArray &vA, std::string *s1, std::string *s2)
{
    int length = list[0]->catevalue.size();
    std::vector<std::pair<int, int>> QQ;
    for(int i = 0; i < list.size(); ++i)
    {
        for (int j = 0; j < i; ++j)
        {
            bool pfounded = true, nfounded = true;
            for (int l = 0; l < length; ++l) //positive direction
            {
                if (list[i]->catevalue[l] != list[j]->catevalue[l])
                {
                    if (s1[l] != list[i]->catevalue[l] || s2[l] != list[j]->catevalue[l])
                    {
                        pfounded = false;
                        break;
                    }
                }
                else
                {
                    if (s1[l] != s2[l])
                    {
                        pfounded = false;
                        break;
                    }

                }
            }
            if (pfounded)
            {
                vA.relation[i][j] = 1;
                vA.relation[j][i] = -1;
                QQ.emplace_back(i, j);
            }
            else
            {
                for (int l = 0; l < length; ++l) //positive direction
                {
                    if (list[i]->catevalue[l] != list[j]->catevalue[l])
                    {
                        if (s1[l] != list[j]->catevalue[l] || s2[l] != list[i]->catevalue[l])
                        {
                            nfounded = false;
                            break;
                        }
                    }
                    else
                    {
                        if (s1[l] != s2[l])
                        {
                            nfounded = false;
                            break;
                        }

                    }
                }
                if (nfounded)
                {
                    vA.relation[j][i] = 1;
                    vA.relation[i][j] = -1;
                    QQ.emplace_back(j, i);
                }
            }
        }

        while (QQ.size() > 0)
        {
            vA.update(QQ[0].first, QQ[0].second, QQ);
            QQ.erase(QQ.begin());
        }


    }
}

/**
 * @brief Delete the tuple in the C-Tree
 * @param vA    The relation
 */
void RI::pruneTuple(valueArray &vA, ctree *ct, int level)
{
    for (int i = 0; i < list.size(); ++i)
    {
        for (int j = 0; j < i; ++j)
        {
            if (vA.relation[i][j] == 1)
            {
                for (int k = 0; k < list[j]->nd.size(); ++k)
                {
                    for(int l = 0; l < list[i]->nd.size(); ++l)
                    {
                        if(list[j]->nd[k]->parent == list[i]->nd[l]->parent)
                        {
                            ct->pruneTuple(level, list[j]->nd[k]);
                            list[j]->nd.erase(list[j]->nd.begin() + k);
                            k--;
                            break;
                        }
                    }
                }
            }
            else if (vA.relation[i][j] == -1)
            {
                for (int k = 0; k < list[i]->nd.size(); ++k)
                {
                    for (int l = 0; l < list[j]->nd.size(); ++l)
                    {
                        if(list[i]->nd[k]->parent == list[j]->nd[l]->parent)
                        {
                            ct->pruneTuple(level, list[i]->nd[k]);
                            list[i]->nd.erase(list[i]->nd.begin() + k);
                            k--;
                            break;
                        }
                    }
                }
            }
        }
    }
}



























