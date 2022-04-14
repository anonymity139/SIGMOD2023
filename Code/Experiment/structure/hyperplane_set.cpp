#include "hyperplane_set.h"
#include "../Others/lp.h"
#include "../Others/pruning.h"
#include <iostream>
#include <fstream>
#include <sys/time.h>
#define Precision 0.01

/**
 * @brief Constructor
 */
hyperplane_set::hyperplane_set(){}

/**
 * @brief           Constructor: initial some hyperplanes so that u[i]>=0
 * @param dim       The dimensionality
 * @param initial   Signal: initialization
 */
hyperplane_set::hyperplane_set(int dim, double u_range)
{
    if(dim > 1)
    {
        hyperplane *h;
        for (int i = 0; i < dim; ++i)
        {
            h = new hyperplane(dim);
            for (int j = 0; j < dim; ++j)
            {
                if (j == i)
                    h->norm[j] = -1;
                else
                    h->norm[j] = 0;
            }
            h->offset = 0;
            hyperplanes.push_back(h);
        }

        h = new hyperplane(dim);
        for (int i = 0; i < dim; ++i)
            h->norm[i] = 1;
        h->offset = -u_range;
        hyperplanes.push_back(h);

        //set extreme points
        this->set_ext_pts(u_range);
    }
    else
    {
        point_t *p = new point_t(dim);
        for (int i = 0; i < dim; ++i)
            p->attr[i] = u_range;
        ext_pts.push_back(p);
    }
}


hyperplane_set::hyperplane_set(int d_num, int dt_cat, double u_range)
{
    int dim = d_num + dt_cat;
    hyperplane *h;
    //-x[i] <= 0
    for (int i = 0; i < dim; ++i)
    {
        h = new hyperplane(dim);
        for (int j = 0; j < dim; ++j)
        {
            if (j == i)
                h->norm[j] = -1;
            else
                h->norm[j] = 0;
        }
        h->offset = 0;
        hyperplanes.push_back(h);
    }

    h = new hyperplane(dim);
    for(int i = 0; i < dim; ++i)
        h->norm[i] = 1;
    h->offset = - U_RANGE;
    hyperplanes.push_back(h);


    //sum_{d_num} x[i] - 1 <=0
    /*
    h = new hyperplane(dim);
    for(int i = 0; i < dim; ++i)
    {
        if (i < dt_cat)
            h->norm[i] = 0;
        else
            h->norm[i] = 1;
    }
    h->offset = - u_range;
    hyperplanes.push_back(h);

    //sum_{d_num} x[i] - 1 >=0
    h = new hyperplane(dim);
    for(int i = 0; i < dim; ++i)
    {
        if (i < dt_cat)
            h->norm[i] = 0;
        else
            h->norm[i] = -1;
    }
    h->offset = u_range;
    hyperplanes.push_back(h);

    //x[dt_cat] <= U_RANGE

    for(int i = 0; i < dt_cat; ++i)
    {
        h = new hyperplane(dim);
        for(int j = 0; j < dim; ++j)
        {
            if (i == j)
                h->norm[j] = 1;
            else
                h->norm[j] = 0;
        }
        h->offset = -U_RANGE;
        hyperplanes.push_back(h);
    }
    */

}


/*
 * @brief Destructor
 *        Delete the hyperplanes and extreme points
 */
hyperplane_set::~hyperplane_set()
{
    //delete the hyperplanes
    int i = hyperplanes.size();
    while(i > 0)
    {
        delete hyperplanes[i-1];
        i--;
    }
    hyperplanes.clear();

    //delete the extreme points
    i = ext_pts.size();
    while(i > 0)
    {
        delete ext_pts[i-1];
        i--;
    }
    ext_pts.clear();

}


/**
    * @brief Prepare the file for computing the convex hull (the utility range R)
    *        via halfspace interaction
    * @param feasible_pt   A points inside R
    * @param filename      The name of the file written
    */
void hyperplane_set::write(point_t *feasible_pt, char *filename)
{
    //char filename[MAX_FILENAME_LENG];
    int dim = feasible_pt->d, size = hyperplanes.size();
    ofstream wPtr;
    wPtr.open(filename, std::ios::out);
    wPtr.setf(ios::fixed, ios::floatfield);  // set as fixed model
    wPtr.precision(6);  // 设置精度 2

    // write the feasible point
    wPtr << dim <<"\n" << 1 <<"\n";
    for(int i = 0; i < dim; i++)
        wPtr << feasible_pt->attr[i] << " ";
    wPtr << "\n";

    // write the hyperplane(ux<0)
    wPtr << dim + 1 <<"\n" << size << "\n";//record the offset as one dimension
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < dim; j++)
        {
            wPtr << 100 * hyperplanes[i]->norm[j] <<" ";
        }
        wPtr << 100 * hyperplanes[i]->offset <<"\n";
    }

    wPtr.close();
}

/**
 * @brief   Print the information of the hyperplane set
 *          Including hyperplanes and extreme points of the intersection(convex hull)
 */
void hyperplane_set::print()
{
    if(hyperplanes.size() <= 0)
        return ;
    //print hyperplanes
    for (int i = 0; i < hyperplanes.size(); i++)
    {
        printf("hyperplane ");
        hyperplanes[i]->print();
    }
    //print extreme points
    int dim = hyperplanes[0]->d;
    for (int i = 0; i < ext_pts.size(); i++)
    {
        printf("point ");
        for (int j = 0; j < dim; j++)
        {
            printf("%lf ", ext_pts[i]->attr[j]);
        }
        printf("\n");
    }
}

/**
 * @brief   Check whether the intersection of the hyperplane exists
 *          Set the extreme points of the hyperplane set.
 *          Refine the bounded hyperplanes
 * @param   u_range     The sum of the numerical attributes u[i] should be u_range
 * @return  Whether R is updated
 *          true  R is the different
 *          false R is the same as before
 */
void hyperplane_set::set_ext_pts(double u_range)
{
    int M = hyperplanes.size(), size = 0;
    if (M <= 0)
    {
        printf("%s\n", "Error: No hyperplane in the set.");
        return ;
    }
    int dim = hyperplanes[0]->d;
    char file1[MAX_FILENAME_LENG];
    sprintf(file1, "../output/hyperplane_data.txt");
    char file2[MAX_FILENAME_LENG];
    sprintf(file2, "../output/ext_pt.txt");
    point_t *feasible_pt = find_feasible(hyperplanes);//the feasible point

    if (feasible_pt == NULL)
    {
        cout<<"The intersection is infeasible.\n";
        hyperplanes.pop_back();
        return;
    }
    write(feasible_pt, file1);//write hyperplanes and the feasible point to file1,

    //conduct half space intersection and write results to file2
    FILE *rPtr, *wPtr;
    if ((rPtr = fopen(file1, "r")) == NULL)
    {
        fprintf(stderr, "Cannot open the data file.\n");
        exit(0);
    }
    wPtr = (FILE *) fopen(file2, "w");
    halfspace(rPtr, wPtr);
    fclose(rPtr);
    fclose(wPtr);

    //read extreme points in file2
    if ((rPtr = fopen(file2, "r")) == NULL)
    {
        fprintf(stderr, "Cannot open the data file %s.\n", file2);
        exit(0);
    }

    //update the set of extreme points

    fscanf(rPtr, "%i%i", &dim, &size);
    if(size > 0)
    {
        //delete all the original extreme points
        std::vector<point_t *> pset;
        while (ext_pts.size() > 0)
        {
            point_t *pt = ext_pts[ext_pts.size() - 1];
            ext_pts.pop_back();
            delete pt;
        }
        ext_pts.clear();

        //input new extreme points
        for (int i = 0; i < size; ++i)
        {
            bool allZero = true;
            point_t *p = new point_t(dim);
            for (int j = 0; j < dim; ++j)
            {
                fscanf(rPtr, "%lf", &p->attr[j]);
                if (!isZero(p->attr[j]))
                    allZero = false;
            }
            if (allZero)
                delete p;
            else
                ext_pts.push_back(p);
        }

        // update the set of hyperplanes
        fscanf(rPtr, "%i", &size);
        int *hid = new int[size + 1];
        for (int i = 1; i <= size; ++i)
            fscanf(rPtr, "%i", &hid[i]);
        sort(hid, hid + size + 1);
        for (int i = M - 1, count = size; i > -1; --i)
        {
            if (hid[count] < i)
                hyperplanes.erase(hyperplanes.begin() + i);
            else
                --count;
        }
        delete[]hid;
    }
    else
        hyperplanes.pop_back();
    fclose(rPtr);

    //check whether the ext_pts has been renewed
    /*if(pset.size() != ext_pts.size())
    {
        point_t *pt;
        while (pset.size() > 0)
        {
            pt = pset[pset.size()-1];
            pset.pop_back();
            delete pt;
        }
        return true;
    }
    else
    {
        for(int i = 0; i < pset.size(); ++i)
        {
            bool exist = false;
            for(int j = 0; j < ext_pts.size(); j++)
            {
                if(pset[i]->is_same(ext_pts[j]))
                {
                    exist = true;
                    break;
                }
            }
            if(!exist)
            {
                point_t *pt;
                while (pset.size() > 0)
                {
                    pt = pset[pset.size()-1];
                    pset.pop_back();
                    delete pt;
                }
                return true;
            }
        }
    }
    point_t *pt;
    while (pset.size() > 0)
    {
        pt = pset[pset.size()-1];
        pset.pop_back();
        delete pt;
    }
    return false;
    */
}


/**
 * @brief   Find the convex points in the set
 * @param top   Record all the convex points
 */
