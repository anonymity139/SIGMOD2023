#ifndef RUN_L_H
#define RUN_L_H


#include <vector>
#include "Ctree.h"

class L
{
public:
    int index;
    std::vector<std::string> catevalue;
    std::vector<cnode*> nd;

    L(cnode *cnd, int length, int idx);
    bool is_same(cnode *cnd, int idx);
    bool find_tuple(L *l2, tuple_t *&t1, tuple_t *&t2);

};


class RI
{
public:
    std::vector<L*> list;

    void update(valueArray &vA, std::string *s1, std::string *s2);
    void pruneTuple(valueArray &vA, ctree *ct, int level);
};

#endif //RUN_L_H
