#include "point_set.h"
#include "../qhull/qhull_build.h"


/**
 * @brief Constructor
 */
point_set::point_set(){}

/**
 * @brief Constructor
 *        Create a point set the same as p_set, all the points are re-created
 * @param p_set     Point set
 */
point_set::point_set(point_set *p_set)
{
    int num = p_set->points.size(), dim = p_set->points[0]->d;
    point_t *p;
    for (int i = 0; i < num; i++)
    {
        p = new point_t(dim, p_set->points[i]->id);
        for (int j = 0; j < dim; j++)
        {
            p->attr[j] = p_set->points[i]->attr[j];
        }
        points.push_back(p);
    }
}

/**
 * @brief Constructor
 *        Record all the points in the input file to the point set
 * @param input     Name of input file.
 */
point_set::point_set(const char* input)
{
    FILE* c_fp;
    char filename[MAX_FILENAME_LENG];
    sprintf(filename, "input/%s", input);
    printf("%s\n", filename);
    if ((c_fp = fopen(filename, "r")) == NULL)
    {
        fprintf(stderr, "Cannot open the data file %s.\n", filename);
        exit(0);
    }

    int number_of_points, dim;
    point_t* p;
    fscanf(c_fp, "%i%i", &number_of_points, &dim);

    // read points line by line
    for (int i = 0; i < number_of_points; i++)
    {
        p = new point_t(dim, i);
        for (int j = 0; j < dim; j++)
        {
            fscanf(c_fp, "%lf", &p->attr[j]);
        }
        points.push_back(p);
    }

    fclose(c_fp);
}

/**
 * @brief Extract the points only with d_num attributes
 *        Each point (construction) : dt_cat + d_num
 *        NOTE: do not consider points with d_num attribute values as 0
 * @param d_num     The number of extracted attributes
 * @param dt_cat    The number of unextracted attrubites
 * @param pset      The point set
 */
point_set::point_set(int d_num, int dt_cat, std::vector<point_t *> &pset, double u_range)
{
    for(int i = 0; i < pset.size(); ++i)
    {
        double sum = 0;
        for (int j = 0; j < d_num; ++j)
            sum += pset[i]->attr[j + dt_cat];
        if(sum > EQN2)
        {
            point_t *p = new point_t(d_num);
            for (int j = 0; j < d_num; ++j)
                p->attr[j] = (pset[i]->attr[j + dt_cat]) * u_range / sum;
            //check whether it exists
            bool already = false;
            for (int j = 0; j < points.size() && !already; ++j)
            {
                if (p->is_same(points[j]))
                    already = true;
            }
            if (!already)
                points.push_back(p);
        }
    }
}



/**
 *@brief  Destructor
 *        Delete the points in the array
 */
point_set::~point_set()
{
    int i = points.size();
    point_t *p;
    while(i>0)
    {
        p = points[i-1];
        points.pop_back();
        delete  p;
        i--;
    }
    points.clear();
}

/*
 *	For debug purpose, print all the points in the set
 */
void point_set::print()
{
    for (int i = 0; i < points.size(); i++)
        points[i]->print();
    printf("\n");
}

/**
 * @brief           Reload the points Randomly
 *                  Define "RandRate" to control how many point reinserted
 * @param p_set     The returned dataset where the points are inserted randomly
 */
void point_set::random(double RandRate)
{
    int size = points.size();
    //reinsert
    for (int i = 0; i < size * RandRate; i++)
    {
        int n = ((int) rand()) % size;
        point_t *p = points[n];
        points.erase(points.begin() + n);
        points.push_back(p);
    }
}

/**
 * @brief   Delete the same points, keep one left
 */
void point_set::delete_same()
{
    int count, size;
    //insert all the points
    for (int i = 0; i < points.size()-1; i++)
    {
        count = i + 1; size = points.size();
        for (int j = i + 1; j < size; j++)
        {
            if (points[i]->is_same(points[count]))
            {
                points.erase(points.begin() + count);
            }
            else
            {
                count++;
            }
        }
    }
}

/**
 * @brief       Sort points based on their utility w.r.t. linear function u
 *              Do not copy points. Just order them by storing the 'pointer' in the new set
 * @param u     Linear function u
 */
point_set* point_set::sort(point_t *u)
{
    int size = points.size();
    if(size <=0)
        return NULL;
    point_set *return_set = new point_set();
    return_set->points.push_back(points[0]);
    for (int i = 1; i < size; i++)
    {
        double v0 = points[i]->dot_product(u);
        int left = 0, right = return_set->points.size() - 1;
        //find the place for p_set[i] in return_point
        //record the place index in "left"
        while (left <= right)
        {
            int middle = (left + right) / 2;
            double v = return_set->points[middle]->dot_product(u);
            if (v0 > v)
            {
                right = middle - 1;
            }
            else
            {
                left = middle + 1;
            }
        }
        return_set->points.insert(return_set->points.begin() + left, points[i]);
    }
    /*
    for(int i=0; i<return_set->points.size();i++)
    {
        return_set->points[i]->print();
    }
    */
    return return_set;
}

/**
 * @brief Compute the skyline set, each points create a new space for storing points
 * @return The skyline set
 */
point_set* point_set::skyline()
{
    int size = points.size();
    if(size <=0)
    {
        std::cout<<"Error: dataset for producing skyline is empty.";
        return NULL;
    }
    int i, j, m, dominated, index = 0, dim = points[0]->d;
    point_t* pt;

    int* sl = new int[size];

    for (i = 0; i < size; ++i)
    {
        dominated = 0;
        pt = points[i];
        //pt->print();
        // check if pt is dominated by the skyline so far
        for (j = 0; j < index && !dominated; ++j)
            if (points[sl[j]]->dominates(pt))
                dominated = 1;

        if (!dominated)
        {
            // eliminate any points in current skyline that it dominates
            m = index;//number of current points
            index = 0;
            for (j = 0; j < m; ++j)
                if (!pt->dominates(points[sl[j]]))
                    sl[index++] = sl[j];

            // add this point as well
            sl[index++] = i;
        }
    }
    //record skyline points
    point_set* skyline = new point_set();
    point_t *p;
    for (int i = 0; i < index; ++i)
    {
        p = new point_t(points[sl[i]]);
        skyline->points.push_back(p);
    }

    delete[] sl;
    return skyline;
}

/**
 * @brief Prune point p in the point set
 * @param p     The point
 */
void point_set::prune(point_t *p)
{
    int M = points.size();
    for(int i = 0; i < M; ++i)
    {
        if(p->id == points[i]->id)
        {
            points.erase(points.begin() + i);
            return;
        }
    }
}


void write_point(FILE *rPtr, point_t* p, int d_num, int a[10])
{
    int dim=p->d;
    if(d_num == dim-1)
    {
        double sum=0;
        for(int j=0; j<dim;j++)
            sum+=a[j];
        if(sum!=dim&&sum!=0)
        {
            //printf("%d %d %d %d\n", a[0], a[1], a[2], a[3]);
            for(int j=0; j<dim;j++)
            {
                if(a[j]==0)
                {
                    fprintf(rPtr, "%lf ", 0.0);
                    //printf("%lf ", 0.0);
                }
                else
                {
                    fprintf(rPtr, "%lf ", p->attr[j]);
                    //printf("%lf ", p->coord[j]);
                }
            }
            fprintf(rPtr, "\n");
            //printf("\n");
        }
    }
    else
    {
        a[d_num+1] = 0;
        write_point(rPtr, p, d_num + 1, a);
        a[d_num+1] = 1;
        write_point(rPtr, p, d_num + 1, a);
    }
}