void hyperplane_set::find_boundary(point_set *p_set, double u_range)
{
    int dim = p_set->points[0]->d;
    int M = p_set->points.size(), original_size  = p_set->points.size();
    FILE *rPtr = NULL, *wPtr = NULL;
    rPtr = (FILE *) fopen("../output/point.txt", "w");
    // write the extreme point
    fprintf(rPtr, "%i\n%i\n", dim, M+1);
    for (int i = 0; i < M; i++)
    {
        for (int j = 0; j < dim; j++)
        {
            fprintf(rPtr, "%lf ", p_set->points[i]->attr[j]);
        }
        fprintf(rPtr, "\n");
    }
    for(int j=0; j<dim;j++)
        fprintf(rPtr, "%lf ", 0.0);
    fprintf(rPtr, "\n");
    fclose(rPtr);

    rPtr = (FILE *) fopen("../output/point.txt", "r");
    wPtr = (FILE *) fopen("../output/top.txt", "w");
    Convex_H(rPtr, wPtr);
    fclose(rPtr);
    fclose(wPtr);

    //obtain the points
    wPtr = (FILE *) fopen("../output/top.txt", "r");
    fscanf(wPtr, "%i", &M);
    int index;
    for (int i = 0; i < M; ++i)
    {
        fscanf(wPtr, "%i", &index);
        if(index < original_size)
            ext_pts.push_back(p_set->points[index]);
    }

    //obtain the hyperplane
    fscanf(wPtr, "%i%i", &dim, &M);
    for (int i = 0; i < M; ++i)
    {
        hyperplane *h = new hyperplane(dim - 1);
        for(int j = 0; j < dim -1; ++j)
            fscanf(wPtr, "%lf", &h->norm[j]);
        fscanf(wPtr, "%lf", &h->offset);
        if(h->offset < 1 && h->offset > -1)
        {
            hyperplanes.push_back(h);
        }
    }
    //add the boundary
    hyperplane *h = new hyperplane(dim - 1);
    for(int j = 0; j < dim -1; ++j)
        h->norm[j] = 1;
    h->offset = -u_range;
    hyperplanes.push_back(h);
    fclose(wPtr);
}



/**
 * @brief Check the relation between the hyperplane and the intersection of the hyperplane set
 *        Since the extreme points of the half_set can not be accurate enough, we set "Precision" to solve the error
 * @param h     The hyperplane
 * @return      1: half_set on the positive side of the hyperplane
 *              -1: half_set on the negative side of the hyperplane
 *              0: half_set intersects with the hyperplane
 *              -2: Error for check situation
 */
int hyperplane_set::check_relation(hyperplane *h)
{
    int M = ext_pts.size();
    if (M < 1)
    {
        printf("%s\n", "None of the ext_pts in the set.");
        return -2;
    }
    int dim = ext_pts[0]->d;
    int positive = 0, negative = 0;
    for (int i = 0; i < M; i++)
    {
        double sum = 0;
        for (int j = 0; j < dim; j++)
        {
            sum += (h->norm[j] * ext_pts[i]->attr[j]);
        }
        if (sum < -Precision)
        {
            negative++;
            break;
        }
    }
    if(negative == 0)
        return 1;
    for (int i = 0; i < M; i++)
    {
        double sum = 0;
        for (int j = 0; j < dim; j++)
        {
            sum += (h->norm[j] * ext_pts[i]->attr[j]);
        }
        if (sum > Precision)
        {
            positive++;
            break;
        }
    }
    if (positive > 0 && negative > 0)
    {
        return 0;
    }
    //printf("%s\n", "Error for check situation.");
    return -1;
}

/**
 * @brief Check whether the intersection of the hyperplane set is on the positive side of the hyperplane
 *        Since the extreme points of the half_set can not be accurate enough, we set "Precision" to solve the error
 * @param h     The hyperplane
 * @return      1: half_set on the positive side of the hyperplane
 *              0: half_set on the negative side of the hyperplane/intersects with the hyperplane
 */
int hyperplane_set::check_relation_positive(hyperplane *h)
{
    int M = ext_pts.size();
    if (M < 1)
    {
        printf("%s\n", "None of the ext_pts in the set.");
        return -2;
    }
    int dim = ext_pts[0]->d;
    for (int i = 0; i < M; i++)
    {
        double sum = 0;
        for (int j = 0; j < dim; j++)
        {
            sum += (h->norm[j] * ext_pts[i]->attr[j]);
        }
        if (sum < -Precision / 2)
        {
            return 0;
        }
    }
    return 1;
}

/**
 * @brief Check whether v could be a top-1 vector w.r.t R
 * @param pset  Point set
 * @param v     Vector
 * @return      1 v could be a top-1 vector
 *              0 v could not be a top-1 vector
 */
bool hyperplane_set::is_top_geq(std::vector<double *> &pset, double *v)
{
    int p_size = pset.size(), h_size = hyperplanes.size(), dim = hyperplanes[0]->d;
    char file1[MAX_FILENAME_LENG];
    sprintf(file1, "../output/hyperplane_data.txt");
    char file2[MAX_FILENAME_LENG];
    sprintf(file2, "../output/ext_pt.txt");

    //construct the hyperplanes and a feasible point
    hyperplane_set *h_set = new hyperplane_set();
    hyperplane *h;
    for (int i = 0; i < h_size; ++i)
    {
        h = new hyperplane(dim);
        for (int j = 0; j < dim; j++)
            h->norm[j] = hyperplanes[i]->norm[j];
        h->offset = hyperplanes[i]->offset;
        h_set->hyperplanes.push_back(h);
    }
    //note all the inequality are <=
    for (int i = 0; i < p_size; i++)
    {
        h = new hyperplane(dim);
        for (int j = 0; j < dim; j++)
            h->norm[j] = pset[i][j] - v[j] - EDGE;
        h->offset = 0;
        h_set->hyperplanes.push_back(h);
    }
    //the feasible point
    point_t *feasible_pt = find_feasible(h_set->hyperplanes);
    delete h_set;
    if (feasible_pt == NULL)
        return false;
    else
        return true;
}

bool hyperplane_set::is_top_geq(std::vector<std::pair<double *, int>> &pset, double *v)
{
    int p_size = pset.size(), h_size = hyperplanes.size(), dim = hyperplanes[0]->d;
    char file1[MAX_FILENAME_LENG];
    sprintf(file1, "../output/hyperplane_data.txt");
    char file2[MAX_FILENAME_LENG];
    sprintf(file2, "../output/ext_pt.txt");

    //construct the hyperplanes and a feasible point
    hyperplane_set *h_set = new hyperplane_set();
    hyperplane *h;
    for (int i = 0; i < h_size; ++i)
    {
        h = new hyperplane(dim);
        for (int j = 0; j < dim; j++)
            h->norm[j] = hyperplanes[i]->norm[j];
        h->offset = hyperplanes[i]->offset;
        h_set->hyperplanes.push_back(h);
    }
    //note all the inequality are <=
    for (int i = 0; i < p_size; i++)
    {
        h = new hyperplane(dim);
        for (int j = 0; j < dim; j++)
            h->norm[j] = pset[i].first[j] - v[j] - EDGE;
        h->offset = 0;
        h_set->hyperplanes.push_back(h);
    }
    h_set->print();
    //the feasible point
    point_t *feasible_pt = find_feasible(h_set->hyperplanes);
    delete h_set;
    if (feasible_pt == NULL)
        return false;
    else
        return true;
}

bool hyperplane_set::is_top_leq(std::vector<double *> &pset, double *v)
{
    int p_size = pset.size(), h_size = hyperplanes.size(), dim = hyperplanes[0]->d;
    char file1[MAX_FILENAME_LENG];
    sprintf(file1, "../output/hyperplane_data.txt");
    char file2[MAX_FILENAME_LENG];
    sprintf(file2, "../output/ext_pt.txt");

    //construct the hyperplanes and a feasible point
    hyperplane_set *h_set = new hyperplane_set();
    hyperplane *h;
    for (int i = 0; i < h_size; ++i)
    {
        h = new hyperplane(dim);
        for (int j = 0; j < dim; j++)
            h->norm[j] = hyperplanes[i]->norm[j];
        h->offset = hyperplanes[i]->offset;
        h_set->hyperplanes.push_back(h);
    }
    //note all the inequality are <=
    for (int i = 0; i < p_size; i++)
    {
        h = new hyperplane(dim);
        for (int j = 0; j < dim; j++)
            h->norm[j] = -pset[i][j] + v[j] - EDGE;
        h->offset = 0;
        h_set->hyperplanes.push_back(h);
    }
    //the feasible point
    point_t *feasible_pt = find_feasible(h_set->hyperplanes);
    delete h_set;
    if (feasible_pt == NULL)
        return false;
    else
        return true;
}

bool hyperplane_set::is_top_leq(std::vector<std::pair<double *, int>> &pset, double *v)
{
    int p_size = pset.size(), h_size = hyperplanes.size(), dim = hyperplanes[0]->d;
    char file1[MAX_FILENAME_LENG];
    sprintf(file1, "../output/hyperplane_data.txt");
    char file2[MAX_FILENAME_LENG];
    sprintf(file2, "../output/ext_pt.txt");

    //construct the hyperplanes and a feasible point
    hyperplane_set *h_set = new hyperplane_set();
    hyperplane *h;
    for (int i = 0; i < h_size; ++i)
    {
        h = new hyperplane(dim);
        for (int j = 0; j < dim; j++)
            h->norm[j] = hyperplanes[i]->norm[j];
        if(hyperplanes[i]->offset >= 0.00001 || hyperplanes[i]->offset<=-0.00001)
            h->offset = hyperplanes[i]->offset;
        else
            h->offset = 0;
        h_set->hyperplanes.push_back(h);
    }
    //point_t *feasible_pt1 = find_feasible(h_set->hyperplanes);
    //note all the inequality are <=
    for (int i = 0; i < p_size; i++)
    {
        h = new hyperplane(dim);
        for (int j = 0; j < dim; j++)
            h->norm[j] = - pset[i].first[j] + v[j] - EDGE;
        h->offset = 0;
        h_set->hyperplanes.push_back(h);
        //point_t *feasible_pt2 = find_feasible(h_set->hyperplanes);

    }
    //h_set->print();
    //the feasible point
    point_t *feasible_pt = find_feasible(h_set->hyperplanes);
    //feasible_pt->print();
    delete h_set;
    if (feasible_pt == NULL)
        return false;
    else
        return true;
}

