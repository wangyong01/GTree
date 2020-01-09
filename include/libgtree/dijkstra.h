//
// Created by Yong on 2019/4/28.
//

#ifndef GTREE_INCLUDE_LIBGTREE_DIJKSTRA_H_
#define GTREE_INCLUDE_LIBGTREE_DIJKSTRA_H_

#include <iostream>
#include <queue>
#include <vector>
#include "graph.h"
#include "misc.h"

using namespace std;

namespace gtree {

long dijkstra(int s, Graph &G, vector<long> &dists) {
    auto cmp = [](std::pair<int, long> left, std::pair<int, long> right) {
        return left.second > right.second;
    };
    dists = vector<long>(G.n, INT_MAX);
    std::priority_queue<std::pair<int, long>, std::vector<std::pair<int, long>>, decltype(cmp)>
            vexs_pq(cmp);
    vexs_pq.emplace(s, 0);
    dists[s] = 0;
    while (!vexs_pq.empty()) {
        auto vex = vexs_pq.top();
        vexs_pq.pop();
        if (vex.second != dists[vex.first]) //only settle for entry with shortest travel time
            continue;
        for (int j = G.head[vex.first]; j != -1; j = G.next[j]) {
            long dist_new = G.weights[j] + vex.second;
            if (dist_new < dists[G.adjv[j]]) { // relax adj vertices
                dists[G.adjv[j]] = dist_new;
                vexs_pq.emplace(G.adjv[j], dist_new);
            }
        }
    }
    return -1;
}

/**
 * return (visited vertices, dist)
 * @param s
 * @param t
 * @param G
 * @return
 */
pair<int, long> dijkstra(int s, int t, Graph &G, vector<int> &prevs) {
    auto cmp = [](std::pair<int, long> left, std::pair<int, long> right) {
        return left.second> right.second;
    };
    vector<long> dists = vector<long>(G.n, INT_MAX);
    prevs = vector<int>(G.n);
    std::priority_queue<std::pair<int, long>, std::vector<std::pair<int, long >>, decltype(cmp)>
            vexs_pq(cmp);
    vexs_pq.emplace(s, 0);
    dists[s] = 0;
    prevs[s] = s;
    int cnt_vexs = 0;
    while (!vexs_pq.empty()) {
        auto vex = vexs_pq.top();
        vexs_pq.pop();
        if (vex.first == t)
            return make_pair(cnt_vexs, vex.second);
        if (vex.second!= dists[vex.first]) //only settle for entry with shortest travel time
            continue;
        cnt_vexs++;
        for (int j = G.head[vex.first]; j != -1; j = G.next[j]) {
            long dist_new = G.weights[j] + vex.second;
            if (dist_new < dists[G.adjv[j]]) { // relax adj vertices
                dists[G.adjv[j]] = dist_new;
                prevs[G.adjv[j]] = vex.first;
                vexs_pq.emplace(G.adjv[j], dist_new);
            }
        }
    }
    return make_pair(-1, -1);
}


pair<int, long> dijkstra(int s, int t, Graph &G) {
    auto cmp = [](std::pair<int, long> left, std::pair<int, long> right) {
        return left.second> right.second;
    };
    vector<long> dists = vector<long>(G.n, INT_MAX);
    std::priority_queue<std::pair<int, long>, std::vector<std::pair<int, long >>, decltype(cmp)>
            vexs_pq(cmp);
    vexs_pq.emplace(s, 0);
    dists[s] = 0;
    int cnt_vexs = 0;
    while (!vexs_pq.empty()) {
        auto vex = vexs_pq.top();
        vexs_pq.pop();
        if (vex.first == t)
            return make_pair(cnt_vexs, vex.second);
        if (vex.second!= dists[vex.first]) //only settle for entry with shortest travel time
            continue;
        cnt_vexs++;
        for (int j = G.head[vex.first]; j != -1; j = G.next[j]) {
            long dist_new = G.weights[j] + vex.second;
            if (dist_new < dists[G.adjv[j]]) { // relax adj vertices
                dists[G.adjv[j]] = dist_new;
                vexs_pq.emplace(G.adjv[j], dist_new);
            }
        }
    }
    return make_pair(-1, -1);
}

}  // namespace gtree

#endif  // GTREE_INCLUDE_LIBGTREE_DIJKSTRA_H_

