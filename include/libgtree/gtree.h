//
// Created by Yong on 2019/5/6.
//

#ifndef GTREE_INCLUDE_LIBGTREE_GTREE_H_
#define GTREE_INCLUDE_LIBGTREE_GTREE_H_

#include <algorithm>
#include <climits>
#include <cmath>
#include <cstring>
#include <iostream>
#include <memory>
#include <queue>
#include <unordered_map>
#include <vector>

#include "dijkstra.h"
#include "misc.h"
#include "node.h"

using namespace std;

namespace gtree {

struct GTree {

    int nodes_num;
    Node *nodes;
    unordered_map<int, int> gid2leafid; // mapping global vertex id to its tree leaf node id

    long long tree_build_time;

    GTree() : nodes_num(0), nodes(nullptr), tree_build_time(INT_MAX) {}

    ~GTree() { delete[] nodes; }

    /**
     * synchronize borders generated in cur node into border list of its child nodes
     * i.e., borders cut in upper partitioning should be borders in its descendant nodes
     * @param p cur node id
     */
    void sync_b2child_nodes(int p) {
        for (int lgid : nodes[p].bid2lgid)
            nodes[p].pcls_idx[lgid] = -nodes[p].pcls_idx[lgid] - 1;
        vector<int> cnt_son_id(nodes[p].children.size(), 0);// replay lgid gene proc in child node
        for (int i = 0; i < nodes[p].G.n; i++) {
            if (nodes[p].pcls_idx[i] < 0) {
                nodes[p].pcls_idx[i] = -nodes[p].pcls_idx[i] - 1;
                int cid = nodes[p].children[nodes[p].pcls_idx[i]];
                nodes[cid].add_border(nodes[p].G.id[i], cnt_son_id[nodes[p].pcls_idx[i]]);
                int gid = nodes[p].G.id[i];
                int bid = nodes[p].gid2bid[gid];
                nodes[p].bid_c[bid] = nodes[cid].gid2bid[gid];// record bid in child
            }
            cnt_son_id[nodes[p].pcls_idx[i]]++;
        }
    }

    /**
     * build cur node structure
     * @param p tree node id
     */
    void build_node(int p) {
        auto child_num = static_cast<int>(nodes[p].children.size());
        nodes[p].id = p;
        nodes[p].depth = nodes[nodes[p].father].depth + 1;
        if (nodes[p].G.n > MAX_LEAF_SIZE) { //non-leaf, partition it
            for (int i = 0; i < child_num; i++) {
                int cid = nodes_num + i + 1;
                nodes[p].children[i] = cid;
                nodes[cid].father = p;
            }
            nodes_num += child_num;
            auto **graphs = new Graph *[child_num];
            for (int i = 0; i < child_num; i++)
                graphs[i] = &nodes[nodes[p].children[i]].G;
            nodes[p].G.Split(nodes[p].pcls_idx, child_num);
            nodes[p].G.buildSubgraphs(nodes[p].pcls_idx, graphs, static_cast<unsigned long>(child_num));
            for (auto cid:nodes[p].children)
                nodes[cid].initialize();
            delete[] graphs;
        } else { //leaf node, indexing all vertices
            for (int i = 0; i < nodes[p].G.n; i++) {
                nodes[p].pcls_idx[i] = i;         //叶节点内所有顶点全划分
                gid2leafid[nodes[p].G.id[i]] = p; //标识真实顶点id对应的叶节点
            }
        }
        nodes[p].add_borders();
        nodes[p].stablize();
        nodes[p].init_matrix();
        if (nodes[p].father) {  //record bid in father
            int f = nodes[p].father;
            for (auto &e:nodes[p].gid2bid)
                if (nodes[f].gid2bid.count(e.first))
                    nodes[p].bid_f[e.second] = nodes[f].gid2bid[e.first];
        }
        if (nodes[p].G.n > MAX_LEAF_SIZE)//sync gid2bid and mat edges to child
            sync_b2child_nodes(p);
    }