/**
 * @brief Check whether v could be a top-1(only) vector w.r.t R
 *        Used with extreme points of R
 * @param pset  Point set
 * @param v     Vector
 * @return      1 v could be a top-1 vector
 *              0 v could not be a top-1 vector
 */
double hyperplane_set::top_leq_check(std::vector<inequality> &pset, double *v)
{
    int M = pset.size(), K = ext_pts.size(), dim = ext_pts[0]->d;
    if(M < 1)
        return 100;
    int *ia = new int[(dim + M + 1) * (dim + K + 1) + 1];  //TODO: delete
    int *ja = new int[(dim + M + 1) * (dim + K + 1) + 1];  //TODO: delete
    double* ar = new double[(dim + M + 1) * (dim + K + 1) + 1];   //TODO: delete

    glp_prob *lp;
    lp = glp_create_prob();
    glp_set_prob_name(lp, "is_topl");
    glp_set_obj_dir(lp, GLP_MAX);

    // add dim+1 rows: q_1, ..., q_{dim+M+1}
    glp_add_rows(lp, dim + M + 1);
    for (int i = 1; i <= dim; ++i) {
        char buf[10];
        sprintf(buf, "q%d", i);
        glp_set_row_name(lp, i, buf);
        glp_set_row_bnds(lp, i, GLP_FX, 0.0, 0.0); // q_i
    }
    for (int i = dim + 1; i <= dim + M; ++i) {
        char buf[10];
        sprintf(buf, "q%d", i);
        glp_set_row_name(lp, i, buf);
        glp_set_row_bnds(lp, i, GLP_UP, 0.0, 0.0); // q_i
    }
    char buf[10];
    sprintf(buf, "q%d", dim + M + 1);
    glp_set_row_name(lp, dim + M + 1, buf);
    glp_set_row_bnds(lp, dim + M + 1, GLP_FX, 1.0, 1.0); // q_{dim+1} = 1

    // add M columns: v_1, ..., v_{dim + M +1}
    glp_add_cols(lp, dim + K + 1);
    for (int i = 1; i <= dim + K; ++i) {
        char buf[10];
        sprintf(buf, "v%d", i);
        glp_set_col_name(lp, i, buf);
        glp_set_col_bnds(lp, i, GLP_LO, 0.0, 0.0); // 0 <= v[i] < infty
        glp_set_obj_coef(lp, i, 0.0);  // objective: 0
    }
    char buff[10];
    sprintf(buff, "v%d", dim + K + 1);
    glp_set_col_name(lp, dim + K + 1, buff);
    glp_set_col_bnds(lp, dim + K + 1, GLP_FR, 0.0, 0.0); // -infty < v[i] < infty
    glp_set_obj_coef(lp, dim + K + 1, 1.0);  // objective: 0


    int counter = 1;
    // set coeffcient on each row
    for (int i = 1; i <= dim; ++i) //1, 2, ..., dim
    {
        for (int j = 1; j <= K; ++j)
        {
            ia[counter] = i; ja[counter] = j;
            ar[counter++] = ext_pts[j-1]->attr[i-1];
        }
        for (int j = K + 1; j <= K + dim; ++j)
        {
            ia[counter] = i; ja[counter] = j;
            if(i == j - K)
                ar[counter++] = -1;
            else
                ar[counter++] = 0;
        }
        ia[counter] = i; ja[counter] = dim + K + 1;
        ar[counter++] = 0;
    }
    for (int i = dim + 1; i <= dim + M; ++i)//dim+1, dim+2, ..., dim+M
    {
        for (int j = 1; j <= K; ++j)
        {
            ia[counter] = i; ja[counter] = j;
            ar[counter++] = 0;
        }
        for (int j = K + 1; j <= K + dim; ++j)
        {
            ia[counter] = i; ja[counter] = j;
            ar[counter++] = v[j-K-1] - pset[i-dim-1].norm[j-K-1];
        }
        ia[counter] = i; ja[counter] = dim + K + 1;
        ar[counter++] = 1;
    }
    //dim+M+1
    for (int j = 1; j <= K; ++j)
    {
        ia[counter] = dim + M + 1; ja[counter] = j;
        ar[counter++] = 1;
    }
    for (int j = K + 1; j <= K + dim + 1; ++j)
    {
        ia[counter] = dim + M + 1; ja[counter] = j;
        ar[counter++] = 0;
    }

    // loading data
    glp_load_matrix(lp, counter - 1, ia, ja, ar);
    // running simplex
    glp_smcp parm;
    glp_init_smcp(&parm);
    parm.msg_lev = GLP_MSG_OFF; // turn off all message by glp_simplex
    glp_simplex(lp, &parm);

    double val = glp_get_obj_val(lp);
    glp_delete_prob(lp); // clean up
    delete[]ia;
    delete[]ja;
    delete[]ar;
    return val;
}

/**
 * @brief Check whether v could be a top-1(only) vector w.r.t R
 *        Used with extreme points of R
 * @param pset  Point set
 * @param v     Vector
 * @return      1 v could be a top-1 vector
 *              0 v could not be a top-1 vector
 */
double hyperplane_set::top_leq_check_threshold(std::vector<inequality> &pset, double *v, double &utility)
{
    int M = pset.size(), K = ext_pts.size(), dim = ext_pts[0]->d;
    if(M < 1)
        return 100;
    int *ia = new int[(dim + M + 1) * (dim + K + 1) + 1];  //TODO: delete
    int *ja = new int[(dim + M + 1) * (dim + K + 1) + 1];  //TODO: delete
    double* ar = new double[(dim + M + 1) * (dim + K + 1) + 1];   //TODO: delete

    glp_prob *lp;
    lp = glp_create_prob();
    glp_set_prob_name(lp, "is_topl");
    glp_set_obj_dir(lp, GLP_MAX);

    // add dim+1 rows: q_1, ..., q_{dim+M+1}
    glp_add_rows(lp, dim + M + 1);
    for (int i = 1; i <= dim; ++i) {
        char buf[10];
        sprintf(buf, "q%d", i);
        glp_set_row_name(lp, i, buf);
        glp_set_row_bnds(lp, i, GLP_FX, 0.0, 0.0); // q_i
    }
    for (int i = dim + 1; i <= dim + M; ++i) {
        char buf[10];
        sprintf(buf, "q%d", i);
        glp_set_row_name(lp, i, buf);
        glp_set_row_bnds(lp, i, GLP_UP, 0.0, 0.0); // q_i
    }
    char buf[10];
    sprintf(buf, "q%d", dim + M + 1);
    glp_set_row_name(lp, dim + M + 1, buf);
    glp_set_row_bnds(lp, dim + M + 1, GLP_FX, 1.0, 1.0); // q_{dim+1} = 1

    // add M columns: v_1, ..., v_{dim + M +1}
    glp_add_cols(lp, dim + K + 1);
    for (int i = 1; i <= dim + K; ++i) {
        char buf[10];
        sprintf(buf, "v%d", i);
        glp_set_col_name(lp, i, buf);
        glp_set_col_bnds(lp, i, GLP_LO, 0.0, 0.0); // 0 <= v[i] < infty
        glp_set_obj_coef(lp, i, 0.0);  // objective: 0
    }
    char buff[10];
    sprintf(buff, "v%d", dim + K + 1);
    glp_set_col_name(lp, dim + K + 1, buff);
    glp_set_col_bnds(lp, dim + K + 1, GLP_FR, 0.0, 0.0); // -infty < v[i] < infty
    glp_set_obj_coef(lp, dim + K + 1, 1.0);  // objective: 0


    int counter = 1;
    // set coeffcient on each row
    for (int i = 1; i <= dim; ++i) //1, 2, ..., dim
    {
        for (int j = 1; j <= K; ++j)
        {
            ia[counter] = i; ja[counter] = j;
            ar[counter++] = ext_pts[j-1]->attr[i-1];
        }
        for (int j = K + 1; j <= K + dim; ++j)
        {
            ia[counter] = i; ja[counter] = j;
            if(i == j - K)
                ar[counter++] = -1;
            else
                ar[counter++] = 0;
        }
        ia[counter] = i; ja[counter] = dim + K + 1;
        ar[counter++] = 0;
    }
    for (int i = dim + 1; i <= dim + M; ++i)//dim+1, dim+2, ..., dim+M
    {
        for (int j = 1; j <= K; ++j)
        {
            ia[counter] = i; ja[counter] = j;
            ar[counter++] = 0;
        }
        for (int j = K + 1; j <= K + dim; ++j)
        {
            ia[counter] = i; ja[counter] = j;
            ar[counter++] = v[j-K-1] - pset[i-dim-1].norm[j-K-1];
        }
        ia[counter] = i; ja[counter] = dim + K + 1;
        ar[counter++] = 1;
    }
    //dim+M+1
    for (int j = 1; j <= K; ++j)
    {
        ia[counter] = dim + M + 1; ja[counter] = j;
        ar[counter++] = 1;
    }
    for (int j = K + 1; j <= K + dim + 1; ++j)
    {
        ia[counter] = dim + M + 1; ja[counter] = j;
        ar[counter++] = 0;
    }

    // loading data
    glp_load_matrix(lp, counter - 1, ia, ja, ar);
    // running simplex
    glp_smcp parm;
    glp_init_smcp(&parm);
    parm.msg_lev = GLP_MSG_OFF; // turn off all message by glp_simplex
    glp_simplex(lp, &parm);

    double val = glp_get_obj_val(lp);
    utility = 0;
    if(val > EQN2)
    {
        double *para = new double[K], *uv = new double[dim];
        for (int i = 0; i < K; ++i)
            para[i] = glp_get_col_prim(lp, i + 1);
        for (int i = 0; i < dim; ++i)
        {
            uv[i] = 0;
            for (int j = 0; j < K; ++j)
                uv[i] += para[j] * ext_pts[j]->attr[i];
            utility += uv[i] * v[i];
        }
    }
    glp_delete_prob(lp); // clean up
    delete[]ia;
    delete[]ja;
    delete[]ar;
    return val;
}


