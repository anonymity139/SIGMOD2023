//#include "stdAfx.h"
#include "lp.h"

#include <vector>

//#define DEBUG_LP

// Takes an array of points s (of size N) and  a point pt and returns
// the direction in which pt has the worst regret ratio (in array v)
// as well as this regret ratio itself. 

/* We solve the following LP with col variables v[0], v[1], ..., v[D], x 
   and row variables q_1 ... q_{K-1}, r1, r_2
   
   Max x
   s.t. - (pt.a[0]-s[0].a[0])v[0] - (pt.a[1]-s[0].a[1])v[1] - ... - (pt.a[D-1]-s[0].a[D-1])v[D-1] + x = q_1
        - (pt.a[0]-s[1].a[0])v[0] - (pt.a[1]-s[1].a[1])v[1] - ... - (pt.a[D-1]-s[1].a[D-1])v[D-1] + x = q_2
        ...
        - (pt.a[0]-s[K-1].a[0])v[0] - (pt.a[1]-s[K-1].a[1])v[1] - ... - (pt.a[D-1]-s[K-1].a[D]-1)v[D-1] + x= q_K
           pt.a[0]v[0] + pt.a[1]v[1]  ....  + pt.a[D-1]v[D-1] = r1
          -pt.a[0]v[0] - pt.a[1]v[1]  ....  - pt.a[D-1]v[D-1] = r2
   variables have the following bounds
       0 <= v[j] < infty
       0 <= x < infty
       -infty < q_i  <=0
       -infty < r1 <= 1
       -infty < r2 <= -1
*/


// Use LP to check whether a point pt is a conical combination of the vectors in ExRays
bool insideCone(std::vector<point_t*> ExRays, point_t* pt)
{
	int M = ExRays.size();
	int D = pt->d;

	int* ia = new int[1 + D * M];  //TODO: delete
	int* ja = new int[1 + D * M];  //TODO: delete
	double* ar = new double[1 + D * M];   //TODO: delete
	int i, j;
	double epsilon = 0.0000000000001;


	glp_prob *lp;
	lp = glp_create_prob();
	glp_set_prob_name(lp, "inside_cone");
	glp_set_obj_dir(lp, GLP_MAX);


	glp_add_rows(lp, D);  // add D rows: q_1...q_D
							  // Add rows q_1 ... q_D
	for (i = 1; i <= D; i++) {
		char buf[10];
		sprintf(buf, "q%d", i);
		glp_set_row_name(lp, i, buf);
		glp_set_row_bnds(lp, i, GLP_FX, pt->attr[i - 1], pt->attr[i-1]); // qi = pt->coord[i-1]
	}
	

	glp_add_cols(lp, M);    // add D columns: v[1] ... v[D]
								// Add col v[1] ... v[D]
	for (i = 1; i <= M; i++) {
		char buf[10];
		sprintf(buf, "v%d", i);

		glp_set_col_name(lp, i, buf);
		glp_set_col_bnds(lp, i, GLP_LO, 0.0, 0.0); // 0 <= v[i] < infty
		glp_set_obj_coef(lp, i, 0.0);  // objective: 0
	}

	int counter = 1;
	// set value on row q1 ... qD
	for (i = 1; i <= D; i++) {
		for (j = 1; j <= M; j++) {

			ia[counter] = i; ja[counter] = j;
			ar[counter++] = ExRays[j-1]->attr[i-1];
		}
	}



	// loading data  
	glp_load_matrix(lp, counter - 1, ia, ja, ar);



										  // running simplex
	glp_smcp parm;
	glp_init_smcp(&parm);
	parm.msg_lev = GLP_MSG_OFF; // turn off all message by glp_simplex 

	glp_simplex(lp, &parm);

	bool isOutside = glp_get_status(lp) == GLP_NOFEAS;

	//printf("solution is optimal: %d\n", GLP_OPT);
	//printf("no feasible solution: %d\n", GLP_NOFEAS);


	//printf("return: %d\n", result);
	//for (i = 0; i < M; i++)
	//{
	//	double v = glp_get_col_prim(lp, i + 1);
	//	printf("w%d = %lf\n", i + 1, v);
	//}

	glp_delete_prob(lp); // clean up
	delete[]ia;
	delete[]ja;
	delete[]ar;

	return !isOutside;
}


