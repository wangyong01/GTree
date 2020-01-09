//
// Created by Yong on 2019/5/6.
//

#ifndef GTREE_INCLUDE_LIBGTREE_NODE_H_
#define GTREE_INCLUDE_LIBGTREE_NODE_H_

#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>
#include "graph.h"
#include "matrix.h"
#include "misc.h"

using namespace std;

namespace gtree {

/**
 *  tree node
 */
struct Node {
    Graph G;    //graph info
    Matrix matrix, interv;  //shortest distance matrix between borders, and its intermediate vertices' matrix
    int id, father, depth; //node id，father node id, depth on tree

    std::vector<int> children;    // children nodes ids
    std::unordered_map<int, int> gid2bid;   //mapping global vertex id to border id
    std::unordered_map<int, int> bid_f; //mapping border id to its border id in father node
    std::vector<int> bid_c, bid2gid;//mapping border id to its bid in child node, mapping bid to global vertex id
    std::vector<long> dist2b;      //for query, record dists to borders in cur node
    std::vector<int> pre_vex;    //for query, record preceding vertex info

    //following only used for building tree
    std::vector<int> pcls_idx; // partitioning vertices idx, mapping local vex id to partitioning class id
    std::vector<int> bid2lgid;  //mapping border id to local subgraph id

    Node() {
        id = 0;
        father = 0;
        depth = 0;
        children = std::vector<int>(FANOUT, 0);
    }

    void initialize() { //allocate enough space for containers
        assert(G.n > 0);
        pcls_idx = std::vector<int>(G.n, -1);
        bid_c = std::vector<int>(G.n, -1);
        bid2gid = std::vector<int>(G.n, -1);
        bid2lgid = std::vector<int>(G.n, -1);
    }

    void stablize() {   // reclaim extra space
        bid_c.resize(gid2bid.size());
        bid2gid.resize(gid2bid.size());
        bid2lgid.resize(gid2bid.size());
    }

    void add_edges2mat() {     //将图边界顶点的距离添加到矩阵
        for (int bid1 = 0; bid1 < bid2lgid.size(); bid1++) {
            int lgid1 = bid2lgid[bid1];
            for (int j = G.head[lgid1]; j != -1; j = G.next[j]) {
                int lgid2 = G.adjv[j];
                if (gid2bid.count(G.id[lgid2])) { //another v is border
                    int bid2 = gid2bid[G.id[lgid2]];
                    matrix[bid1][bid2] = G.weights[j];
                    interv[bid1][bid2] = INT_MAX;
                }
            }
        }
    }

    void add_border(int gid_global, int gid_local)        //如果顶点未被添加为边界，则将其添加
    {
        auto iter = gid2bid.find(gid_global);
        if (iter == gid2bid.end()) {
            auto bid_new = static_cast<int>(gid2bid.size());
            gid2bid[gid_global] = bid_new;
            bid2gid[bid_new] = gid_global;
            bid2lgid[bid_new] = gid_local;
        }
    }

    void add_borders()                //寻找节点划分后的边界，并将其添加
    {
        for (int i = 0; i < G.n; i++) {
            int id = G.id[i];
            for (int j = G.head[i]; j != -1; j = G.next[j]) {
                if (pcls_idx[i] != pcls_idx[G.adjv[j]]) {
                    add_border(id, i);
                    break;
                }
            }
        }
    }

    void init_matrix() {//n为边界的数目
        //初始化距离和路径表，对角线距离为0，直接连接(-1)
        matrix.init(gid2bid.size());
        interv.init(gid2bid.size());
    }


    //调用n次TD迪杰斯特拉计算所有边界顶点间的最短距离
    void floyd() {
        for (int k = 0; k < gid2bid.size(); k++) {
            for (int i = 0; i < gid2bid.size(); i++) {
                for (int j = 0; j < gid2bid.size(); j++) {
                    auto dist_new = matrix[i][k] + matrix[k][j];
                    if (dist_new < matrix[i][j]) {
                        matrix[i][j] = dist_new;
                        interv[i][j] = k;   // record path
                    }
                }
            }
        }
    }