/**
 * @brief Check whether v could be a top-1(only) vector w.r.t R
 *        Used with extreme points of R
 * @param pset  Point set
 * @param v     Vector
 * @return      1 v could be a top-1 vector
 *              0 v could not be a top-1 vector
 */
double hyperplane_set::top_geq_check(std::vector<inequality> &pset, double *v)
{
    int M = pset.size(), K = ext_pts.size(), dim = ext_pts[0]->d;
    if(M < 1)
        return 100;
    int *ia = new int[(dim + M + 1) * (dim + K + 1) + 1];  //TODO: delete
    int *ja = new int[(dim + M + 1) * (dim + K + 1) + 1];  //TODO: delete
    double* ar = new double[(dim + M + 1) * (dim + K + 1) + 1];   //TODO: delete

    glp_prob *lp;
    lp = glp_create_prob();
    glp_set_prob_name(lp, "is_topg");
    glp_set_obj_dir(lp, GLP_MAX);

    // add dim+1 rows: q_1, ..., q_{dim+M+1}
    glp_add_rows(lp, dim + M + 1);
    for (int i = 1; i <= dim; ++i) {
        char buf[10];
        sprintf(buf, "q%d", i);
        glp_set_row_name(lp, i, buf);
        glp_set_row_bnds(lp, i, GLP_FX, 0.0, 0.0); // q_i
    }
    for (int i = dim + 1; i <= dim + M; ++i) {
        char buf[10];
        sprintf(buf, "q%d", i);
        glp_set_row_name(lp, i, buf);
        glp_set_row_bnds(lp, i, GLP_UP, 0.0, 0.0); // q_i
    }
    char buf[10];
    sprintf(buf, "q%d", dim + M + 1);
    glp_set_row_name(lp, dim + M + 1, buf);
    glp_set_row_bnds(lp, dim + M + 1, GLP_FX, 1.0, 1.0); // q_{dim+1} = 1

    // add M columns: v_1, ..., v_{dim + M +1}
    glp_add_cols(lp, dim + K + 1);
    for (int i = 1; i <= dim + K; ++i) {
        char buf[10];
        sprintf(buf, "v%d", i);
        glp_set_col_name(lp, i, buf);
        glp_set_col_bnds(lp, i, GLP_LO, 0.0, 0.0); // 0 <= v[i] < infty
        glp_set_obj_coef(lp, i, 0.0);  // objective: 0
    }
    char buff[10];
    sprintf(buff, "v%d", dim + K + 1);
    glp_set_col_name(lp, dim + K + 1, buff);
    glp_set_col_bnds(lp, dim + K + 1, GLP_FR, 0.0, 0.0); // -infty < v[i] < infty
    glp_set_obj_coef(lp, dim + K + 1, 1.0);  // objective: 0


    int counter = 1;
    // set coeffcient on each row
    for (int i = 1; i <= dim; ++i) //1, 2, ..., dim
    {
        for (int j = 1; j <= K; ++j)
        {
            ia[counter] = i; ja[counter] = j;
            ar[counter++] = ext_pts[j-1]->attr[i-1];
        }
        for (int j = K + 1; j <= K + dim; ++j)
        {
            ia[counter] = i; ja[counter] = j;
            if(i == j - K)
                ar[counter++] = -1;
            else
                ar[counter++] = 0;
        }
        ia[counter] = i; ja[counter] = dim + K + 1;
        ar[counter++] = 0;
    }
    for (int i = dim + 1; i <= dim + M; ++i)//dim+1, dim+2, ..., dim+M
    {
        for (int j = 1; j <= K; ++j)
        {
            ia[counter] = i; ja[counter] = j;
            ar[counter++] = 0;
        }
        for (int j = K + 1; j <= K + dim; ++j)
        {
            ia[counter] = i; ja[counter] = j;
            ar[counter++] = pset[i-dim-1].norm[j-K-1] - v[j-K-1];
        }
        ia[counter] = i; ja[counter] = dim + K + 1;
        ar[counter++] = 1;
    }
    //dim+M+1
    for (int j = 1; j <= K; ++j)
    {
        ia[counter] = dim + M + 1; ja[counter] = j;
        ar[counter++] = 1;
    }
    for (int j = K + 1; j <= K + dim + 1; ++j)
    {
        ia[counter] = dim + M + 1; ja[counter] = j;
        ar[counter++] = 0;
    }

    // loading data
    glp_load_matrix(lp, counter - 1, ia, ja, ar);
    // running simplex
    glp_smcp parm;
    glp_init_smcp(&parm);
    parm.msg_lev = GLP_MSG_OFF; // turn off all message by glp_simplex
    glp_simplex(lp, &parm);

    double val = glp_get_obj_val(lp);
    glp_delete_prob(lp); // clean up
    delete[]ia;
    delete[]ja;
    delete[]ar;
    return val;
}

/**
 * @brief Check whether v could be a top-1(only) vector w.r.t R
 *        Used with extreme points of R
 * @param pset  Point set
 * @param v     Vector
 * @return      1 v could be a top-1 vector
 *              0 v could not be a top-1 vector
 */
double hyperplane_set::top_geq_check_threshold(std::vector<inequality> &pset, double *v, double &utility)
{
    int M = pset.size(), K = ext_pts.size(), dim = ext_pts[0]->d;
    if(M < 1)
        return 100;
    int *ia = new int[(dim + M + 1) * (dim + K + 1) + 1];  //TODO: delete
    int *ja = new int[(dim + M + 1) * (dim + K + 1) + 1];  //TODO: delete
    double* ar = new double[(dim + M + 1) * (dim + K + 1) + 1];   //TODO: delete

    glp_prob *lp;
    lp = glp_create_prob();
    glp_set_prob_name(lp, "is_topg");
    glp_set_obj_dir(lp, GLP_MAX);

    // add dim+1 rows: q_1, ..., q_{dim+M+1}
    glp_add_rows(lp, dim + M + 1);
    for (int i = 1; i <= dim; ++i) {
        char buf[10];
        sprintf(buf, "q%d", i);
        glp_set_row_name(lp, i, buf);
        glp_set_row_bnds(lp, i, GLP_FX, 0.0, 0.0); // q_i
    }
    for (int i = dim + 1; i <= dim + M; ++i) {
        char buf[10];
        sprintf(buf, "q%d", i);
        glp_set_row_name(lp, i, buf);
        glp_set_row_bnds(lp, i, GLP_UP, 0.0, 0.0); // q_i
    }
    char buf[10];
    sprintf(buf, "q%d", dim + M + 1);
    glp_set_row_name(lp, dim + M + 1, buf);
    glp_set_row_bnds(lp, dim + M + 1, GLP_FX, 1.0, 1.0); // q_{dim+1} = 1

    // add M columns: v_1, ..., v_{dim + M +1}
    glp_add_cols(lp, dim + K + 1);
    for (int i = 1; i <= dim + K; ++i) {
        char buf[10];
        sprintf(buf, "v%d", i);
        glp_set_col_name(lp, i, buf);
        glp_set_col_bnds(lp, i, GLP_LO, 0.0, 0.0); // 0 <= v[i] < infty
        glp_set_obj_coef(lp, i, 0.0);  // objective: 0
    }
    char buff[10];
    sprintf(buff, "v%d", dim + K + 1);
    glp_set_col_name(lp, dim + K + 1, buff);
    glp_set_col_bnds(lp, dim + K + 1, GLP_FR, 0.0, 0.0); // -infty < v[i] < infty
    glp_set_obj_coef(lp, dim + K + 1, 1.0);  // objective: 0


    int counter = 1;
    // set coeffcient on each row
    for (int i = 1; i <= dim; ++i) //1, 2, ..., dim
    {
        for (int j = 1; j <= K; ++j)
        {
            ia[counter] = i; ja[counter] = j;
            ar[counter++] = ext_pts[j-1]->attr[i-1];
        }
        for (int j = K + 1; j <= K + dim; ++j)
        {
            ia[counter] = i; ja[counter] = j;
            if(i == j - K)
                ar[counter++] = -1;
            else
                ar[counter++] = 0;
        }
        ia[counter] = i; ja[counter] = dim + K + 1;
        ar[counter++] = 0;
    }
    for (int i = dim + 1; i <= dim + M; ++i)//dim+1, dim+2, ..., dim+M
    {
        for (int j = 1; j <= K; ++j)
        {
            ia[counter] = i; ja[counter] = j;
            ar[counter++] = 0;
        }
        for (int j = K + 1; j <= K + dim; ++j)
        {
            ia[counter] = i; ja[counter] = j;
            ar[counter++] = pset[i-dim-1].norm[j-K-1] - v[j-K-1];
        }
        ia[counter] = i; ja[counter] = dim + K + 1;
        ar[counter++] = 1;
    }
    //dim+M+1
    for (int j = 1; j <= K; ++j)
    {
        ia[counter] = dim + M + 1; ja[counter] = j;
        ar[counter++] = 1;
    }
    for (int j = K + 1; j <= K + dim + 1; ++j)
    {
        ia[counter] = dim + M + 1; ja[counter] = j;
        ar[counter++] = 0;
    }

    // loading data
    glp_load_matrix(lp, counter - 1, ia, ja, ar);
    // running simplex
    glp_smcp parm;
    glp_init_smcp(&parm);
    parm.msg_lev = GLP_MSG_OFF; // turn off all message by glp_simplex
    glp_simplex(lp, &parm);

    double val = glp_get_obj_val(lp);
    utility = 0;
    if(val > EQN2)
    {
        double *para = new double[K], *uv = new double[dim];
        for (int i = 0; i < K; ++i)
            para[i] = glp_get_col_prim(lp, i + 1);
        for (int i = 0; i < dim; ++i)
        {
            uv[i] = 0;
            for (int j = 0; j < K; ++j)
                uv[i] += para[j] * ext_pts[j]->attr[i];
            utility += uv[i] * v[i];
        }
    }
    glp_delete_prob(lp); // clean up
    delete[]ia;
    delete[]ja;
    delete[]ar;
    return val;
}

