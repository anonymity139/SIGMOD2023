#include "Ctree.h"


/**
 * @brief Constructor
 */
cnode::cnode(){}

/**
 * @brief Constructor
 * @param str
 */
cnode::cnode(std::string str, int l)
{
    s = str;
    level = l;
}

/**
 * @brief   Find whether there is a child node which contains the queried categorical value
 * @param str   The queried categorical value
 * @return      -1 There is no such child
 *              i  The index of the child
 */
int cnode::search_child(std::string str)
{
    int size = child.size();
    if(size <= 0)
        return -1;
    for(int i = 0; i < size; ++i)
    {
        if(child[i]->s == str)
            return i;
    }
    return -1;
}

/**
 * @brief Build the node in the tree for the rest of the categorical value in the tuple
 * @param tp            The tuple
 * @param index         Indicate the start position in the string array
 */
void cnode::build(tuple_t *tp, int index, std::vector<cnode *> *Tlist)
{
    int len = tp->d_cat;
    cnode* cn = this;
    for(int i = index; i < len; ++i)
    {
        cnode *new_node = new cnode(tp->attr_cat[i], i + 1);
        Tlist[i+1].push_back(new_node);
        cn->child.push_back(new_node);
        new_node->parent = cn;
        cn = new_node;
    }
    cn->tuple.push_back(tp);

}

/**
 * @brief Print the node
 * @param level
 * @param posi
 */
void cnode::print(int lev, int posi)
{
    if(posi != 0)
        std::cout << std::setw((lev - 1) * 6) << "";
    if(lev > 0)
        std::cout << std::setw(5) << s << "|";
    if(child.size() <= 0)
    {
        for (int i = 0; i < tuple.size(); ++i)
            std::cout << tuple[i]->id;
        std::cout << "\n";
    }
    else
    {
        for(int i = 0; i < child.size(); ++i)
            child[i]->print(lev + 1, i);
    }
}

/**
 * @brief   Find the corresponding tuples for this node
 * @return
 */
tuple_t *cnode::goforTuple()
{
    cnode *cn = this;
    while (cn->child.size() > 0)
        cn = cn->child[0];
    return cn->tuple[0];
}


/**
 * @brief Constructor
 */
ctree::ctree(int num_cat)
{
    root = new cnode("", 0);
    Tlist = new std::vector<cnode*>[num_cat + 1];
    Tlist[0].push_back(root);
}


/**
 * @brief Add a tuple to the ctree
 * @param tp    The tuple
 */
void ctree::add(tuple_t *tp)
{
    int len = tp->d_cat, index = len;
    cnode *cn = root;
    for(int i = 0; i < len; ++i)//foreach categorical value
    {
        int ind = cn->search_child(tp->attr_cat[i]);
        if(ind != -1)
            cn = cn->child[ind];
        else
        {
            index = i;
            break;
        }
    }
    if(index < len)
        cn->build(tp, index, Tlist);
    else
        cn->tuple.push_back(tp);
}

/**
 * @brief Print the tree
 */
void ctree::print()
{
    root->print(0, 0);
}

/**
 * @brief Check the index-th level node, whether each node connects with only 1 child
 * @param index     The index of the level
 * @return      true    Each node connects with 1 child
 *              false   Each node does not connect with 1 child
 */
bool ctree::is_oneTone(int index)
{
    for(int i = 0; i < Tlist[index].size(); ++i)
    {
        if (Tlist[index][i]->child.size() > 1)
            return false;
    }
    return true;
}

/**
 * @brief Prune the tuples in the tree which cannot be the best
 *        Check whether each node has only one child
 * @param index         The index of the value level in the tree
 * @param strOrderAll   The string list (in order)
 * @return Whether for each node, the size of the child are 1
 */
