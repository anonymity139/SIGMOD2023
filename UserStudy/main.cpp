#include "structure/data_utility.h"
#include "structure/data_struct.h"
#include "structure/point_set.h"
#include "structure/tuple_set.h"
#include "structure/define.h"
#include <vector>
#include <ctime>
#include <sys/time.h>
#include "UH/maxUtility.h"
#include "structure/cluster_t.h"
#include "Separation.h"
#include "Combination.h"
#include "Groundtruth.h"
#include "Ranking/active_ranking.h"
#include "Adaptive/adaptive.h"

int main(int argc, char *argv[])
{
    //initialization
    std::map<std::string, double> categorical_value;
    std::vector<std::string> categorical;
    tuple_set *car_set = new tuple_set("car1.txt");
    tuple_set *orgset = new tuple_set("car2.txt");
    tuple_set *t_set = new tuple_set();
    orgset->random_choose(t_set);
    t_set->preprocessCar();
    point_set *p_set = t_set->transform(categorical);
    point_set *p_skyline = p_set->skyline();
    tuple_set *t_skyline = t_set->skyline_based_num();
    t_set->find_categorical(categorical_value);
    int d_num = t_set->tuples[0]->d_num; //number of numerical dimension
    int dim_p = p_skyline->points[0]->d; //dimensionality of transformed points
    int d_new_attr = dim_p - t_set->tuples[0]->d_num; //number of newly generated dimension


    // obtain the utility vector
    //double util[14] = {60.56, 82.82, 36.44, 143.89, 157.75, 21.92, 73.36, 110.27, 44.65, 65.49, 118.40};
    double util[11] = {107.04, 93.21, 57.21, 35.52, 30.62, 113.88, 159.85, 6.58, 106.47, 99.30, 190.32};
    point_t *up = new point_t(dim_p);
    for(int i = 0; i < dim_p; ++i)
        up->attr[i] = util[i];
    point_t *ut = up->extract_num(d_num, d_new_attr);//for numerical attributes of tuples
    double u_range = 0;
    for (int i = 0; i < d_num; i++)
        u_range += up->attr[d_new_attr + i];
    std::map<std::string, double>::iterator it;
    for (int i = 0; i < d_new_attr; i++)
        categorical_value[categorical[i]] = up->attr[i];

    double Csize;
    int Qcount = 0, TID; vector<int> TID_list;


    string userName;
    // the welcome message
    cout <<"Please input your name: "; cin >> userName;
    cout << "-------------------------Welcome to the recommending car system------------------------------\n";
    cout <<"1. In our research project, we want to ask as few questions as possible so that we could help\n"
           "you to find your favorite used car in our used car database. \n"
           "2. The final car returned by our system should be your favorite car.\n"
           "3. There are 6 rounds in the system. We will ask you a list of consecutive questions for each\n"
           "round. Each round involving consecutive questions correspond to a method in our system.\n"
           "4. You will be presented two cars each time and you need to choose the one you favor more.\n"
           "5. Price 1049-95400 Year 2002-2015 Power 52-575 Used(km) 10000-125000\n";
           
    ofstream fp("../Result/" + userName + ".txt");

    // the UH-Simplex algorithm
    cout << "\n\n=============================Round 1=====================================\n";
    max_utility(car_set, p_skyline, up, 2, 0.0, 60, Qcount, Csize, SIMPLEX, EXACT_BOUND, RTREE, HYPER_PLANE, TID, fp);
    insert_intlist(TID_list, TID);

    //Adaptive Algorithm
    cout << "\n\n=============================Round 2=====================================\n";
    Adaptive(car_set, p_skyline, up, Qcount, TID, fp);
    insert_intlist(TID_list, TID);

    //Separation Algorithm
    cout << "\n\n=============================Round 3=====================================\n";
    separation_round(car_set, t_skyline, ut->attr, categorical_value, 1, Qcount, 2, TID, fp);
    insert_intlist(TID_list, TID);

    //Active-Ranking Algorithm
    cout << "\n\n=============================Round 4=====================================\n";
    Active_Ranking(car_set, p_skyline, up, Qcount, TID, fp);
    insert_intlist(TID_list, TID);

    //Algorithm combination
    cout << "\n\n=============================Round 5=====================================\n";
    combination_round(car_set, t_skyline, ut->attr, categorical_value, 1, Qcount, 2, TID, fp);
    insert_intlist(TID_list, TID);

    // the UH-Random algorithm
    cout << "\n\n=============================Round 6=====================================\n";
    max_utility(car_set, p_skyline, up, 2, 0.0, 60, Qcount, Csize, RANDOM, EXACT_BOUND, RTREE, HYPER_PLANE, TID, fp);
    insert_intlist(TID_list, TID);




    cout<< "\n\n\nThe recommended tuples: \n";
    std::cout << "-------------------------------------------------------------------------\n";
    std::cout << std::setw(10) << " " << std::setw(10) << "Fuel" << std::setw(12) << "Price($)" << std::setw(12) << "Year" << std::setw(12) << "PowerPS" << std::setw(12) << "Used KM\n";
    std::cout << "-------------------------------------------------------------------------\n";
    for(int i = 0; i < TID_list.size(); ++i)
    {
        std::cout << std::setw(10) << i+1 << std::setw(10) << car_set->tuples[TID_list[i]]->attr_cat[0] << std::setw(12) <<
        car_set->tuples[TID_list[i]]->attr_num[0] << std::setw(12) << car_set->tuples[TID_list[i]]->attr_num[1] << std::setw(12) << car_set->tuples[TID_list[i]]->attr_num[2] << std::setw(12) << car_set->tuples[TID_list[i]]->attr_num[3] <<"\n";
        std::cout << "-------------------------------------------------------------------------\n";
    }
    cout <<"Please give an order for the shown used car(s) (e.g., ";
    for(int i = 0; i < TID_list.size() - 1; ++i)
        cout << i + 1 << " ";
    cout << TID_list.size();
    cout <<"), ";
    cout << "where the first one \nis the most preferred tuple and the last one is the least preferred tuple: ";
    int *order = new int[TID_list.size()];
    for(int i = 0; i < TID_list.size(); ++i)
        cin >> order[i];
    fp << "order: \n";
    fp << "---------------------------------------------------------------------------------\n";
    for(int i = 0; i < TID_list.size(); ++i)
    {
        fp << std::setw(10) << i + 1 << std::setw(10) << car_set->tuples[TID_list[order[i] - 1]]->attr_cat[0] << std::setw(12) <<
           car_set->tuples[TID_list[order[i] - 1]]->attr_num[0] << std::setw(12) << car_set->tuples[TID_list[order[i] - 1]]->attr_num[1]
           << std::setw(12) << car_set->tuples[TID_list[order[i] - 1]]->attr_num[2] << std::setw(12) << car_set->tuples[TID_list[order[i] - 1]]->attr_num[3] << "\n";
        fp << "---------------------------------------------------------------------------------\n";
    }


    fp.close();
    delete p_skyline;
    return 0;
}