/**
 * @brief Check whether v could be a top-1(only) vector w.r.t R
 *        Used with extreme points of R
 * @param pset  Point set
 * @param v     Vector
 * @return      1 v could be a top-1 vector
 *              0 v could not be a top-1 vector
 */
double hyperplane_set::istop_leq(std::vector<double *> &pset, double *v)
{
    int M = pset.size(), K = ext_pts.size(), dim = ext_pts[0]->d;
    if(M < 2)
        return 100;
    int *ia = new int[(dim + M + 1) * (dim + K + 1) + 1];  //TODO: delete
    int *ja = new int[(dim + M + 1) * (dim + K + 1) + 1];  //TODO: delete
    double* ar = new double[(dim + M + 1) * (dim + K + 1) + 1];   //TODO: delete

    glp_prob *lp;
    lp = glp_create_prob();
    glp_set_prob_name(lp, "is_topl");
    glp_set_obj_dir(lp, GLP_MAX);

    // add dim+1 rows: q_1, ..., q_{dim+M+1}
    glp_add_rows(lp, dim + M + 1);
    for (int i = 1; i <= dim; ++i) {
        char buf[10];
        sprintf(buf, "q%d", i);
        glp_set_row_name(lp, i, buf);
        glp_set_row_bnds(lp, i, GLP_FX, 0.0, 0.0); // q_i
    }
    for (int i = dim + 1; i <= dim + M; ++i) {
        char buf[10];
        sprintf(buf, "q%d", i);
        glp_set_row_name(lp, i, buf);
        glp_set_row_bnds(lp, i, GLP_UP, 0.0, 0.0); // q_i
    }
    char buf[10];
    sprintf(buf, "q%d", dim + M + 1);
    glp_set_row_name(lp, dim + M + 1, buf);
    glp_set_row_bnds(lp, dim + M + 1, GLP_FX, 1.0, 1.0); // q_{dim+1} = 1

    // add M columns: v_1, ..., v_{dim + M +1}
    glp_add_cols(lp, dim + K + 1);
    for (int i = 1; i <= dim + K; ++i) {
        char buf[10];
        sprintf(buf, "v%d", i);
        glp_set_col_name(lp, i, buf);
        glp_set_col_bnds(lp, i, GLP_LO, 0.0, 0.0); // 0 <= v[i] < infty
        glp_set_obj_coef(lp, i, 0.0);  // objective: 0
    }
    char buff[10];
    sprintf(buff, "v%d", dim + K + 1);
    glp_set_col_name(lp, dim + K + 1, buff);
    glp_set_col_bnds(lp, dim + K + 1, GLP_FR, 0.0, 0.0); // -infty < v[i] < infty
    glp_set_obj_coef(lp, dim + K + 1, 1.0);  // objective: 0


    int counter = 1;
    // set coeffcient on each row
    for (int i = 1; i <= dim; ++i) //1, 2, ..., dim
    {
        for (int j = 1; j <= K; ++j)
        {
            ia[counter] = i; ja[counter] = j;
            ar[counter++] = ext_pts[j-1]->attr[i-1];
        }
        for (int j = K + 1; j <= K + dim; ++j)
        {
            ia[counter] = i; ja[counter] = j;
            if(i == j - K)
                ar[counter++] = -1;
            else
                ar[counter++] = 0;
        }
        ia[counter] = i; ja[counter] = dim + K + 1;
        ar[counter++] = 0;
    }
    for (int i = dim + 1; i <= dim + M; ++i)//dim+1, dim+2, ..., dim+M
    {
        for (int j = 1; j <= K; ++j)
        {
            ia[counter] = i; ja[counter] = j;
            ar[counter++] = 0;
        }
        for (int j = K + 1; j <= K + dim; ++j)
        {
            ia[counter] = i; ja[counter] = j;
            ar[counter++] = v[j-K-1] - pset[i-dim-1][j-K-1];
        }
        ia[counter] = i; ja[counter] = dim + K + 1;
        ar[counter++] = 1;
    }
    //dim+M+1
    for (int j = 1; j <= K; ++j)
    {
        ia[counter] = dim + M + 1; ja[counter] = j;
        ar[counter++] = 1;
    }
    for (int j = K + 1; j <= K + dim + 1; ++j)
    {
        ia[counter] = dim + M + 1; ja[counter] = j;
        ar[counter++] = 0;
    }


    // loading data
    glp_load_matrix(lp, counter - 1, ia, ja, ar);
    // running simplex
    glp_smcp parm;
    glp_init_smcp(&parm);
    parm.msg_lev = GLP_MSG_OFF; // turn off all message by glp_simplex
    glp_simplex(lp, &parm);

    double val = glp_get_obj_val(lp);
    glp_delete_prob(lp); // clean up
    delete[]ia;
    delete[]ja;
    delete[]ar;
    return val >= EQN2;
}

/**
 * @brief Check whether v could be a top-1(only) vector w.r.t R
 *        Used with extreme points of R
 * @param pset  Point set
 * @param v     Vector
 * @return      1 v could be a top-1 vector
 *              0 v could not be a top-1 vector
 */
double hyperplane_set::istop_geq(std::vector<double*> &pset, double *v)
{
    int M = pset.size(), K= ext_pts.size(), dim = ext_pts[0]->d;
    if(M < 2)
        return 100;
    int *ia = new int[(dim + M + 1) * (dim + K + 1) + 1];  //TODO: delete
    int *ja = new int[(dim + M + 1) * (dim + K + 1) + 1];  //TODO: delete
    double* ar = new double[(dim + M + 1) * (dim + K + 1) + 1];   //TODO: delete

    glp_prob *lp;
    lp = glp_create_prob();
    glp_set_prob_name(lp, "is_topg");
    glp_set_obj_dir(lp, GLP_MAX);

    // add dim+1 rows: q_1, ..., q_{dim+M+1}
    glp_add_rows(lp, dim + M + 1);
    for (int i = 1; i <= dim; ++i) {
        char buf[10];
        sprintf(buf, "q%d", i);
        glp_set_row_name(lp, i, buf);
        glp_set_row_bnds(lp, i, GLP_FX, 0.0, 0.0); // q_i
    }
    for (int i = dim + 1; i <= dim + M; ++i) {
        char buf[10];
        sprintf(buf, "q%d", i);
        glp_set_row_name(lp, i, buf);
        glp_set_row_bnds(lp, i, GLP_UP, 0.0, 0.0); // q_i
    }
    char buf[10];
    sprintf(buf, "q%d", dim + M + 1);
    glp_set_row_name(lp, dim + M + 1, buf);
    glp_set_row_bnds(lp, dim + M + 1, GLP_FX, 1.0, 1.0); // q_{dim+1} = 1

    // add M columns: v_1, ..., v_{dim + M +1}
    glp_add_cols(lp, dim + K + 1);
    for (int i = 1; i <= dim + K; ++i) {
        char buf[10];
        sprintf(buf, "v%d", i);
        glp_set_col_name(lp, i, buf);
        glp_set_col_bnds(lp, i, GLP_LO, 0.0, 0.0); // 0 <= v[i] < infty
        glp_set_obj_coef(lp, i, 0.0);  // objective: 0
    }
    char buff[10];
    sprintf(buff, "v%d", dim + K + 1);
    glp_set_col_name(lp, dim + K + 1, buff);
    glp_set_col_bnds(lp, dim + K + 1, GLP_FR, 0.0, 0.0); // -infty < v[i] < infty
    glp_set_obj_coef(lp, dim + K + 1, 1.0);  // objective: 0


    int counter = 1;
    // set coeffcient on each row
    for (int i = 1; i <= dim; ++i) //1, 2, ..., dim
    {
        for (int j = 1; j <= K; ++j)
        {
            ia[counter] = i; ja[counter] = j;
            ar[counter++] = ext_pts[j-1]->attr[i-1];
        }
        for (int j = K + 1; j <= K + dim; ++j)
        {
            ia[counter] = i; ja[counter] = j;
            if(i == j - K)
                ar[counter++] = -1;
            else
                ar[counter++] = 0;
        }
        ia[counter] = i; ja[counter] = dim + K + 1;
        ar[counter++] = 0;
    }
    for (int i = dim + 1; i <= dim + M; ++i)//dim+1, dim+2, ..., dim+M
    {
        for (int j = 1; j <= K; ++j)
        {
            ia[counter] = i; ja[counter] = j;
            ar[counter++] = 0;
        }
        for (int j = K + 1; j <= K + dim; ++j)
        {
            ia[counter] = i; ja[counter] = j;
            ar[counter++] = pset[i-dim-1][j-K-1] - v[j-K-1];
        }
        ia[counter] = i; ja[counter] = dim + K + 1;
        ar[counter++] = 1;
    }
    //dim+M+1
    for (int j = 1; j <= K; ++j)
    {
        ia[counter] = dim + M + 1; ja[counter] = j;
        ar[counter++] = 1;
    }
    for (int j = K + 1; j <= K + dim + 1; ++j)
    {
        ia[counter] = dim + M + 1; ja[counter] = j;
        ar[counter++] = 0;
    }


    // loading data
    glp_load_matrix(lp, counter - 1, ia, ja, ar);
    // running simplex
    glp_smcp parm;
    glp_init_smcp(&parm);
    parm.msg_lev = GLP_MSG_OFF; // turn off all message by glp_simplex
    glp_simplex(lp, &parm);

    double val = glp_get_obj_val(lp);
    glp_delete_prob(lp); // clean up
    delete[]ia;
    delete[]ja;
    delete[]ar;
    return val >= EQN2;
}