    void sync_dist2father(Node &fnode) {
        for (auto &e1:bid_f)
            for (auto &e2: bid_f) {
                fnode.matrix[e1.second][e2.second] = matrix[e1.first][e2.first];
                if (interv[e1.first][e2.first] < 0)
                    fnode.interv[e1.second][e2.second] = interv[e1.first][e2.first];
                else
                    fnode.interv[e1.second][e2.second] = -id;   // use minus version to label node
            }
    }

    bool sync_dist_from_father(Node &fnode) {
        bool changed = false;
        for (auto &e1:bid_f)
            for (auto &e2: bid_f)
                if (fnode.matrix[e1.second][e2.second] < matrix[e1.first][e2.first]) {
                    changed = true;
                    matrix[e1.first][e2.first] = fnode.matrix[e1.second][e2.second];
                    if (fnode.interv[e1.second][e2.second] < 0)
                        interv[e1.first][e2.first] = fnode.interv[e1.second][e2.second];
                    else
                        interv[e1.first][e2.first] = -fnode.id;
                }
        return changed;
    }

    void push_up(Node &pre_node) {
        dist2b = std::vector<long>(gid2bid.size(), INT_MAX);
        pre_vex = std::vector<int>(gid2bid.size());
        int cnt1 = 0, cnt2 = 0;
        vector<int> from(pre_node.bid_f.size()), to(bid_f.size());
        for (auto &e:pre_node.bid_f) { //init dist to shared b in cur
            from[cnt1++] = e.second;
            dist2b[e.second] = pre_node.dist2b[e.first];
            pre_vex[e.second] = e.second; //no pre in cur
        }
        for (auto &e:bid_f)
            if (dist2b[e.first] == INT_MAX)
                to[cnt2++] = e.first;
        for (int i = 0; i < cnt1; i++) {
            for (int j = 0; j < cnt2; j++) {
                int b1 = from[i], b2 = to[j];
                long dist_new = dist2b[b1] + matrix[b1][b2];
                if (dist_new < dist2b[b2]) {
                    dist2b[b2] = dist_new;
                    pre_vex[b2] = b1;
                }
            }
        }
    }

    void push_down(Node &pre_node, Node &fwd_node) {
        dist2b = std::vector<long>(gid2bid.size(), INT_MAX);
        pre_vex = std::vector<int>(gid2bid.size());
        int cnt1 = 0, cnt2 = 0;
        vector<int> from(bid_f.size()), to(fwd_node.bid_f.size());
        for (auto &e:bid_f) {
            from[cnt1++] = e.first;
            dist2b[e.first] = pre_node.dist2b[e.second];
            pre_vex[e.first] = e.first;
        }
        for (auto &e:fwd_node.bid_f)
            if (dist2b[e.second] == INT_MAX)
                to[cnt2++] = e.second;
        for (int i = 0; i < cnt1; i++) {
            for (int j = 0; j < cnt2; j++) {
                int b1 = from[i], b2 = to[j];
                long dist_new = dist2b[b1] + matrix[b1][b2];
                if (dist_new < dist2b[b2]) {
                    dist2b[b2] = dist_new;
                    pre_vex[b2] = b1;
                }
            }
        }
    }

    friend std::ostream &operator<<(std::ostream &out, Node &);

};

std::ostream &operator<<(std::ostream &out, Node &node) {
    if (node.children[0])
        out << "\tnleaf_id\t";
    else
        out << "\tleaf_id\t";
    out << node.id << "\t";
    out << "\tdepth: " << node.depth << "\t";
    out << "\tvexs num: " << node.G.n << "\t";
    out << "\tborders num: " << node.gid2bid.size() << "\t";
    out << "\tmat size: " << node.matrix.n * node.matrix.n << "\t";
    return out;
}

string node2tring(Node &node) {
    string nodeinfo;
    nodeinfo += "node id:" + to_string(node.id) + "\t";
    if (node.children[0])
        nodeinfo += "nonleaf\t";
    else
        nodeinfo += "leaf\t";
    nodeinfo += "depth:" + to_string(node.depth) + "\t";
    nodeinfo += "subgraphVex#" + to_string(node.G.n) + "\t";
    nodeinfo += "border#" + to_string(node.gid2bid.size()) + "\t";
    return nodeinfo;
}

}  // namespace gtree

#endif  // GTREE_INCLUDE_LIBGTREE_NODE_H_