    void build_nodes() {
        std::ifstream in(graph_path);
        assert(in);
        unsigned int n;
        in >> n;
        in.close();
        nodes = new Node[2 * n + 2]; //assert nodes num no more than n
        nodes[1].G.readOriginalGraph(graph_path);
        PRINT_BUILD("Building tree structure: vertices# " +
                    to_string(nodes[1].G.n) + "\t edges# " + to_string(nodes[1].G.m))
        nodes[1].initialize();
        nodes_num = 1;
        int x = 1;  //iter node_idx
        queue<int> nodes_que;
        if (Forest_Speed_Up_Query) {
            auto kpart = max_parti_size(nodes[1].G);
            nodes[x].children.resize(kpart);
        }
        nodes_que.push(1);
        while (!nodes_que.empty()) {
            x = nodes_que.front();
            nodes_que.pop();
            build_node(x);
            if (nodes[x].G.n > MAX_LEAF_SIZE) {
                for (auto &child_id:nodes[x].children)
                    nodes_que.push(child_id);
            }
        }
        PRINT_BUILD("Finish Building Tree Structure with " + to_string(nodes_num) + " nodes")
    }


    //compute the matrix in each node
    void build_node_matrices() {
        //build from leaf to root, local optimal
        clock_t tbegin, tend;
        for (int x = 1; x <= nodes_num; x++)//init_matrices_with_graph
            nodes[x].add_edges2mat();
        for (int p = nodes_num; p > 1; p--) {
            tbegin = clock();
            nodes[p].floyd();
            nodes[p].sync_dist2father(nodes[nodes[p].father]);
            tend = clock();
            if (p % DISP_FREQ == 0 || p <= nodes[1].children.size() + 1)
                PRINT_INFO("Building matrices up, " + node2tring(nodes[p]) +
                           "\ttime cost: " + to_string((tend - tbegin) / CLOCKS_PER_SEC))
        }
        tbegin = clock();
        nodes[1].floyd();
        tend = clock();
        PRINT_INFO("Building matrices up, " + node2tring(nodes[1]) +
                   "\ttime cost: " + to_string((tend - tbegin) / CLOCKS_PER_SEC))
        //refine from root to leaf, relax local optimal matrices to global optimal
        int cnt_down_updated_nodes = 0;
        for (int p = 1; p <= nodes_num; p++) {
            if (nodes[p].children[0])   //sync dists to children, updated ones enque
                for (auto &pc:nodes[p].children) {
                    if (nodes[pc].sync_dist_from_father(nodes[p])) {// updated then recal
                        tbegin = clock();
                        nodes[pc].floyd();
                        tend = clock();
                        if (pc % DISP_FREQ == 0 || pc <= nodes[1].children.size() + 1)
                            PRINT_INFO("Building matrices down, " + node2tring(nodes[pc]) +
                                       "\ttime cost: " + to_string((tend - tbegin) / CLOCKS_PER_SEC))
                        cnt_down_updated_nodes++;
                    }
                }
        }
        string info = to_string(cnt_down_updated_nodes) + "/" + to_string(nodes_num) + "(" +
                      to_string((int) ((double) cnt_down_updated_nodes / nodes_num * 100)) +
                      "%) recomputed node matrices during building down phase";
        PRINT_BUILD(info)
    }