/**
 * @brief Check whether p1 R-dominates p2
 * @param p1 The first point
 * @param p2 The second point
 * @return   1 R-dominates
 *          -1 Does not R-dominate
 */
bool hyperplane_set::R_dominate(double *p1, double *p2)
{
    int size = ext_pts.size(), d = ext_pts[0]->d;
    double *v = new double[d]; //set the difference as a vector
    double dot = 0;
    for(int i = 0; i < d; i++)
    {
        v[i] = p1[i] - p2[i];
        if(-EQN3 < v[i] && v[i] < EQN3)
            dot++;
    }
    if(dot == d)
        return true;
    for (int i = 0; i < size; i++)
    {
        dot = ext_pts[i]->dot_product(v);
        if (dot < -EQN2)
            return false;
    }
    return true;
}

/**
 * @brief   Check whether \sum u(v + a1x1 + a2x2 + ... ) >= 0
 * @param x A set of vectors x
 * @param v The vector v
 * @return  -true >=0
 *          -false <=0
 */
bool hyperplane_set::R_dominate_geq(std::vector<std::pair<double *, int>> &x, double *v)
{
    if(x.size() < 1)
        return false;
    int row = ext_pts.size(), col = x.size();
    int *ia = new int[(row + 1) * col + 1];  //TODO: delete
    int *ja = new int[(row + 1) * col + 1];  //TODO: delete
    double* ar = new double[(row + 1) * col + 1];   //TODO: delete

    glp_prob *lp;
    lp = glp_create_prob();
    glp_set_prob_name(lp, "r_dominate");
    glp_set_obj_dir(lp, GLP_MAX);
    glp_add_rows(lp, row + 1);  // add (row + 1) rows: q_0, ..., q_{row}
    for (int i = 1; i <= row; i++)  // q_0, ..., q_{row - 1}
    {
        char buf[10];
        sprintf(buf, "q%d", i);
        glp_set_row_name(lp, i, buf);
        glp_set_row_bnds(lp, i, GLP_LO, -ext_pts[i - 1]->dot_product(v), 0); // q_i >= -uv
    }
    char buf[10];
    sprintf(buf, "q%d", row + 1);
    glp_set_row_name(lp, row + 1, buf);
    glp_set_row_bnds(lp, row + 1, GLP_FX, 1.0, 1.0); // q_{dim} = 1

    glp_add_cols(lp, col);    // add M columns: v_1, ..., v_D
    for (int i = 1; i <= col; i++) {
        char buf[10];
        sprintf(buf, "v%d", i);
        glp_set_col_name(lp, i, buf);
        glp_set_col_bnds(lp, i, GLP_LO, 0.0, 0.0); // 0 <= v[i] < infty
        glp_set_obj_coef(lp, i, 0.0);  // objective: 0
    }


    int counter = 1;
    // set value on row q1 ... qD
    for (int i = 1; i <= row + 1; ++i) {
        for (int j = 1; j <= col; ++j)
        {
            ia[counter] = i; ja[counter] = j;
            if(i <= row)
                ar[counter++] = ext_pts[i - 1]->dot_product(x[j - 1].first);
            else
                ar[counter++] = 1;
        }
    }

    glp_load_matrix(lp, counter - 1, ia, ja, ar);
    glp_smcp parm;
    glp_init_smcp(&parm);
    parm.msg_lev = GLP_MSG_OFF; // turn off all message by glp_simplex
    glp_simplex(lp, &parm);

    double w1 = glp_get_prim_stat(lp);
    glp_delete_prob(lp); // clean up
    delete[]ia;
    delete[]ja;
    delete[]ar;
    return w1 == GLP_OPT || w1 == GLP_FEAS;
}

/**
 * @brief   Check whether \sum u(v + a1x1 + a2x2 + ... ) <= 0
 * @param x A set of vectors x
 * @param v The vector v
 * @return  -true <=0
 *          -false >=0
 */
bool hyperplane_set::R_dominate_leq(std::vector<std::pair<double *, int>> &x, double *v)
{
    if(x.size() < 1)
        return false;
    int row = ext_pts.size(), col = x.size();
    int *ia = new int[(row + 1) * col + 1];  //TODO: delete
    int *ja = new int[(row + 1) * col + 1];  //TODO: delete
    double* ar = new double[(row + 1) * col + 1];   //TODO: delete

    glp_prob *lp;
    lp = glp_create_prob();
    glp_set_prob_name(lp, "r_dominate");
    glp_set_obj_dir(lp, GLP_MAX);
    glp_add_rows(lp, row + 1);  // add (row + 1) rows: q_0, ..., q_{row}
    for (int i = 1; i <= row; i++)  // q_0, ..., q_{row - 1}
    {
        char buf[10];
        sprintf(buf, "q%d", i);
        glp_set_row_name(lp, i, buf);
        glp_set_row_bnds(lp, i, GLP_UP, 0, -ext_pts[i - 1]->dot_product(v)); // q_i >= -uv
    }
    char buf[10];
    sprintf(buf, "q%d", row + 1);
    glp_set_row_name(lp, row + 1, buf);
    glp_set_row_bnds(lp, row + 1, GLP_FX, 1.0, 1.0); // q_{dim} = 1

    glp_add_cols(lp, col);    // add M columns: v_1, ..., v_D
    for (int i = 1; i <= col; i++) {
        char buf[10];
        sprintf(buf, "v%d", i);
        glp_set_col_name(lp, i, buf);
        glp_set_col_bnds(lp, i, GLP_LO, 0.0, 0.0); // v[i] <= 0
        glp_set_obj_coef(lp, i, 0.0);  // objective: 0
    }


    int counter = 1;
    // set value on row q1 ... qD
    for (int i = 1; i <= row + 1; ++i) {
        for (int j = 1; j <= col; ++j)
        {
            ia[counter] = i; ja[counter] = j;
            if(i <= row)
                ar[counter++] = ext_pts[i - 1]->dot_product(x[j - 1].first);
            else
                ar[counter++] = 1;
        }
    }

    glp_load_matrix(lp, counter - 1, ia, ja, ar);
    glp_smcp parm;
    glp_init_smcp(&parm);
    parm.msg_lev = GLP_MSG_OFF; // turn off all message by glp_simplex
    glp_simplex(lp, &parm);

    double w1 = glp_get_prim_stat(lp);
    glp_delete_prob(lp); // clean up
    delete[]ia;
    delete[]ja;
    delete[]ar;
    return w1 == GLP_OPT || w1 == GLP_FEAS;
}

void hyperplane_set::find_ext_leq(std::vector<inequality> &ineqleq, int i, std::vector<point_t *> &ext)
{
    int size = ineqleq.size(), dim = ext_pts[0]->d;
    if(size < 1)
        return;
    else if(size == 1)
    {
        for (int j = 0; j < ext_pts.size(); ++j)
        {
            bool allZero = true;
            point_t *p = new point_t(dim);
            for (int k = 0; k < dim; ++k)
                p->attr[k] = ext_pts[j]->attr[k];
            ext.push_back(p);
        }
        return;
    }

    //obtain hyperplanes
    hyperplane_set *hset = new hyperplane_set();
    for(int j = 0; j < hyperplanes.size(); ++j)
    {
        hyperplane *h = new hyperplane(dim);
        for(int k = 0; k < dim; ++k)
            h->norm[k] = hyperplanes[j]->norm[k];
        h->offset = hyperplanes[j]->offset;
        hset->hyperplanes.emplace_back(h);
    }
    for(int j = 0; j < size; ++j)
    {
        if(i != j)
        {
            hyperplane *h = new hyperplane(dim);
            for (int k = 0; k < dim; k++)
                h->norm[k] = ineqleq[i].norm[k] - ineqleq[j].norm[k] - EDGE;
            h->offset = 0;
            hset->hyperplanes.emplace_back(h);
        }
    }

    char file1[MAX_FILENAME_LENG];
    sprintf(file1, "../output/hyperplane_data.txt");
    char file2[MAX_FILENAME_LENG];
    sprintf(file2, "../output/ext_pt.txt");


    point_t *feasible_pt = find_feasible(hset->hyperplanes);//the feasible point
    if (feasible_pt == NULL)
    {
        //cout<<"The intersection is infeasible.\n";
        delete hset;
        return;
    }
    hset->write(feasible_pt, file1);//write hyperplanes and the feasible point to file1
    delete hset;
    FILE *rPtr;
    FILE *wPtr;
    rPtr = (FILE *) fopen(file1, "r");
    wPtr = (FILE *) fopen(file2, "w");
    halfspace(rPtr, wPtr);
    fclose(rPtr);
    fclose(wPtr);

    //read extreme points in file2
    rPtr = (FILE *)fopen(file2, "r");
    fscanf(rPtr, "%i%i", &dim, &size);
    for (int j = 0; j < size; ++j)
    {
        bool allZero = true;
        point_t *p = new point_t(dim);
        for (int k = 0; k < dim; ++k)
        {
            fscanf(rPtr, "%lf", &p->attr[k]);
            if (!isZero(p->attr[k]))
                allZero = false;
        }
        if (allZero)
            delete p;
        else
            ext.push_back(p);
    }
    fclose(rPtr);
}

