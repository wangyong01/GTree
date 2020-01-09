//
// Created by Yong on 2019/4/28.
//

#ifndef GTREE_INCLUDE_LIBGTREE_GRAPH_H_
#define GTREE_INCLUDE_LIBGTREE_GRAPH_H_

#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>
#include "metis.h"
#include "misc.h"

namespace gtree {

struct Graph {
    unsigned long n, m;
    int mCount; //添加边数量计数
    std::vector<int> id, head, next, adjv;   //id存放顶点的真实id
    std::vector<long> weights;   //存放每个时间段对应的权值，即通过时间

    Graph() {
        n = 0;
        m = 0;
        mCount = 0;
    }

    ~Graph() {
        head.clear();
        next.clear();
        adjv.clear();
        weights.clear();
    }

    //初始化容器
    void init(unsigned long _n, unsigned long _m) {
        this->n = _n;
        this->m = _m;
        id = std::vector<int>(n, -1);
        head = std::vector<int>(n, -1);     //初始化邻接表结构
        next = std::vector<int>(m, -1);
        adjv = std::vector<int>(m, -1);
        weights = std::vector<long>(m);
    }


    void readOriginalGraph(const std::string &graph_path) {
        std::ifstream in(graph_path);
        assert(in.is_open());
        unsigned int n, m;
        in >> n >> m;
        init(n, m);
        int vs, vt;
        long weight;
        while (in >> vs >> vt >> weight) {//input edges
            if (!VEX_ID_START_WITH_ZERO) {
                vs--;
                vt--;
            }
            addEdge(vs, vt, weight);
            if (!DIRECTED_GRAPH)
                addEdge(vt, vs, weight);
        }
        for (int i = 0; i < n; i++)
            id[i] = i;
    }

    void generevGraph(Graph &gnew) {
        gnew.n = n;
        gnew.m = m;
        gnew.init(n, m);
        for (int i = 0; i < n; i++) {
            for (int j = head[i]; j != -1; j = next[j]) {
                gnew.addEdge(adjv[j], i, weights[j]);
            }
        }
    }

    void addEdge(int s, int t, long weight) {
        adjv[mCount] = t;
        next[mCount] = head[s];
        head[s] = mCount;
        weights[mCount] = weight;
        mCount++;
    }

    //图划分算法
    void
    Split(std::vector<int> &PClasses, idx_t nparts = static_cast<idx_t>(FANOUT))  //mark partition result in pcls_idx
    {
        idx_t options[METIS_NOPTIONS];
        memset(options, 0, sizeof(options));
        {
            METIS_SetDefaultOptions(options);
            options[METIS_OPTION_PTYPE] = METIS_PTYPE_KWAY; // _RB
            options[METIS_OPTION_OBJTYPE] = METIS_OBJTYPE_CUT; // _VOL
            options[METIS_OPTION_CTYPE] = METIS_CTYPE_SHEM; // _RM  _SHEM
            options[METIS_OPTION_IPTYPE] = METIS_IPTYPE_RANDOM; // _GROW _EDGE _NODE
            options[METIS_OPTION_RTYPE] = METIS_RTYPE_FM; // _GREEDY _SEP2SIDED _SEP1SIDED
            options[METIS_OPTION_NCUTS] = 5;  // generate how many versions, choose the least cut one
//             options[METIS_OPTION_NSEPS] = 10;
//            options[METIS_OPTION_NITER] = 10; //default
            /* balance factor, used to be 500 */
            options[METIS_OPTION_UFACTOR] = 500;
            // options[METIS_OPTION_MINCONN];
            options[METIS_OPTION_CONTIG] = 1;
            // options[METIS_OPTION_SEED];
            options[METIS_OPTION_NUMBERING] = 0;
            // options[METIS_OPTION_DBGLVL] = 0;
        }
        auto nvtxs = static_cast<idx_t>(n);
        idx_t ncon = 1;
        auto *xadj = new idx_t[n + 1];
        auto *adjncy = new idx_t[mCount];
        auto *adjwgt = new idx_t[mCount];
        auto *part = new idx_t[n];

        int xadj_pos = 1;
        int adjncy_pos = 0;

        xadj[0] = 0;
        for (int i = 0; i < n; i++) {
            for (int j = head[i]; j != -1; j = next[j]) {
                adjncy[adjncy_pos] = adjv[j];
                adjncy_pos++;
            }
            xadj[xadj_pos++] = adjncy_pos;
        }
        for (int i = 0; i < adjncy_pos; i++)           //划分与边权值无关，因此边权值设置为1
            adjwgt[i] = 1;
        int objval = 0;
        METIS_PartGraphKway(&nvtxs, &ncon, xadj, adjncy, nullptr, nullptr, adjwgt, &nparts, nullptr, nullptr, options,
                            &objval,
                            part);
        for (int i = 0; i < n; i++) {
            PClasses[i] = part[i];
        }
        delete[] xadj;
        delete[] adjncy;
        delete[] adjwgt;
        delete[] part;
    }

