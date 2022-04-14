#ifndef RUN_VALUEARRAY_H
#define RUN_VALUEARRAY_H
#include <map>
#include <vector>

class valueArray
{
public:
    int **relation, num;
    explicit valueArray(int numm);
    void update(int index1, int index2, std::vector<std::pair<int, int>>& Q);
    void print();
};


#endif //RUN_VALUEARRAY_H
