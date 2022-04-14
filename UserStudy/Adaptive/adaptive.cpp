#include "adaptive.h"

/**
 * @brief Used to find the estimated utility vector by max-min
 * @param V All the hyperplanes which bounds the utility range
 * @return  The estimated utility vector
 */
point_t* find_estimate(std::vector<point_t*> &V)
{
    if (V.size() == 0)
        return NULL;
    else if (V.size() == 1)
        return new point_t(V[0]);

    int dim = V[0]->d;
    double zero = 0.00000001;

    //Use QuadProg++ to solve the quadratic optimization problem
    Matrix<double> G, CE, CI;
    Vector<double> g0, ce0, ci0, x;
    int n, m, p;
    char ch;
    // Matrix G and g0 determine the objective function.
    // min 1/2 x^T*G*x + g0*x
    n = dim;
    G.resize(n, n);
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            if (i == j)
                G[i][j] = 1;
            else
                G[i][j] = 0;
        }
    }

    g0.resize(n);
    for (int i = 0; i < n; i++)
        g0[i] = 0;

    // CE and ce0 determine the equality constraints
    // CE*x = ce0
    m = 0;
    CE.resize(n, m);
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
            CE[i][j] = 0;
    }

    ce0.resize(m);
    for (int j = 0; j < m; j++)
        ce0[j] = 0;

    // CI and ci0 determine the inequality constraints
    // CI*x >= ci0
    p = V.size();
    CI.resize(n, p);
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < p; j++)
            CI[i][j] = V[j]->attr[i];
    }

    ci0.resize(p);
    for (int j = 0; j < p; j++)
        ci0[j] = -1;

    x.resize(n);

    //invoke solve_quadprog() in QuadProg++ to solve the problem for us
    // result is stored in x
    solve_quadprog(G, g0, CE, ce0, CI, ci0, x);

    //transform x into our struct
    point_t *estimate = new point_t(dim);
    for (int i = 0; i < dim; i++)
        estimate->attr[i] = x[i];
    return estimate;
}


/**
 * @brief Used to classified the hyperplanes into L clusters by k-means
          Note that by re-assign hyperplane into clusters, the number of clusters may <L finally
 * @param hyper      All the hyperplanes
 * @param clu        All the clusters obtained
 */
void k_means_cosine(std::vector<hyperplane*> hyper, std::vector<cluster_h*> &clu)
{
    int M = hyper.size(), dim = hyper[0]->d;
    int seg = M/Lnum;
    for (int i = 0; i < Lnum; ++i)
    {
        cluster_h *c = new cluster_h();
        c->center = new point_t(dim);
        for (int j = 0; j < dim; ++j)
            c->center->attr[j] = hyper[seg * i]->norm[j];
        clu.push_back(c);
    }
    double shift = 9999;
    while (shift >= 0.1)
    {
        //initial
        shift = 0;
        for (int i = 0; i < Lnum; i++)
        {
            clu[i]->h_set.clear();
        }
        //scan each hyperplane, insert into a cluster
        for (int i = 0; i < M; i++)
        {
            //Find the cluster for each hyperplane
            double cos_sim = -2;
            int index = -1;
            for (int j = 0; j < Lnum; j++)
            {
                double cs = clu[j]->center->cosine0(hyper[i]->norm);
                if (cs > cos_sim)
                {
                    cos_sim = cs;
                    index = j;
                }
            }
            clu[index]->h_set.push_back(hyper[i]);
        }
        //renew the center of each cluster
        for (int i = 0; i < Lnum; i++)
        {
            for (int j = 0; j < dim; j++)
            {
                double value = 0;
                for (int a = 0; a < clu[i]->h_set.size(); a++)
                {
                    value += clu[i]->h_set[a]->norm[j];
                }
                value /= clu[i]->h_set.size();
                shift = shift + value - clu[i]->center->attr[j];
                clu[i]->center->attr[j] = value;
            }
        }
    }
    //delete the cluster with 0 members
    int index = 0;
    for (int i = 0; i < Lnum; i++)
    {
        if (clu[index]->h_set.size() == 0)
            clu.erase(clu.begin() + index);
        else
            index++;
    }
}


/**
 * @brief Used to build the sphereical cap
          1. Find the representative vector
          2. Set the cos()
 * @param The spherical node
 */
void cap_construction(s_node *node)
{
    std::vector<point_t *> V;
    point_t *pt;
    int M = node->hyper.size(), dim = node->hyper[0]->d;
    for (int i = 0; i < M; ++i)
    {
        pt = new point_t(dim);
        for (int j = 0; j < dim; ++j)
            pt->attr[j] = node->hyper[i]->norm[j];
        V.push_back(pt);
    }
    node->center = find_estimate(V);
    node->center->normalization();
    node->angle = node->center->cosine0(node->hyper[0]->norm);
    for (int i = 1; i < M; ++i)
    {
        double angle = node->center->cosine0(node->hyper[i]->norm);
        if (angle < node->angle)
            node->angle = angle;
    }
}


