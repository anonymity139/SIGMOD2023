cmake_minimum_required(VERSION 3.15)
project(run)

set(CMAKE_CXX_STANDARD 14)
set(INC_DIR /usr/local/Cellar/glpk/5.0/include)
set(LINK_DIR /usr/local/Cellar/glpk/5.0/lib)

include_directories(.)
include_directories(Others)
include_directories(qhull)
include_directories(QuadProg)
include_directories(structure)
include_directories(UH)

include_directories(${INC_DIR})
link_directories(${LINK_DIR})
link_libraries(glpk)


add_executable(run
        Others/lp.cpp
        Others/lp.h
        Others/operation.cpp
        Others/operation.h
        Others/pruning.cpp
        Others/pruning.h
        output/ext_pt.txt
        output/hyperplane_data.txt
        output/point.txt
        output/top.txt
        Qhull/geom.cpp
        Qhull/geom.h
        Qhull/geom2.cpp
        Qhull/global.cpp
        Qhull/io.cpp
        Qhull/io.h
        Qhull/libqhull.cpp
        Qhull/libqhull.h
        Qhull/mem.cpp
        Qhull/mem.h
        Qhull/merge.cpp
        Qhull/merge.h
        Qhull/poly.cpp
        Qhull/poly.h
        Qhull/poly2.cpp
        Qhull/qhull_a.h
        Qhull/qhull_build.cpp
        Qhull/qhull_build.h
        Qhull/qset.cpp
        Qhull/qset.h
        Qhull/random.cpp
        Qhull/random.h
        Qhull/rboxlib.cpp
        Qhull/stat.cpp
        Qhull/stat.h
        Qhull/user.cpp
        Qhull/user.h
        Qhull/usermem.cpp
        Qhull/userprintf.cpp
        Qhull/userprintf_rbox.cpp
        QuadProg/Array.cc
        QuadProg/Array.hh
        QuadProg/QuadProg++.cc
        QuadProg/QuadProg++.hh
        Ranking/active_ranking.cpp
        Ranking/active_ranking.h
        structure/cluster_h.cpp
        structure/cluster_h.h
        structure/cluster_t.cpp
        structure/cluster_t.h
        structure/data_struct.h
        structure/data_utility.cpp
        structure/data_utility.h
        structure/define.h
        structure/hyperplane.cpp
        structure/hyperplane.h
        structure/hyperplane_set.cpp
        structure/hyperplane_set.h
        structure/point_set.cpp
        structure/point_set.h
        structure/point_t.cpp
        structure/point_t.h
        structure/r_node.cpp
        structure/r_node.h
        structure/relational_graph.cpp
        structure/relational_graph.h
        structure/rtree.cpp
        structure/rtree.h
        structure/s_node.cpp
        structure/s_node.h
        structure/tuple_set.cpp
        structure/tuple_set.h
        structure/tuple_t.cpp
        structure/tuple_t.h
        structure/L.h
        structure/L.cpp
        UH/frame.cpp
        UH/frame.h
        UH/maxUtility.cpp
        UH/maxUtility.h
        Combination/Combination.cpp
        Combination/Combination.h
        main.cpp
        Separation/Separation.cpp
        Separation/Separation.h
        structure/inequality.cpp
        structure/inequality.h
        structure/rnode_tree.cpp
        structure/rnode_tree.h
        Groudtruth/Groundtruth.cpp
        Groudtruth/Groundtruth.h
        Adaptive/adaptive.cpp
        Adaptive/adaptive.h
        structure/Ctree.cpp
        structure/Ctree.h Tree/Tree.cpp Tree/Tree.h structure/valueArray.cpp structure/valueArray.h structure/L.cpp structure/L.h)

target_link_libraries(run glpk)