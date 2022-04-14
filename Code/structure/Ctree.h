#ifndef REGRET_CTREE_H
#define REGRET_CTREE_H

#include <string>
#include <queue>
#include <iomanip>
#include <tuple_t.h>
#include <vector>
#include <string>
#include "valueArray.h"
class cnode
{
public:
    std::string s;
    int level;
    cnode* parent;
    std::vector<cnode*> child;
    std::vector<tuple_t*> tuple;

    cnode();
    cnode(std::string str, int l);
    int search_child(std::string str);
    tuple_t* goforTuple();
    void build(tuple_t *tp, int index, std::vector<cnode *> *Tlist);
    void print(int level, int posi);

};

class ctree
{
public:
    cnode *root;
    std::vector<cnode *> *Tlist;

    ctree(int num_cat);
    void add(tuple_t *tp);
    bool is_oneTone(int index);
    bool find_tuple(int index, std::string s1, std::string s2, tuple_t *&t1, tuple_t *&t2);
    bool pruneTuple(int index, valueArray &vA, std::map<std::string, int> &valueMap);
    bool pruneTuple(int level, cnode *cn);
    bool updateString(int index, std::vector<std::string> catevalue);
    int count_tuple(int index);
    void print();
};


#endif //REGRET_CTREE_H