// Use LP to find a feasible point of the half space intersection (used later in Qhull for half space intersection)
point_t* find_feasible(std::vector<hyperplane*> hyperplane)
{
	int M = hyperplane.size();
	int D = hyperplane[0]->d;

	// D + 2variables: D for dim, 2 for additional var for feasible
	int* ia = new int[1 + (D + 2) * M];  //TODO: delete
	int* ja = new int[1 + (D + 2) * M];  //TODO: delete
	double* ar = new double[1 + (D + 2) * M];   //TODO: delete
	int i, j;
	double epsilon = 0.0000000000001;


	glp_prob *lp;
	lp = glp_create_prob();
	glp_set_prob_name(lp, "find_feasible");
	glp_set_obj_dir(lp, GLP_MAX);


	glp_add_rows(lp, M);  // add D rows: q_1...q_D
	// Add rows q_1 ... q_D
	for (i = 1; i <= M; i++) {
		char buf[10];
		sprintf(buf, "q%d", i);
		glp_set_row_name(lp, i, buf);
		glp_set_row_bnds(lp, i, GLP_UP, 0, 0); // qi = 0
	}


	glp_add_cols(lp, D + 2);    // add D columns: v[1] ... v[D]
	// Add col v[1] ... v[D]
	for (i = 1; i <= D + 2; i++) {
		char buf[10];
		sprintf(buf, "v%d", i);

		glp_set_col_name(lp, i, buf);

		if(i <= D)
			glp_set_col_bnds(lp, i, GLP_FR, 0.0, 0.0); // -infty <= v[i] < infty
		else if (i == D + 1)
			glp_set_col_bnds(lp, i, GLP_LO, 0.0, 0.0); // 0 <= v[i] < infty
		else
			glp_set_col_bnds(lp, i, GLP_UP, 0.0, D+1);

		if(i == D + 2)
			glp_set_obj_coef(lp, i, 1);  // objective: 0
		else
			glp_set_obj_coef(lp, i, 0.0);  // objective: 0
	}


	int counter = 1;
	// set value on row q1 ... qD
	for (i = 1; i <= M; i++) {
		for (j = 1; j <= D + 2; j++) {

			ia[counter] = i; ja[counter] = j;

			if(j <= D)
			{
				ar[counter++] = hyperplane[i-1]->norm[j-1];
				//printf("%lf ", hyperplane[i-1]->normal->coord[j-1]);
			}
			else if (j == D+1)
			{
				ar[counter++] = hyperplane[i-1]->offset;
				//printf("%lf ", hyperplane[i-1]->offset);
			}
			else if (j == D+2)
			{
				ar[counter++] = 1;
				//printf("1.00000\n");
			}
		}
	}

	// loading data
	glp_load_matrix(lp, counter - 1, ia, ja, ar);

	// running simplex
	glp_smcp parm;
	glp_init_smcp(&parm);
	parm.msg_lev = GLP_MSG_OFF; // turn off all message by glp_simplex

	glp_simplex(lp, &parm);


	point_t* feasible_pt = new point_t(D);
	double w1, w2;
	w1 = glp_get_col_prim(lp, D+1);
	w2 = glp_get_col_prim(lp, D+2);
    double status = glp_get_prim_stat(lp);

	if(w1 < 0 || w2 < 0 || isZero(w1) || isZero(w2))
	{
		//printf("LP feasible error.\n");
		return NULL;
	}
	for (i = 0; i < D; i++)
	{
		double v = glp_get_col_prim(lp, i + 1);
		//printf("w%d = %lf\n", i + 1, v);
		feasible_pt->attr[i] = v / w1;
	}

	//printf("solution status: %d\n",glp_get_status(lp));
	//printf("solution is unbounded: %d\n", GLP_UNBND);
	//printf("solution is optimal: %d\n", GLP_OPT);
	//printf("no feasible solution: %d\n", GLP_NOFEAS);
	//printf("return: %lf\n", glp_get_obj_val(lp));
	//for (i = 0; i < D + 2; i++)
	//{
	//	double v = glp_get_col_prim(lp, i + 1);
	//	printf("w%d = %lf\n", i + 1, v);
	//}

	glp_delete_prob(lp); // clean up
	delete[]ia;
	delete[]ja;
	delete[]ar;

	return feasible_pt;
}