bool ctree::pruneTuple(int index, valueArray &vA, std::map<std::string, int> &valueMap)
{
    bool is_one = true;
    for(int i = 0; i < Tlist[index].size(); ++i)//for each node
    {
        bool *isDelete = new bool[Tlist[index][i]->child.size()];
        for (int j = 0; j < Tlist[index][i]->child.size(); ++j)
            isDelete[j] = false;
        //consider each pair of child
        for (int j = 0; j < Tlist[index][i]->child.size() - 1; ++j)
        {
            for (int k = j + 1; k < Tlist[index][i]->child.size(); ++k)
            {
                int ind1, ind2;
                std::map<std::string, int>::iterator iter;
                iter = valueMap.find(Tlist[index][i]->child[j]->s);
                if (iter != valueMap.end())
                    ind1 = iter->second;
                iter = valueMap.find(Tlist[index][i]->child[k]->s);
                if (iter != valueMap.end())
                    ind2 = iter->second;
                if (vA.relation[ind1][ind2] == 1)
                    isDelete[k] = true;
                else if (vA.relation[ind1][ind2] == -1)
                    isDelete[j] = true;
            }
        }

        for (int j = Tlist[index][i]->child.size() - 1; j >= 0; --j)
        {
            if (isDelete[j])
                Tlist[index][i]->child.erase(Tlist[index][i]->child.begin() + j);
        }
        if(Tlist[index][i]->child.size() > 1)
            is_one = false;
    }
    return is_one;
}

/**
 * @brief Prune tuples in the tree
 * @param level The level the node is in
 * @param cn    The node
 * @return
 */
bool ctree::pruneTuple(int level, cnode *cn)
{
    //parent
    cnode *pcn = cn->parent;
    for(int i = 0; i < pcn->child.size(); ++i)
    {
        if (pcn->child[i] == cn)
        {
            pcn->child.erase(pcn->child.begin() + i);
            break;
        }
    }
    for(int i = 0; i < Tlist[level].size(); ++i)
    {
        if(Tlist[level][i] == cn)
        {
            Tlist[level].erase(Tlist[level].begin() + i);
            break;
        }
    }
}


/**
 * @brief Find the tuple in the tree
 * @param index The level
 * @param s1    The first value
 * @param s2    The second value
 * @param t1    The first tuple
 * @param t2    The second tuple
 * @return      Whether there are two tuples exist
 */
bool ctree::find_tuple(int index, std::string s1, std::string s2, tuple_t *&t1, tuple_t *&t2)
{
    for(int i = 0; i < Tlist[index].size(); ++i)
    {
        bool first = false, second = false;
        for(int j = 0; j < Tlist[index][i]->child.size(); ++j)
        {
            if(Tlist[index][i]->child[j]->s == s1)
            {
                first = true;
                t1 = Tlist[index][i]->child[j]->goforTuple();
            }
            if(Tlist[index][i]->child[j]->s == s2)
            {
                second = true;
                t2 = Tlist[index][i]->child[j]->goforTuple();
            }
            if(first && second)
                return true;
        }
    }
    return false;
}

/**
 * @brief Delete the value in the strOrderAll which will not be used in the comparison
 * @param strOrderAll
 */
bool ctree::updateString(int index, std::vector<std::string> catevalue)
{
    int length = catevalue.size();
    for (int i = 0; i < Tlist[index].size(); ++i)
    {
        if(Tlist[index][i]->child.size() > 1)
        {
            for(int k = 0; k < Tlist[index][i]->child.size(); ++k)
            {
                bool isFind = true;
                cnode *cc = Tlist[index][i]->child[k];
                for(int j = 0; j < length; ++j)
                {
                    if(cc->s == catevalue[j])
                    {
                        if (j < length - 1)
                            cc = cc->child[0];
                    }
                    else
                    {
                        isFind = false;
                        break;
                    }
                }
                if (isFind)
                    return true;
            }
        }
    }
    return false;
}

/**
 * @brief Count the number of tuples
 * @param index
 * @return
 */
int ctree::count_tuple(int index)
{
    int total = 0;
    for(int i = 0; i < Tlist[index].size(); ++i)
        total += Tlist[index][i]->child.size();
    return total;
}
































