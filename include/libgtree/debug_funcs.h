//
// Created by Yong on 2019/5/7.
//

#ifndef GTREE_INCLUDE_LIBGTREE_DEBUG_FUNCS_H_
#define GTREE_INCLUDE_LIBGTREE_DEBUG_FUNCS_H_

#include "gtree.h"

namespace gtree {

void check_matrix(GTree &tree) {
    for (int p = tree.nodes_num; p >= 1; p--) {
        cout << "Check : " << tree.nodes[p] << endl;
        for (auto &e1:tree.nodes[p].gid2bid)
            for (auto &e2:tree.nodes[p].gid2bid) {
                pair<int, long> result = dijkstra(e1.first, e2.first, tree.nodes[1].G);
                if (result.second != tree.nodes[p].matrix[e1.second][e2.second])
                    cout << tree.nodes[1].matrix[e1.second][e2.second] << "\t" << result.second << endl;
            }
    }
    PRINT_INFO(" Finish check matrix")
}

void check_query(GTree &tree, const string &path = query_path) {
    ifstream is(path);
    int query_num;
    is >> query_num;
    vector<int> pathr1, pathr2;
    for (int i = 0; i < query_num; i++) {
        int u, v;
        vector<int> prevs;
        is >> u >> v;
        long dist = tree.shortest_path_querying(u, v);
        tree.path_recovery(u, v, pathr1);
        pair<int, long> result = dijkstra(u, v, tree.nodes[1].G, prevs);
        recover_path_from_prevs(u, v, prevs, pathr2);
        assert(dist == result.second);
        assert(pathr1.size() == pathr2.size());
    }
}

}  // namespace gtree

#endif  // GTREE_INCLUDE_LIBGTREE_DEBUG_FUNCS_H_