    void buildTree() {
        PRINT_BUILD("Building tree from " + graph_path)
        clock_t tbegin, tend;
        tbegin = clock();
        build_nodes();
        build_node_matrices();
        tend = clock();
        PRINT_BUILD("Finish Building Graph Tree \t time cost: " + to_string((tend - tbegin) / CLOCKS_PER_SEC)+"s")
        PRINT_BUILD("Tree nodes num:" + to_string(nodes_num) + "\t depth:" +
                    to_string((int) (log2(nodes_num) / log2(FANOUT))))
    }


//compute shortest ditance from lfS to LCA
    void pushBordersUp(int lfS, int LCA, int fwd_snode) {
        //processing shortest path from lfS.father to LCA
        int p = lfS;
        //forwarding from lfS towards LCA
        for (; nodes[p].father != LCA; p = nodes[p].father) {
            nodes[nodes[p].father].push_up(nodes[p]);
        }
        //process shared b and record from to LCA, edge cond for up
        nodes[LCA].dist2b = std::vector<long>(nodes[LCA].gid2bid.size(), INT_MAX);
        nodes[LCA].pre_vex = std::vector<int>(nodes[LCA].gid2bid.size());
        for (auto &e:nodes[p].bid_f) {
            nodes[LCA].dist2b[e.second] = nodes[p].dist2b[e.first];
            nodes[LCA].pre_vex[e.second] = e.second;
        }
        //handle forwarding on LCA
        for (auto &e1:nodes[p].bid_f) {
            for (auto &e2:nodes[fwd_snode].bid_f) {
                int from = e1.second, to = e2.second;
                long dist_new = nodes[LCA].dist2b[from] + nodes[LCA].matrix[from][to];
                if (dist_new < nodes[LCA].dist2b[to]) {
                    nodes[LCA].dist2b[to] = dist_new;
                    nodes[LCA].pre_vex[to] = from;
                }
            }
        }
        nodes[fwd_snode].dist2b = std::vector<long>(nodes[fwd_snode].gid2bid.size(), INT_MAX);
        nodes[fwd_snode].pre_vex = std::vector<int>(nodes[fwd_snode].gid2bid.size());
        for (auto &e:nodes[fwd_snode].bid_f) {
            nodes[fwd_snode].dist2b[e.first] = nodes[LCA].dist2b[e.second];
            nodes[fwd_snode].pre_vex[e.first] = e.first;
        }
    }

//compute shortest distance from LCA to lfT
    void pushBordersDown(int lfT, int LCA, stack<int> &down, int T_) {
        int pre = LCA;
        int p = down.top();
        down.pop();
        while (!down.empty()) {
            nodes[p].push_down(nodes[pre], nodes[down.top()]);
            pre = p;
            p = down.top();
            down.pop();
        }
        //process lfT based on arr to bordres
        assert(p == lfT);
        nodes[p].dist2b = std::vector<long>(nodes[p].gid2bid.size(), INT_MAX);
        nodes[p].pre_vex = std::vector<int>(nodes[p].gid2bid.size());
        for (auto &e:nodes[p].bid_f) {
            nodes[p].dist2b[e.first] = nodes[nodes[p].father].dist2b[e.second];
            nodes[p].pre_vex[e.first] = e.first;
            long dist_new = nodes[p].dist2b[e.first] + nodes[p].matrix[e.first][T_];
            if (dist_new < nodes[p].dist2b[T_]) {
                nodes[p].dist2b[T_] = dist_new;
                nodes[p].pre_vex[T_] = e.first;
            }
        }
    }

// find least common ancestor node
    int findLCA(int x, int y, stack<int> &down) {
        if (nodes[x].depth < nodes[y].depth)
            while (nodes[y].depth > nodes[x].depth) {
                down.push(y);
                y = nodes[y].father;
            }
        else
            while (nodes[x].depth > nodes[y].depth)
                x = nodes[x].father;
        while (x != y) {
            down.push(y);
            x = nodes[x].father;
            y = nodes[y].father;
        }
        return x;
    }

    int findLCA(int x, int y) {
        if (nodes[x].depth < nodes[y].depth)
            while (nodes[y].depth > nodes[x].depth) {
                y = nodes[y].father;
            }
        else
            while (nodes[x].depth > nodes[y].depth)
                x = nodes[x].father;
        while (x != y) {
            x = nodes[x].father;
            y = nodes[y].father;
        }
        return x;
    }

/**
 *
 * @param S source vertex
 * @param T target vertex
 * @return shortest distance
 */
    long shortest_path_querying(int S, int T) {
        int lfS = gid2leafid[S], lfT = gid2leafid[T];
        int S_ = nodes[lfS].gid2bid[S], T_ = nodes[lfT].gid2bid[T];
        int LCA;
        stack<int> down;
        nodes[lfS].dist2b = vector<long>(nodes[lfS].gid2bid.size());
        nodes[lfS].pre_vex = vector<int>(nodes[lfS].gid2bid.size());
        if (lfS == lfT) {//the same leaf node, calculate arr[S_, T_](t_d)
            LCA = lfS;
            nodes[lfS].dist2b[T_] = nodes[lfS].matrix[S_][T_];
            nodes[lfS].pre_vex[T_] = S_;
        } else {// in different nodes, push arr from lfS to lfT
            for (auto &e:nodes[lfS].bid_f) {
                nodes[lfS].dist2b[e.first] = nodes[lfS].matrix[S_][e.first];
                nodes[lfS].pre_vex[e.first] = S_;
            }
            //get LCA and pushing down tree nodes path
            LCA = findLCA(lfS, lfT, down);
            //pushing arrs along passing tree nodes.
            pushBordersUp(lfS, LCA, down.top());
            pushBordersDown(lfT, LCA, down, T_);
        }
        return nodes[lfT].dist2b[T_];
    }


