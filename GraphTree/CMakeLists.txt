cmake_minimum_required(VERSION 3.10)
project(GraphTree)

include_directories(/usr/local/include util entity ./)
link_directories(/usr/local/lib)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "-O3 -DNDEBUG")
#set(CMAKE_CXX_FLAGS "-O3")

add_executable(GraphTree
        main.cpp
        util/io.h util/misc.h util/constants.h util/gene_query.h util/debug_funcs.h
        entity/Graph.h entity/Matrix.h entity/Node.h
        Dijkstra.h GTree.h)
target_link_libraries(GraphTree libmetis.a)