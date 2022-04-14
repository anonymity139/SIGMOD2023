#ifndef U_2_TUPLE_SET_H
#define U_2_TUPLE_SET_H

#include <vector>
#include "define.h"
#include "tuple_t.h"
#include "point_set.h"
#include <map>

class between_tupleset
{
public:
    int node_index;
    int position; //the position of the tuple set in the relation
};


class tuple_set
{
public:
    int ID, *relation_exist;
    between_tupleset* bettupleset;
    std::vector<tuple_t*> tuples;

    tuple_set();
    explicit tuple_set(const char *input);
    ~tuple_set();

    tuple_set* skyline_based_num();//find the skyline points based on the numerical attributes only
    point_set* transform(std::vector<std::string> &categorical);//numerical&&categorical=>numerical
    void print();
    void prune_same_cat(std::vector<point_t*> &R);//prune tuples(assume same categorical values)
    void find_categorical(std::map<std::string, double> &list);
    void find_categorical(std::map<std::string, double> *list, double *num_c);
    bool is_relation_cover(int index, int num, int *index_cluster);
    double regret_sameset(std::vector<point_t*> &R);
    double regret_sameset(std::vector<point_t*> &R, int ID);
    int max_uvalue(point_t* u, std::map<std::string, double> &categorical_value);

    int show_to_user(int idx_1, int idx_2);

    void random_choose(tuple_set *tset);

    void preprocessCar();
};


#endif //U_2_TUPLE_SET_H