// solve the LP in frame computation
void solveLP(std::vector<point_t*> B, point_t* b, double& theta, point_t* & pi)
{
	int M = B.size()+1;
	int D = b->d;

	point_t *mean = new point_t(D);
	for(int i = 0; i < D; i++)
		mean->attr[i] = 0;
	for(int i = 0; i < B.size(); i++)
	{
		for(int j = 0; j < D; j++)
			mean->attr[j] += B[i]->attr[j];
	}
	for(int i = 0; i < D; i++)
		mean->attr[i] /= B.size();

	int* ia = new int[1 + D * M];  //TODO: delete
	int* ja = new int[1 + D * M];  //TODO: delete
	double* ar = new double[1 + D * M];   //TODO: delete
	int i, j;
	double epsilon = 0.0000000000001;


	glp_prob *lp;
	lp = glp_create_prob();
	glp_set_prob_name(lp, "sloveLP");
	glp_set_obj_dir(lp, GLP_MIN);


	glp_add_rows(lp, D);  // add D rows: q_1...q_D
							  // Add rows q_1 ... q_D
	for (i = 1; i <= D; i++) {
		char buf[10];
		sprintf(buf, "q%d", i);
		glp_set_row_name(lp, i, buf);
		glp_set_row_bnds(lp, i, GLP_FX, b->attr[i - 1], b->attr[i-1]); // qi = pt->coord[i-1]
	}
	

	glp_add_cols(lp, M);    // add D columns: v[1] ... v[D]
								// Add col v[1] ... v[D]
	for (i = 1; i <= M; i++) {
		char buf[10];
		sprintf(buf, "v%d", i);

		glp_set_col_name(lp, i, buf);
		glp_set_col_bnds(lp, i, GLP_LO, 0.0, 0.0); // 0 <= v[i] < infty

		if(i == 1)
			glp_set_obj_coef(lp, i, 1); 
		else
			glp_set_obj_coef(lp, i, 0.0); 
	}

	int counter = 1;
	// set value on row q1 ... qD
	for (i = 1; i <= D; i++) {
		for (j = 1; j <= M; j++) {

			if(j == 1)
			{
				ia[counter] = i; ja[counter] = j;
				ar[counter++] = -mean->attr[i-1];
			}
			else
			{
				ia[counter] = i; ja[counter] = j;
				ar[counter++] = B[j-2]->attr[i-1];
			}
		}
	}



	// loading data  
	glp_load_matrix(lp, counter - 1, ia, ja, ar);

										  // running simplex
	glp_smcp parm;
	glp_init_smcp(&parm);
	parm.msg_lev = GLP_MSG_OFF; // turn off all message by glp_simplex 
	
	glp_simplex(lp, &parm);


	//printf("solution is optimal: %d\n", GLP_OPT);
	//printf("no feasible solution: %d\n", GLP_NOFEAS);
	bool feasible = glp_get_status(lp) == GLP_NOFEAS;

	theta = glp_get_obj_val(lp);
	for(int i = 0; i < D; i++)
	{
		pi->attr[i] = glp_get_row_dual(lp, i + 1);
	}
	//printf("return primal: %lf\n", glp_get_obj_val(lp));
	//for (i = 0; i < M; i++)
	//{
	//	double v = glp_get_col_prim(lp, i + 1);
	//	printf("p%d = %lf\n", i + 1, v);
	//}

	//printf("return dual: \n");
	//double dot_v = 0;
	//for (i = 0; i < D; i++)
	//{
	//	double v = glp_get_row_dual(lp, i + 1);
	//	dot_v += v * b->coord[i];
	//	printf("d%d = %lf\n", i + 1, v);
	//}
	//printf("dual objective: %lf\n", dot_v);

	glp_delete_prob(lp); // clean up
	delete[]ia;
	delete[]ja;
	delete[]ar;

	//printf("LP-verify:\n");
	//if(theta > 0 && insideCone(B, b) || isZero(theta) && !insideCone(B, b))
	//	printf("In-out error.\n");
	//if(!isZero(dot_prod(pi, b) - theta))
	//	printf("Objective value error.\n");
	//for(int i = 0; i < B.size(); i++)
	//{
	//	double v = dot_prod(B[i], pi);
	//	if(v >0 && !isZero(v))
	//		printf("Hyperplane error.\n");
	//}
	//double v = dot_prod(mean,pi);
	//if(v > -1 && !isZero(v+1))
	//	printf("Mean vector error.\n");

	delete mean;
}


/**
 * @brief Check whether there are vector which could be combined by others
 *        Delete all the vectors which could be constructed by others
 * @param p_set The original vectors
 * @param v     The newly added vector
 * @param dim   The number of dimension
 * @return      1   v cannot be combined by the original vectors
 *              0   v can be combined by the original vectors
 */
