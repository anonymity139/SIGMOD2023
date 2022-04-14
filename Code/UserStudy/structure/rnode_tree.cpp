#include "rnode_tree.h"

/**
 * @brief Constructor
 */
rnode_node::rnode_node()
{
    s = "\0";
}

/**
 * @brief Constructor
 * @param str The categorical value in the node
 */
rnode_node::rnode_node(std::string str)
{
    s = str;
}

/**
 * @brief   Find whether there is a child node which contains the queried categroical value
 * @param str   The queried categorical value
 * @return      --1 There is no such child
 *              -i  The index of the child
 */
int rnode_node::search_child(std::string str)
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
 * @brief Build the node in the tree for the rest of the categorical value in the r_node
 * @param rNode         The r_node
 * @param str_index     Indicate which string array s_1 or s_2
 * @param index         Indicate the start position in the string array
 */
void rnode_node::build(r_node *rNode, int str_index, int index)
{
    int len = rNode->len;
    rnode_node* cnode = this;
    if(str_index == 1)
    {
        for(int i = index; i < len; ++i)
        {
            rnode_node *new_node = new rnode_node(rNode->s_1[i]);
            cnode->child.push_back(new_node);
            cnode = new_node;
        }
        cnode->build(rNode, 2, 0);
    }
    else
    {
        for(int i = index; i < len; ++i)
        {
            rnode_node *new_node = new rnode_node(rNode->s_2[i]);
            cnode->child.push_back(new_node);
            cnode = new_node;
        }
        cnode->rnodes.push_back(rNode);
    }
}

/**
 * @brief Start from the first tree node, search the path which covers the categorical values of the r_node
 * @param rNode         The r_node
 * @param str_index     If there is no path, the searching stops at which string array s_1 or s_2
 * @param index         If there is no path, the searching stops at which position in the string array
 * @return              -NUll If there is a path, insert the r_node
 *                      -rnode_node If there is no path, return the tree node where the searching stops
 */
rnode_node *rnode_node::add(r_node *rNode, int &str_index, int &index)
{
    int len = rNode->len;
    rnode_node *cnode = this;
    //s_1
    for(int i = 0; i < len; ++i)//foreach categorical value
    {
        int ind = cnode->search_child(rNode->s_1[i]);
        if(ind != -1)
            cnode = cnode->child[ind];
        else
        {
            str_index = 1;
            index = i;
            return cnode;
        }
    }
    //s_2
    for(int i = 0; i < len; ++i)//foreach categorical value
    {
        int ind = cnode->search_child(rNode->s_2[i]);
        if(ind != -1)
            cnode = cnode->child[ind];
        else
        {
            str_index = 2;
            index = i;
            return cnode;
        }
    }
    cnode->rnodes.push_back(rNode);
    return NULL;
}

/**
 * @brief   Print
 */
void rnode_node::print(int level, int posi)
{
    if(posi != 0)
        cout << setw((level - 1) * 6) << "";
    if(level > 0)
        cout << setw(5) << s << "|";
    if(child.size() <= 0)
    {
        for (int i = 0; i < rnodes.size(); ++i)
            rnodes[i]->print_cat();
    }
    else
    {
        for(int i = 0; i < child.size(); ++i)
            child[i]->print(level + 1, i);
    }
}


/**
 * @brief Constructor
 * @param list The list of r_node
 */
rnode_tree::rnode_tree(std::vector<r_node *> list)
{
    root = new rnode_node();
    rnode_node *cnode;
    int size = list.size();
    int str_index, index;
    for(int i = 0; i < size; ++i)//foreach r_node
    {
        cnode = root->add(list[i], str_index, index);
        if(cnode != NULL)
            cnode->build(list[i], str_index, index);
    }
}

/**
 * @brief Print the tree
 */
void rnode_tree::print()
{
    root->print(0, 0);
}

/**
 * @brief Obtain the r_node in the tree, whose categorical values are the same as str
 * @param str   The string array
 * @return      The index of the r_node.
 */
int rnode_tree::search(string *str, int len, int &direction)
{
    rnode_node *cnode = root;
    //C
    for(int i = 0; i < 2 * len; ++i)
    {
        int ind = cnode->search_child(str[i]);
        if(ind != -1)
            cnode = cnode->child[ind];
        else
        {
            cnode = NULL;
            break;
        }
    }
    if(cnode != NULL)
    {
        direction = 1;
        return cnode->rnodes[0]->ID;
    }
    //-C
    cnode = root;
    for(int i = 0; i < 2 * len; ++i)//foreach categorical value
    {
        int ind;
        if(i < len)
            ind = cnode->search_child(str[len + i]);
        else
            ind = cnode->search_child(str[i - len]);
        if(ind != -1)
            cnode = cnode->child[ind];
        else
            return -1;
    }
    direction = -1;
    return cnode->rnodes[0]->ID;
}









































