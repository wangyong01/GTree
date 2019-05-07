//
// Created by Yong on 2019/5/7.
//

#include <iostream>
#include <iomanip>
#include <io.h>
#include "GTree.h"
#include "gene_query.h"

void command_line_query(GTree &tree) {
    cout << "Please input source and target vertices between 0 and " + to_string(tree.nodes[1].G.n) << endl;
    int s, t;
    while (cin >> s >> t) {
        long dist = tree.shortest_path_querying(s, t);
        vector<int> path;
        tree.path_recovery(s, t, path);
        cout << s << " to " << t << "\tshortest distance\t" << dist << "\t" << "path:" << path << endl;
    }
}

void run_query_exp() {
    GTree tree;
    read_GTree(tree);
    clock_t tcost_sd = 0, tcost_sp = 0, t_begin, t_end;
    ifstream is(query_path);
    int query_num;
    is >> query_num;
    vector<int> pathr1, pathr2;
    for (int i = 0; i < query_num; i++) {
        int u, v;
        vector<int> prevs;
        is >> u >> v;
        //shortest distance query
        t_begin = clock();
        tree.shortest_path_querying(u, v);
        t_end = clock();
        tcost_sd += t_end - t_begin;
        //shortest path recovery
        t_begin = clock();
        tree.path_recovery(u, v, pathr1);
        t_end = clock();
        tcost_sp += t_end - t_begin;
    }
    cout << std::fixed << setprecision(2);
    cout << "Avg query time: " << (double) tcost_sd * 1000000 / CLOCKS_PER_SEC / query_num
         << "us\t Avg path-recovery time: "
         << tcost_sp * 1000000 / CLOCKS_PER_SEC / query_num << "us" << endl;
}

void run_build_tree() {
    GTree gtree;
    gtree.buildTree();
    write_GTree(gtree);
}

void run_query_demo() {
    GTree tree;
    read_GTree(tree);
    command_line_query(tree);
}

int main() {
//    SPSP_Generator();
    run_build_tree();
    run_query_exp();
//    run_query_demo();
}