void hyperplane_set::find_ext_geq(std::vector<inequality> &ineqgeq, int i, std::vector<point_t *> &ext)
{
    int size = ineqgeq.size(), dim = ext_pts[0]->d;
    if(size < 1)
        return;
    else if(size == 1)
    {
        for (int j = 0; j < ext_pts.size(); ++j)
        {
            bool allZero = true;
            point_t *p = new point_t(dim);
            for (int k = 0; k < dim; ++k)
                p->attr[k] = ext_pts[j]->attr[k];
            ext.push_back(p);
        }
        return;
    }

    //obtain hyperplanes
    hyperplane_set *hset = new hyperplane_set();
    for(int j = 0; j < hyperplanes.size(); ++j)
    {
        hyperplane *h = new hyperplane(dim);
        for(int k = 0; k < dim; ++k)
            h->norm[k] = hyperplanes[j]->norm[k];
        h->offset = hyperplanes[j]->offset;
        hset->hyperplanes.emplace_back(h);
    }
    for(int j = 0; j < size; ++j)
    {
        if(i != j)
        {
            hyperplane *h = new hyperplane(dim);
            for (int k = 0; k < dim; k++)
                h->norm[k] = ineqgeq[j].norm[k] - ineqgeq[i].norm[k] - EDGE;
            h->offset = 0;
            hset->hyperplanes.emplace_back(h);
        }
    }

    char file1[MAX_FILENAME_LENG];
    sprintf(file1, "../output/hyperplane_data.txt");
    char file2[MAX_FILENAME_LENG];
    sprintf(file2, "../output/ext_pt.txt");


    point_t *feasible_pt = find_feasible(hset->hyperplanes);//the feasible point
    if (feasible_pt == NULL)
    {
        //cout<<"The intersection is infeasible.\n";
        delete hset;
        return;
    }
    hset->write(feasible_pt, file1);//write hyperplanes and the feasible point to file1
    delete hset;
    FILE *rPtr;
    FILE *wPtr;
    rPtr = (FILE *) fopen(file1, "r");
    wPtr = (FILE *) fopen(file2, "w");
    halfspace(rPtr, wPtr);
    fclose(rPtr);
    fclose(wPtr);

    //read extreme points in file2
    rPtr = (FILE *) fopen(file2, "r");
    fscanf(rPtr, "%i%i", &dim, &size);
    for (int j = 0; j < size; ++j)
    {
        bool allZero = true;
        point_t *p = new point_t(dim);
        for (int k = 0; k < dim; ++k)
        {
            fscanf(rPtr, "%lf", &p->attr[k]);
            if (!isZero(p->attr[k]))
                allZero = false;
        }
        if (allZero)
            delete p;
        else
            ext.push_back(p);
    }
    fclose(rPtr);
}

/**
 * @brief   Check the relation between two tuples
 * @param x The relation between the correspondin u
 * @param ext The set of the utility vectors (extreme points)
 * @param v     The numerical difference between two tuples
 * @param direction geq (direction = 1) && leq (direction = -1)
 * @return true/false
 */
bool hyperplane_set::dominate(std::vector<inequality> &x, std::vector<point_t *> *ext, double *v, int direction)
{
    int size = x.size(), dim = ext_pts[0]->d;
    if(direction == 1)
    {
        //bound check
        double *minv = new double[dim];
        for(int k = 0; k < dim; ++k)
            minv[k] = INF;
        for (int i = 0; i < size; ++i)
        {
            for(int k = 0; k < dim; ++k)
            {
                if (minv[k] > x[i].norm[k])
                    minv[k] = x[i].norm[k];
            }
        }
        bool is_dom = true;
        for (int j = 0; j < ext_pts.size(); ++j)
        {
            double sum = 0;
            for (int k = 0; k < dim; ++k)
                sum += (v[k] + minv[k]) * ext_pts[j]->attr[k];
            if (sum < -EQN2)
            {
                is_dom = false;
                break;
            }
        }
        if(is_dom)
            return true;

        //check each equality
        for (int i = 0; i < size; ++i)
        {
            for (int j = 0; j < ext[i].size(); ++j)
            {
                double sum = 0;
                for (int k = 0; k < dim; ++k)
                    sum += (v[k] + x[i].norm[k]) * ext[i][j]->attr[k];
                if (sum < -EQN2)
                    return false;
            }
        }
    }
    else
    {
        //bound check
        double *maxv = new double[dim];
        for(int k = 0; k < dim; ++k)
            maxv[k] = -INF;
        for (int i = 0; i < size; ++i)
        {
            for(int k = 0; k < dim; ++k)
            {
                if (maxv[k] < x[i].norm[k])
                    maxv[k] = x[i].norm[k];
            }
        }
        bool is_dom = true;
        for (int j = 0; j < ext_pts.size(); ++j)
        {
            double sum = 0;
            for (int k = 0; k < dim; ++k)
                sum += (v[k] + maxv[k]) * ext_pts[j]->attr[k];
            if (sum > EQN2)
            {
                is_dom = false;
                break;
            }
        }
        if(is_dom)
            return true;

        //check each equality
        for(int i = 0; i < size; ++i)
        {
            for(int j = 0; j < ext[i].size(); ++j)
            {
                double sum = 0;
                for(int k = 0; k < dim; ++k)
                    sum += (v[k] + x[i].norm[k]) * ext[i][j]->attr[k];
                if(sum > EQN2)
                    return false;
            }
        }
    }
    return true;
}

/**
 * @brief   Check the relation between two tuples
 * @param x The relation between the correspondin u
 * @param ext The set of the utility vectors (extreme points)
 * @param v     The numerical difference between two tuples
 * @param direction geq (direction = 1) && leq (direction = -1)
 * @return true/false
 */
bool hyperplane_set::dominated(std::vector<inequality> &x, std::vector<point_t *> *ext, double *v, int direction, int *dom)
{
    int size = x.size(), dim = ext_pts[0]->d, countdom = 0;
    if(direction == 1)
    {
        //check each equality
        for (int i = 0; i < size; ++i)
        {
            if(dom[i] == 0)
            {
                dom[i] = 1;
                for (int j = 0; j < ext[i].size(); ++j)
                {
                    double sum = 0;
                    for (int k = 0; k < dim; ++k)
                        sum += (v[k] + x[i].norm[k]) * ext[i][j]->attr[k];
                    if (sum < -EQN2)
                    {
                        dom[i] = 0;
                        break;
                    }
                }
                if (dom[i] == 1)
                    countdom++;
            }
            else
                countdom++;
        }
        if(countdom == size)
            return true;
    }
    else
    {
        //check each equality
        for(int i = 0; i < size; ++i)
        {
            if(dom[i] == 0)
            {
                dom[i] = 1;
                for (int j = 0; j < ext[i].size(); ++j)
                {
                    double sum = 0;
                    for (int k = 0; k < dim; ++k)
                        sum += (v[k] + x[i].norm[k]) * ext[i][j]->attr[k];
                    if (sum > EQN2)
                    {
                        dom[i] = 0;
                        break;
                    }
                }
                if (dom[i] == 1)
                    countdom++;
            }
            else
                countdom++;
        }
        if(countdom == size)
            return true;
    }
    return false;
}


/**
 * @brief   Check the relation between two tuples
 * @param x The relation between the correspondin u
 * @param ext The set of the utility vectors (extreme points)
 * @param v     The numerical difference between two tuples
 * @param direction geq (direction = 1) && leq (direction = -1)
 * @return      If dominate, return -1. Otherwise, return the differences
 */
double hyperplane_set::dominate_distance(std::vector<inequality> &x, std::vector<point_t *> *ext, double *v, int direction)
{
    int size = x.size(), dim = ext_pts[0]->d;
    double is_dominate = INF;
    if(direction == 1)
    {
        double min = 100;
        for (int i = 0; i < size; ++i)
        {
            is_dominate = -1;
            for (int j = 0; j < ext[i].size(); j++)
            {
                double sum = 0;
                for (int k = 0; k < dim; ++k)
                    sum += (v[k] + x[i].norm[k]) * ext[i][j]->attr[k];
                if (sum < min)
                    min = sum;
            }
        }
        if(min < -1)
            return -min;
        else
            return is_dominate;
    }
    else
    {
        double max = -100;
        for(int i = 0; i < size; ++i)
        {
            is_dominate = -1;
            for(int j = 0; j < ext[i].size(); ++j)
            {
                double sum = 0;
                for(int k = 0; k < dim; ++k)
                    sum += (v[k] + x[i].norm[k]) * ext[i][j]->attr[k];
                if(sum > max)
                    max = sum;
            }
        }
        if(max > 1)
            return max;
        else
            return is_dominate;
    }
}

/**
 * @brief Calculate the average point of the extreme points
 * @param ap The average point
 */
