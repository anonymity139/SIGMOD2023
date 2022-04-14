#ifndef REGRET_CTREE_H
#define REGRET_CTREE_H

#include <string>
#include <queue>
#include <iomanip>
#include <tuple_t.h>

class cnode
{
public:
    std::string s;
    int level;
    std::vector<cnode*> child;
    std::vector<tuple_t*> tuple;

    cnode();
    cnode(std::string str);
    int search_child(std::string str);
    void build(tuple_t *tp, int index);
    void print(int level, int posi);

};

class ctree
{
public:
    cnode *root;
    std::vector<std::vector<cnode *>> Tlist;

    void add(tuple_t *tp);
    void build_list();
    void build_index(int index, std::map<std::string, double> *list, std::vector<cnode*> ***Lindex);
    void select_question(std::vector<cnode *> ***Lindex, std::map<std::string, double> *list, int num, int dindex, int cindex, tuple_t *tp1, tuple_t *tp2);
    void print();
};


#endif //REGRET_CTREE_H