bool is_constructed(std::vector< std::pair<double*, int>> &p_set, double *v, int dim)
{
    int M = p_set.size();
    if(M < 2)
    	return false;
    // D + 2variables: D for dim, 2 for additional var for feasible
    int *ia = new int[(dim + 1) * M + 1];  //TODO: delete
    int *ja = new int[(dim + 1) * M + 1];  //TODO: delete
    double* ar = new double[(dim + 1) * M + 1];   //TODO: delete

    glp_prob *lp;
    lp = glp_create_prob();
    glp_set_prob_name(lp, "is_constructed");
    glp_set_obj_dir(lp, GLP_MAX);
    glp_add_rows(lp, dim + 1);  // add dim+1 rows: q_1, ..., q_{dim+1}
    for (int i = 1; i <= dim; i++) {
        char buf[10];
        sprintf(buf, "q%d", i);
        glp_set_row_name(lp, i, buf);
        glp_set_row_bnds(lp, i, GLP_FX, v[i - 1], v[i - 1]); // q_i
    }
    char buf[10];
    sprintf(buf, "q%d", dim + 1);
    glp_set_row_name(lp, dim + 1, buf);
    glp_set_row_bnds(lp, dim + 1, GLP_FX, 1.0, 1.0); // q_{dim+1} = 1

    glp_add_cols(lp, M);    // add M columns: v_1, ..., v_D
    for (int i = 1; i <= M; i++) {
        char buf[10];
        sprintf(buf, "v%d", i);
        glp_set_col_name(lp, i, buf);
        glp_set_col_bnds(lp, i, GLP_LO, 0.0, 0.0); // 0 <= v[i] < infty
        glp_set_obj_coef(lp, i, 0.0);  // objective: 0
    }


    int counter = 1;
    // set value on row q1 ... qD
    for (int i = 1; i <= dim + 1; ++i) {
        for (int j = 1; j <= M; ++j)
        {
            ia[counter] = i; ja[counter] = j;
            if(i <= dim)
                ar[counter++] = p_set[j-1].first[i-1];
            else
                ar[counter++] = 1;
        }
    }


    // loading data
    glp_load_matrix(lp, counter - 1, ia, ja, ar);
    // running simplex
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
 * @brief Check whether there are vector which could be combined by others
 *        Delete all the vectors which could be constructed by others
 * @param p_set The original vectors
 * @param v     The newly added vector
 * @param dim   The number of dimension
 * @return      1   v cannot be combined by the original vectors
 *              0   v can be combined by the original vectors
 */
bool is_constructed(std::vector<double*> &p_set, double *v, int dim)
{
    int M = p_set.size();
    if(M < 1)
        return false;
    // D + 2variables: D for dim, 2 for additional var for feasible
    int *ia = new int[(dim + 1) * M + 1];  //TODO: delete
    int *ja = new int[(dim + 1) * M + 1];  //TODO: delete
    double* ar = new double[(dim + 1) * M + 1];   //TODO: delete

    glp_prob *lp;
    lp = glp_create_prob();
    glp_set_prob_name(lp, "is_constructed");
    glp_set_obj_dir(lp, GLP_MAX);
    glp_add_rows(lp, dim + 1);  // add dim+1 rows: q_1, ..., q_{dim+1}
    for (int i = 1; i <= dim; i++) {
        char buf[10];
        sprintf(buf, "q%d", i);
        glp_set_row_name(lp, i, buf);
        glp_set_row_bnds(lp, i, GLP_FX, v[i - 1], v[i - 1]); // q_i
    }
    char buf[10];
    sprintf(buf, "q%d", dim + 1);
    glp_set_row_name(lp, dim + 1, buf);
    glp_set_row_bnds(lp, dim + 1, GLP_FX, 1.0, 1.0); // q_{dim+1} = 1

    glp_add_cols(lp, M);    // add M columns: v_1, ..., v_D
    for (int i = 1; i <= M; i++) {
        char buf[10];
        sprintf(buf, "v%d", i);
        glp_set_col_name(lp, i, buf);
        glp_set_col_bnds(lp, i, GLP_LO, 0.0, 0.0); // 0 <= v[i] < infty
        glp_set_obj_coef(lp, i, 1.0);  // objective: 0
    }


    int counter = 1;
    // set value on row q1 ... qD
    for (int i = 1; i <= dim + 1; ++i) {
        for (int j = 1; j <= M; ++j)
        {
            ia[counter] = i; ja[counter] = j;
            if(i <= dim)
                ar[counter++] = p_set[j-1][i-1];
            else
                ar[counter++] = 1;
        }
    }


    // loading data
    glp_load_matrix(lp, counter - 1, ia, ja, ar);
    // running simplex
    glp_smcp parm;
    glp_init_smcp(&parm);
    parm.msg_lev = GLP_MSG_OFF; // turn off all message by glp_simplex
    glp_simplex(lp, &parm);

    double w1 = glp_get_prim_stat(lp);
    //cout<<"status: "<<w1<<"\n";
    double *x = new double[M];
    for(int i = 0; i < M; i++)
        x[i] = glp_get_col_prim(lp, i+1);

    bool result = true;
    for(int i = 0; i < dim; i++)
    {
        double sum = 0;
        for(int j = 0; j < M; j++)
            sum += x[j]*p_set[j][i];
        if(v[i] - sum < -EQN4 || v[i] - sum > EQN4)
        {
            result = false;
            break;
        }
    }
    glp_delete_prob(lp); // clean up
    delete[]ia;
    delete[]ja;
    delete[]ar;
    return result;
}