    //图划分算法
    int Split_test_border_num(int nparts)  //mark partition result in pcls_idx
    {
        idx_t options[METIS_NOPTIONS];
        memset(options, 0, sizeof(options));
        {
            METIS_SetDefaultOptions(options);
            options[METIS_OPTION_PTYPE] = METIS_PTYPE_KWAY; // _RB
            options[METIS_OPTION_OBJTYPE] = METIS_OBJTYPE_CUT; // _VOL
            options[METIS_OPTION_CTYPE] = METIS_CTYPE_SHEM; // _RM  _SHEM
            options[METIS_OPTION_IPTYPE] = METIS_IPTYPE_RANDOM; // _GROW _EDGE _NODE
            options[METIS_OPTION_RTYPE] = METIS_RTYPE_FM; // _GREEDY _SEP2SIDED _SEP1SIDED
            options[METIS_OPTION_NCUTS] = 3;  // generate how many versions, choose the least cut one
//             options[METIS_OPTION_NSEPS] = 10;
//            options[METIS_OPTION_NITER] = 10; //default
            /* balance factor, used to be 500 */
            options[METIS_OPTION_UFACTOR] = 600;
            // options[METIS_OPTION_MINCONN];
            options[METIS_OPTION_CONTIG] = 1;
            // options[METIS_OPTION_SEED];
            options[METIS_OPTION_NUMBERING] = 0;
            // options[METIS_OPTION_DBGLVL] = 0;
        }
        auto nvtxs = static_cast<idx_t>(n);
        idx_t ncon = 1;
        auto *xadj = new idx_t[n + 1];
        auto *adjncy = new idx_t[mCount];
        auto *adjwgt = new idx_t[mCount];
        auto *part = new idx_t[n];

        int xadj_pos = 1;
        int adjncy_pos = 0;

        xadj[0] = 0;
        for (int i = 0; i < n; i++) {
            for (int j = head[i]; j != -1; j = next[j]) {
                adjncy[adjncy_pos] = adjv[j];
                adjncy_pos++;
            }
            xadj[xadj_pos++] = adjncy_pos;
        }
        for (int i = 0; i < adjncy_pos; i++)           //划分与边权值无关，因此边权值设置为1
            adjwgt[i] = 1;
        int objval = 0;
        METIS_PartGraphKway(&nvtxs, &ncon, xadj, adjncy, nullptr, nullptr, adjwgt, &nparts, nullptr, nullptr, options,
                            &objval,
                            part);
        int border_cnt = 0;
        for (int i = 0; i < n; i++) {
            for (int j = head[i]; j != -1; j = next[j]) {
                if (part[i] != part[adjv[j]]) {
                    border_cnt++;
                    break;
                }
            }
        }
        delete[] xadj;
        delete[] adjncy;
        delete[] adjwgt;
        delete[] part;
        return border_cnt;
    }

    void buildSubgraphs(std::vector<int> &par_cls_idx, Graph *Gs[], unsigned long cls_num = FANOUT) {
        // Partition
        std::vector<unsigned long> count1(cls_num, 0);
        std::vector<unsigned long> m(cls_num, 0);
        std::vector<unsigned long> new_id(n);
        //统计各子图中顶点和边的个数，建立子图
        for (int i = 0; i < n; i++) {
            new_id[i] = count1[par_cls_idx[i]]++;
            for (int j = head[i]; j != -1; j = next[j]) {
                if (par_cls_idx[i] == par_cls_idx[adjv[j]])
                    m[par_cls_idx[i]]++;
            }
        }
        //添加各子图对应的边
        for (int t = 0; t < cls_num; t++) {
            (*Gs[t]).init(count1[t], m[t]);
            for (int i = 0; i < n; i++)
                if (par_cls_idx[i] == t)
                    for (int j = head[i]; j != -1; j = next[j])
                        if (par_cls_idx[i] == par_cls_idx[adjv[j]]) {
                            (*Gs[t]).addEdge(new_id[i], new_id[adjv[j]], weights[j]);
                        }
        }
        //记录子图顶点的初始id
        for (int i = 0; i < n; i++)
            (*Gs[par_cls_idx[i]]).id[new_id[i]] = id[i];
    }
};

}  // namespace gtree

#endif  // GTREE_INCLUDE_LIBGTREE_GRAPH_H_