/**
 * @brief Used to build the spherical tree
 * @param hyper      All the hyperplanes
 * @param node       The node of the tree. For user, only set the root node to call this function
 */
void build_spherical_tree(std::vector<hyperplane*> hyper, s_node *node)
{
    int M = hyper.size(), dim = hyper[0]->d;
    if (M <= Lnum) //build leaf
    {
        for (int i = 0; i < M; ++i)
            node->hyper.push_back(hyper[i]);
        node->is_leaf = true;
        cap_construction(node);
    }
    else //build internal node
    {
        std::vector<cluster_h*> clu;
        k_means_cosine(hyper, clu); //obtain all the clusters
        //build a node for each cluster and set the center by spherical cap construction
        int clu_size = clu.size();
        s_node *s_n;
        if (clu_size == 1)
        {
            cluster_h *c = new cluster_h();
            int countt = clu[0]->h_set.size();
            for (int j = 0; j < countt / 2; j++)
            {
                c->h_set.push_back(clu[0]->h_set[0]);
                clu[0]->h_set.erase(clu[0]->h_set.begin());
            }
            clu.push_back(c);
            clu_size = clu.size();
        }
        for (int i = 0; i < clu_size; i++)
        {
            s_n = new s_node(dim);
            build_spherical_tree(clu[i]->h_set, s_n);
            node->child.push_back(s_n);
        }
        for (int i = 0; i < M; i++)
            node->hyper.push_back(hyper[i]);
        node->is_leaf = false;
        cap_construction(node);
    }
}


/**
 * @brief Used to prune the impossible spherical caps for searching
 * @param q          The searching utility vector
 * @param S          The spherical caps for searching
 * @param Q          The spherical caps refined
 */
void spherical_cap_pruning(point_t *q, std::vector<s_node*> S, std::vector<s_node*> &Q)
{
    int M = S.size();
    Q.push_back(S[0]);
    double maxLB = S[0]->lower_orthog(q);
    for (int i = 1; i < M; i++)
    {
        double UB = S[i]->upper_orthog(q);
        double LB = S[i]->lower_orthog(q);
        if (UB > maxLB)
        {
            //deal with Q
            int index = 0, q_size = Q.size();
            for (int j = 0; j < q_size; j++)
            {
                double UB0 = Q[index]->upper_orthog(q);
                if (UB0 < LB)
                    Q.erase(Q.begin() + index);
                else
                    index++;
            }
            if (LB > maxLB) //deal with maxLB
                maxLB = LB;
            //insert into Q
            q_size = Q.size();
            int a = 0;
            for (a = 0; a < q_size; a++)
            {
                double UB0 = Q[a]->upper_orthog(q);
                if (UB < UB0)
                    break;
            }
            Q.insert(Q.begin() + a, S[i]);
        }
    }
}


/**
 * @brief Used to find the hyperplane asking question through spherical tree based on estimated u
 * @param node       The root of spherical tree
 * @param q          The estimated u
 * @param best       The best hyperplane found so far. For user, set best=NULL to call this function
 * @return   The hyperplane used to ask user question
 */
hyperplane *orthogonal_search(s_node *node, point_t *q, hyperplane *best)
{
    if (node->is_leaf)
    {
        int M = node->hyper.size();
        double ortho_value;
        if (best == NULL)
            ortho_value = 0;
        else
            ortho_value = q->orthogonality(best->norm);
        for (int i = 0; i < M; i++)
        {
            double v = q->orthogonality(node->hyper[i]->norm);
            if (v > ortho_value)
            {
                ortho_value = v;
                best = node->hyper[i];
            }
        }
        return best;
    }

    std::vector<s_node*> Q;
    spherical_cap_pruning(q, node->child, Q);
    std::stack<s_node*> S;
    for (int i = 0; i < Q.size(); i++)
        S.push(Q[i]);
    Q.clear();
    while (!S.empty())
    {
        s_node *t = S.top();
        double UB_t = t->upper_orthog(q);
        if (best == NULL || UB_t > q->orthogonality(best->norm))
            best = orthogonal_search(t, q, best);
        S.pop();
    }
    return best;
}


/**
 * @brief Find the best point by pairwise comparison.
 *        Use the cos() of real u and estimated u as the accuracy.
 *        The stop condition is that cos() should satisfy the given threshold
 * @param origset   The original dataset
 * @param u         The real utility vector
 * @param Qcount    The number of questions asked
 * @param max_value The largest utility w.r.t. u
 */