    void find_path_recur(long p, int S, int T, vector<int> &raw_path) {
        int S_ = nodes[p].gid2bid[S];
        int T_ = nodes[p].gid2bid[T];
        auto interv = nodes[p].interv[S_][T_];
        if (interv == INT_MAX) {//directly connected by edge
            raw_path.push_back(T);
        } else if (interv < 0) { // connected in node -interv
            find_path_recur(-interv, S, T, raw_path);
        } else { // connected in cur
            int inter_gid = nodes[p].bid2gid[interv];
            find_path_recur(p, S, inter_gid, raw_path);
            return find_path_recur(p, inter_gid, T, raw_path);
        }
    }

    /**
     * should compute shortest distance before path recovery
     * @param S source vertex
     * @param T target vertex
     * @param path complete path
     * @param LCA least common ancestor tree node
     */
    void path_recovery(int S, int T, vector<int> &path, int LCA = -1) {
        if (LCA == -1)
            LCA = findLCA(gid2leafid[S], gid2leafid[T]);
        vector<int> raw_path;
        vector<unsigned long> pieceidx;
        pieceidx.push_back(0);
        int lfS = gid2leafid[S], lfT = gid2leafid[T];
        if (lfS == lfT) {
            //找到对应节点内边界编号
            if (S != T) {
                find_path_recur(lfS, S, T, raw_path);
                pieceidx.push_back(raw_path.size());
            }
        } else {
            int q = lfT;
            int v2 = nodes[lfT].gid2bid[T];
            int v1 = nodes[q].pre_vex[v2];
            if (v1 != v2) {
                find_path_recur(q, nodes[q].bid2gid[v1], T, raw_path);
                pieceidx.push_back(raw_path.size());
            }
            do {
                v2 = nodes[q].bid_f[v1]; // round to new turn in father node
                q = nodes[q].father;
                v1 = nodes[q].pre_vex[v2];
                if (v1 == v2) //shared bor v1=v2 in cur
                    continue;
                find_path_recur(q, nodes[q].bid2gid[v1], nodes[q].bid2gid[v2], raw_path);
                pieceidx.push_back(raw_path.size());
            } while (q != LCA);
            stack<int> down;
            int p = gid2leafid[S];
            while (p != LCA) {
                down.push(p);
                p = nodes[p].father;
            }

            v2 = nodes[q].bid_c[v1]; //node pre LCA
            q = down.top();
            down.pop();
            while (q != lfS) {
                v1 = nodes[q].pre_vex[v2];
                if (v1 != v2) {
                    find_path_recur(q, nodes[q].bid2gid[v1], nodes[q].bid2gid[v2], raw_path);
                    pieceidx.push_back(raw_path.size());
                }
                v2 = nodes[q].bid_c[v1]; //node pre LCA
                q = down.top();
                down.pop();
            }
            v1 = nodes[lfS].pre_vex[v2]; //lfS
            if (v1 != v2) {
                find_path_recur(lfS, S, nodes[lfS].bid2gid[v2], raw_path);
                pieceidx.push_back(raw_path.size());
            }
        }
        path.clear();
        path.emplace_back(S);
        for (auto iter = pieceidx.rbegin(); iter + 1 != pieceidx.rend(); iter++) {
            for (unsigned long i = *(iter + 1); i != *iter; i++)
                path.emplace_back(raw_path[i]);
        }
    }

    unsigned long max_parti_size(Graph &G)//返回该结点在不超过Additional_Memory限制下最多可以划分为多少块
    {
        auto Additional_Memory = 2 * G.n * log2(G.n);
        if ((long long) G.n * G.n <= Additional_Memory) return G.n;
        unsigned long l = 2, r = (unsigned long) sqrt(Additional_Memory), mid;//二分块数
        while (l < r) {
            mid = (l + r + 1) >> 1;
            int num = G.Split_test_border_num(mid);
            if (num * num > Additional_Memory)r = mid - 1;
            else l = mid;
        }
        if (l > 512)
            return 512;
        return l;
    }
};

}  // namespace gtree

#endif  // GTREE_INCLUDE_LIBGTREE_GTREE_H_

