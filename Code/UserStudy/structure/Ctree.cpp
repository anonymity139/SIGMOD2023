#include "Ctree.h"

/**
 * @brief Constructor
 */
cnode::cnode(){}

/**
 * @brief Constructor
 * @param str
 */
cnode::cnode(std::string str)
{
    s = str;
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
void cnode::build(tuple_t *tp, int index)
{
    int len = tp->d_cat;
    cnode* cn = this;
    for(int i = index; i < len; ++i)
    {
        cnode *new_node = new cnode(tp->attr_cat[i]);
        new_node->level = i + 1;
        cn->child.push_back(new_node);
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
 * @brief Add a tuple to the ctree
 * @param tp    The tuple
 */
void ctree::add(tuple_t *tp)
{
    int len = tp->d_cat, index = len - 1;
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
    if(index < len - 1)
        cn->build(tp, index);
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
 * @brief Build a list index for each level of the tree node
 * @param Tlist
 */
void ctree::build_list()
{
    int size = 0;
    std::queue<cnode*> Que;
    Que.push(root);
    std::vector<cnode *> tlist0;
    Tlist.push_back(tlist0);
    while (!Que.empty())
    {
        if(Que.front()->level > size)
        {
            std::vector<cnode *> tlist;
            Tlist.push_back(tlist);
            ++size;
        }
        Tlist[size].push_back(Que.front());
        for(int i = 0; i < Que.front()->child.size(); ++i)
            Que.push(Que.front()->child[i]);
        Que.pop();
    }
}

/**
 * @brief Build a index for the values on the (index+1)-th level nodes
 * @param index     The level of the nodes
 * @param list      A mapping from the categorical values to their index
 * @param Lindex    The index stored the values in the corresponding dimension
 */
void ctree::build_index(int index, std::map<std::string, double> *list, std::vector<cnode*> ***Lindex)
{
    for(int i = 0; i < Tlist[index].size(); ++i)
    {
        for(int j = 0; j < Tlist[index][i]->child.size() - 1; ++j)
        {
            for(int k = j + 1; k < Tlist[index][i]->child.size(); ++k)
            {
                int cin1 = list[index].find(Tlist[index][i]->child[j]->s)->second;
                int cin2 = list[index].find(Tlist[index][i]->child[k]->s)->second;
                if(cin1 < cin2)
                    Lindex[cin1][cin2]->push_back(Tlist[index][i]);
            }
        }
    }
}

/**
 * @brief Select tuples as a question
 * @param Lindex The index stored the values in the corresponding dimension
 * @param dim    The number of dimensions
 * @param cindex The index of the current largest categorical value
 * @param tp1    The first tuple
 * @param tp2    The second tuple
 */
void ctree::select_question(std::vector<cnode *> ***Lindex, std::map<std::string, double> *list, int num, int dindex, int cindex, tuple_t *tp1, tuple_t *tp2)
{
    //find a largest and common


    //find a common


    //find a largest
}





