void Adaptive(tuple_set *car_set, point_set *origset, point_t* u, int &Qcount, int &TID, std::ofstream &fp)
{

    timeval t1, t2;
    gettimeofday(&t1, 0);

    double totalTime = 0, time_cost = 0;
    int M, q_idx, p_idx, maxQcount = 60, testCount = 0, correctCount = 0;
    //p_set: randomly choose 1000 points
    point_set *p_set = new point_set();
    M = origset->points.size();
    for (int i = 0; i < M; i++)
    {
        bool is_same = false;
        for (int j = 0; j < p_set->points.size(); j++)
        {
            if (p_set->points[j]->is_same(origset->points[i]))
            {
                is_same = true;
                break;
            }
        }
        if (!is_same)
            p_set->points.push_back(origset->points[i]);
    }
    p_set->random(0.5);


    //initial
    int dim = p_set->points[0]->d; M = p_set->points.size(); Qcount = 0;
    double accuracy = 0, de_accuracy = 100;

    //the normal vectors
    std::vector<point_t *> V;
    for (int i = 0; i < dim; ++i)
    {
        point_t *b = new point_t(dim);
        for (int j = 0; j < dim; ++j)
        {
            if (i == j)
                b->attr[j] = 1;
            else
                b->attr[j] = 0;
        }
        V.push_back(b);
    }

    //build a hyperplane for each pair of points
    std::vector<hyperplane*> h_set;
    for (int i = 0; i < M; ++i)
    {
        for (int j = 0; j < M; ++j)
        {
            if (i != j && !p_set->points[i]->is_same(p_set->points[j]))
            {
                hyperplane *h1 = new hyperplane(p_set->points[i], p_set->points[j], 0);
                h1->normalization();
                h_set.push_back(h1);
                hyperplane *h2 = new hyperplane(p_set->points[j], p_set->points[i], 0);
                h2->normalization();
                h_set.push_back(h2);
            }
        }
    }
    s_node *stree_root = new s_node(dim);
    build_spherical_tree(h_set, stree_root);

    point_t *estimate_u;
    for (int i = 0; i < maxQcount; i++)
    {
        Qcount++;
        hyperplane *best = NULL;
        point_t *p, *q;
        estimate_u = find_estimate(V);
        estimate_u->normalization();

        if (i % 2 == 1)
        {
            bool done = false;
            while (!done)
            {
                p_idx = rand() % M;
                q_idx = p_idx;
                while (p_idx == q_idx)
                {
                    q_idx = rand() % M;
                }
                if (!p_set->points[p_idx]->dominates(p_set->points[q_idx]) && !p_set->points[q_idx]->dominates(p_set->points[p_idx]))
                    done = true;
            }
            p = p_set->points[p_idx];
            q = p_set->points[q_idx];
        }
        else
        {
            best = orthogonal_search(stree_root, estimate_u, best);
            p = best->p_1;
            q = best->p_2;
        }

        gettimeofday(&t2, 0);
        time_cost = (double) t2.tv_sec + (double) t2.tv_usec / 1000000 - (double) t1.tv_sec - (double) t1.tv_usec / 1000000;
        totalTime += time_cost;
        int option = car_set->show_to_user(p->id, q->id);
        gettimeofday(&t1, 0);

        if (i % 2 == 0) //training
        {
            double v1 = u->dot_product(p);
            double v2 = u->dot_product(q);
            point_t *pt = new point_t(dim);
            if (option == 1) //v1 >= v2)
            {
                for (int i = 0; i < dim; ++i)
                    pt->attr[i] = best->norm[i];
            }
            else
            {
                for (int i = 0; i < dim; i++)
                    pt->attr[i] = -best->norm[i];
            }
            V.push_back(pt);
        }
        else // testing
        {
            //double v1 = u->dot_product(p);
            //double v2 = u->dot_product(q);
            testCount++;
            if ((option == 1 && p->dot_product(estimate_u) >= q->dot_product(estimate_u)) || (option == 2 && p->dot_product(estimate_u) <= q->dot_product(estimate_u)))
            //if ((v1 >= v2 && p->dot_product(estimate_u) >= q->dot_product(estimate_u)) || (v2 >= v1 && p->dot_product(estimate_u) <= q->dot_product(estimate_u)))
            {
                correctCount++;
            }
            double accuracy = ((double) correctCount) / testCount;
            printf("TotalQ:%d, TestQ: %d, Accuracy: %lf\n", i + 1, testCount, accuracy);
            if (testCount >= 5 && accuracy > 0.75)
            {
                break;
            }
        }
        estimate_u = NULL;
    }


    estimate_u = find_estimate(V);
    estimate_u->normalization();
    int maxIdx = 0, size = origset->points.size();
    double maxValue = 0;
    for (int i = 0; i < size; i++)
    {
        double value = origset->points[i]->dot_product(estimate_u); //utility of the points
        if (value > maxValue)
        {
            maxValue = value;
            maxIdx = i;
        }
    }

    gettimeofday(&t2, 0);
    time_cost = (double) t2.tv_sec + (double) t2.tv_usec / 1000000 - (double) t1.tv_sec - (double) t1.tv_usec / 1000000;
    totalTime += time_cost;
    std::cout << "-----------------------------------------------------------------------\n";
    printf("%28s: %5d \n", "Number of questions asked", Qcount);
    std::cout << "-----------------------------------------------------------------------\n";

    //out_cp << Qcount << "    " << time_cost << "\n";
    car_set->tuples[origset->points[maxIdx]->id]->final_result("Adaptive", Qcount, totalTime, fp);
    TID = origset->points[maxIdx]->id;
}