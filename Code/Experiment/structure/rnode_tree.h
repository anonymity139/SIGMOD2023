#ifndef REGRET_RNODE_TREE_H
#define REGRET_RNODE_TREE_H
#include <string>
#include <vector>
#include "r_node.h"


class rnode_node
{
public:
    std::string s;
    std::vector<rnode_node*> child;
    std::vector<r_node*> rnodes;

    rnode_node();
    rnode_node(std::string str);
    int search_child(std::string str);
    void build(r_node* rNode, int str_index, int index);
    rnode_node* add(r_node* rNode, int &str_index, int &index);
    void print(int level, int posi);
};





class rnode_tree
{
public:
    rnode_node *root;

    rnode_tree(){};
    rnode_tree(std::vector<r_node*> list);
    int search(string *str, int len, int &direction);
    void print();
};


#endif //REGRET_RNODE_TREE_H