void hyperplane_set::average_point(point_t* ap)
{
    double size = ext_pts.size(), dim = ap->d;
    for(int j = 0; j < dim; ++j)
        ap->attr[j] = 0;
    //calculate the sum
    for(int i = 0; i < size; ++i)
    {
        for (int j = 0; j < dim; ++j)
            ap->attr[j] += ext_pts[i]->attr[j];
    }
    for(int j = 0; j < dim; ++j)
        ap->attr[j] /= size;
}


point_t* hyperplane_set::appx_center()
{
    int M = hyperplanes.size();
    int D = hyperplanes[0]->d;

    int* ia = new int[1 + (D + 1) * (2 * M + 1)];  //TODO: delete
    int* ja = new int[1 + (D + 1) * (2 * M + 1)];  //TODO: delete
    double* ar = new double[1 + (D + 1) * (2 * M + 1)];   //TODO: delete
    int i, j;
    double epsilon = 0.0000000000001;


    glp_prob *lp;
    lp = glp_create_prob();
    glp_set_prob_name(lp, "appx_center");
    glp_set_obj_dir(lp, GLP_MAX);


    glp_add_rows(lp, 2 * M + 1);
    // Add rows q_1 ... q_D
    for (i = 1; i <= 2 * M + 1; i++) {
        char buf[10];
        sprintf(buf, "q%d", i);
        glp_set_row_name(lp, i, buf);
        if(i < M)
            glp_set_row_bnds(lp, i, GLP_UP, 0, 0); // qi = 0
        else if(i <= 2 * M)
            glp_set_row_bnds(lp, i, GLP_LO, 0, 0);
        else
            glp_set_row_bnds(lp, i, GLP_FX, 1.0, 1.0);
    }

    glp_add_cols(lp, D + 1);
    // Add col v[1] ... v[D + 1]
    for (i = 1; i <= D + 1; i++)
    {
        char buf[10];
        sprintf(buf, "v%d", i);
        glp_set_col_name(lp, i, buf);
        glp_set_col_bnds(lp, i, GLP_LO, 0.0, 0.0);
        if(i <= D)
            glp_set_obj_coef(lp, i, 0.0);  // objective: 0
        else
            glp_set_obj_coef(lp, i, 1.0);
    }


    int counter = 1;
    for (i = 1; i <= M; i++) {
        for (j = 1; j <= D + 1; j++) {
            ia[counter] = i; ja[counter] = j;
            if(j <= D)
                ar[counter++] = hyperplanes[i - 1]->norm[j - 1];
            else
                ar[counter++] = 0;
        }
    }
    for (i = M + 1; i <= 2 * M; ++i) {
        for (j = 1; j <= D + 1; ++j) {
            ia[counter] = i; ja[counter] = j;
            if(j <= D)
                ar[counter++] = hyperplanes[i - M - 1]->norm[j - 1];
            else
                ar[counter++] = -1;
        }
    }
    for (j = 1; j <= D + 1; ++j) {
        ia[counter] = 2 * M + 1; ja[counter] = j;
        if(j <= D)
            ar[counter++] = 1.0;
        else
            ar[counter++] = 0.0;
    }

    // loading data
    glp_load_matrix(lp, counter - 1, ia, ja, ar);

    // running simplex
    glp_smcp parm;
    glp_init_smcp(&parm);
    parm.msg_lev = GLP_MSG_OFF; // turn off all message by glp_simplex
    glp_simplex(lp, &parm);


    point_t* feasible_pt = new point_t(D);
    for (i = 0; i < D; ++i)
    {
        feasible_pt->attr[i] = glp_get_col_prim(lp, i + 1);
    }

    glp_delete_prob(lp); // clean up
    delete[]ia;
    delete[]ja;
    delete[]ar;

    return feasible_pt;
}




/**
 * @brief Approximately calculate the regret ratio when p2 represent p1 (p2 is the represented point)
 * @param p1    The represented point
 * @param p2    The point
 * @return      The regret ratio
 */
double hyperplane_set::regret_bound(point_t *p1, point_t *p2)
{
    int size = ext_pts.size(), dim = ext_pts[0]->d;
    double regret = 0, *coef = new double[size];

    //p1->print(); p2->print();


    for(int round = 0; round < 10000; ++round)
    {
        //obtain the weight of each extreme utility vector
        double sum = 0;
        for(int i = 0; i < size; ++i)
        {
            coef[i] = rand()%1000;
            sum += coef[i];
        }
        for(int i = 0; i < size; ++i)
            coef[i] /= sum;

        //obtain the sampled utility vector
        point_t *u = new point_t(dim);
        for(int j = 0; j < dim; ++j)
            u->attr[j] = 0;
        for(int i = 0; i < size; ++i)
        {
            for(int j = 0; j < dim; ++j)
                u->attr[j] = u->attr[j] + (coef[i] * ext_pts[i]->attr[j]);
        }

        //regret ratio w.r.t. u
        double s1 = p1->dot_product(u);
        double s2 = p2->dot_product(u);
        double value = (s1 - s2) / s1;
        if(value > regret)
            regret = value;
    }


    for(int round = 0; round < size; ++round)
    {
        //regret ratio w.r.t. u
        double s1 = p1->dot_product(ext_pts[round]);
        double s2 = p2->dot_product(ext_pts[round]);
        double value = (s1 - s2) / s1;
        if(value > regret)
            regret = value;
    }

    return regret;
}

double hyperplane_set::regret_bound(point_set* P)
{
    int dim = P->points[0]->d, maxIdx;

    double size = ext_pts.size(), maxValue;
    point_t* ap = new point_t(dim);
    average_point(ap);

    maxIdx = 0; size = P->points.size(); maxValue = 0;
    for (int i = 0; i < size; i++)
    {
        double value = P->points[i]->dot_product(ap);//utility of the points
        if (value > maxValue)
        {
            maxValue = value;
            maxIdx = i;
        }
    }

    double regret = 0;
    for(int i = 0; i < P->points.size(); ++i)
    {
        double value = regret_bound(P->points[i], P->points[maxIdx]);
        if(value > regret)
            regret = value;
    }

    return regret;
}


double hyperplane_set::regret_bound(point_set* P, vector<int>& C_idx)
{
    int dim = P->points[0]->d, maxIdx;

    double size = ext_pts.size(), maxValue;
    point_t* ap = new point_t(dim);
    average_point(ap);

    maxIdx = 0; size = C_idx.size(); maxValue = 0;
    for (int i = 0; i < size; i++)
    {
        double value = P->points[ C_idx[i] ]->dot_product(ap);//utility of the points
        if (value > maxValue)
        {
            maxValue = value;
            maxIdx = i;
        }
    }

    double regret = 0;
    for(int i = 0; i < C_idx.size(); ++i)
    {
        double value = regret_bound(P->points[ C_idx[i] ], P->points[C_idx[maxIdx]]);
        if(value > regret)
            regret = value;
    }

    return regret;
}


double hyperplane_set::regret_bound(point_t* est_u, point_set* P)
{
    int size = P->points.size(), maxIdx = 0;
    double maxValue = 0;

    for (int i = 0; i < size; i++)
    {
        double value = P->points[i]->dot_product(est_u);//utility of the points
        if (value > maxValue)
        {
            maxValue = value;
            maxIdx = i;
        }
    }

    double regret = 0;
    for(int i = 0; i < P->points.size(); ++i)
    {
        double value = regret_bound(P->points[i], P->points[maxIdx]);
        if(value > regret)
            regret = value;
    }

    return regret;
}


void hyperplane_set::prune_same_cat(tuple_set *tset)
{
    int size = tset->tuples.size(), dim = hyperplanes[0]->d;
    bool *is_prune = new bool[size];
    for(int i = 0; i < size; ++i)
    {
        //construct the hyperplanes and a feasible point
        hyperplane_set *h_set = new hyperplane_set();
        hyperplane *h;
        for (int i = 0; i < hyperplanes.size(); ++i)
        {
            h = new hyperplane(dim);
            for (int j = 0; j < dim; j++)
                h->norm[j] = hyperplanes[i]->norm[j];
            h->offset = hyperplanes[i]->offset;
            h_set->hyperplanes.push_back(h);
        }

        for(int j = 0; j < size; ++j)
        {
            if(i != j)
            {
                h = new hyperplane(dim);
                for (int k = 0; k < dim; k++)
                    h->norm[k] = tset->tuples[j]->attr_num[k] - tset->tuples[i]->attr_num[k];
                h->offset = 0;
                h_set->hyperplanes.push_back(h);
            }
        }

        //the feasible point
        point_t *feasible_pt = find_feasible(h_set->hyperplanes);
        if(feasible_pt != NULL)
            is_prune[i] = false;
        else
            is_prune[i] = true;
    }

    for(int i = size - 1; i >= 0; --i)
    {
        if(is_prune[i])
        {
            tset->tuples.erase(tset->tuples.begin() + i);
        }
    }
}

/**
 * @brief Prune all the points which are dominated
 * @param pset
 */
point_set* hyperplane_set::Rdominate_prune(point_set *pset)
{
    int size = pset->points.size();
    int i, j, m, dominated, index = 0, dim = pset->points[0]->d;
    point_t* pt;

    int* sl = new int[size];
    for (i = 0; i < size; ++i)
    {
        dominated = 0;
        pt = pset->points[i];
        //pt->print();
        // check if pt is dominated by the skyline so far
        for (j = 0; j < index && !dominated; ++j)
            if (R_dominate(pset->points[sl[j]]->attr, pt->attr))
                dominated = 1;

        if (!dominated)
        {
            // eliminate any points in current skyline that it dominates
            m = index;//number of current points
            index = 0;
            for (j = 0; j < m; ++j)
                if (!R_dominate(pt->attr, pset->points[sl[j]]->attr))
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
        p = new point_t(pset->points[sl[i]]);
        skyline->points.push_back(p);
    }

    delete[] sl;
    return skyline;
}
























