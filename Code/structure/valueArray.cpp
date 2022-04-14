#include <iostream>
#include "valueArray.h"

/**
 * @brief Constructor
 * @param num   The number of values
 */
valueArray::valueArray(int numm)
{
    num = numm;
    relation = new int*[numm];
    for(int i = 0; i < num; ++i)
    {
        relation[i] = new int[numm];
    }

    for(int i = 0; i < numm; ++i)
    {
        for(int j = 0; j < numm; ++j)
            relation[i][j] = 0;
    }
}

/**
 * @brief Update the relation of the current value
 *        Update the relations related to the value larger than the value index1
 *        Update the relations related to the value smaller than the value index2
 * @param valueMap  The mapping of each value to its position
 * @param index1    The index of the larger value
 * @param index2    The index of the smaller value
 */
void valueArray::update(int index1, int index2, std::vector<std::pair<int, int>>& Q)
{
    //the relation of value index1 and index2
    for(int i = 0; i < num; ++i)
    {
        if(relation[index1][i] == -1)
        {
            if(relation[i][index2] == 0)
                Q.emplace_back(i, index2);
            relation[index2][i] = -1;
            relation[i][index2] = 1;
        }
        if(relation[index2][i] == 1)
        {
            if(relation[index1][i] == 0)
                Q.emplace_back(index1, i);
            relation[index1][i] = 1;
            relation[i][index1] = -1;
        }
    }

    //the relations of values larger than index1
    for(int i = 0; i < num; ++i)
    {
        if(relation[i][index1] == 1)
        {
            for(int j = 0; j < num; ++j)
            {
                if(relation[index2][j] == 0)
                {
                    if(relation[i][j] == 1)
                        Q.emplace_back(i, j);
                    relation[i][j] = 1;
                    relation[j][i] = -1;
                }
            }
        }
    }

    /*
    //the relations of values samller than index2
    for(int i = 0; i < num; ++i)
    {
        if(relation[index2][i] == 1)
        {
            for(int j = 0; j < num; ++j)
            {
                if(relation[index2][j] == -1)
                    relation[i][j] = -1;
            }
        }
    }


    for(int i = 0; i < num; ++i)
    {
        for (int j = 0; j < num; ++j)
            std::cout << relation[i][j] << "  ";
        std::cout<<"\n";
    }
    */
}

void valueArray::print()
{
    for(int i = 0; i < num; ++i)
    {
        for(int j = 0; j < num; ++j)
        {
            std::cout<<relation[i][j]<<"  ";
        }
        std::cout<<"\n";
    }
}

























