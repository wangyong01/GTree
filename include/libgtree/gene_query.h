//
// Created by Yong on 2019/5/7.
//

#ifndef GTREE_INCLUDE_LIBGTREE_GENE_QUERY_H_
#define GTREE_INCLUDE_LIBGTREE_GENE_QUERY_H_

#include <string>
#include <random>
#include <fstream>
#include "misc.h"

namespace gtree {

//generate single pair shortest path queries
void SPSP_Generator(const std::string &inPath = graph_path, const std::string &outPath = query_path) {
    std::random_device dev{};
    std::mt19937 gen{dev()};
    gen.seed(666666);
    const int queryNum = 10000;
    std::ifstream in(inPath);
    std::ofstream out(outPath);
    int n;
    in >> n;
    in.close();
    std::uniform_int_distribution<> uniint(0, n - 1);
    out << queryNum << std::endl;
    for (int i = 0; i < queryNum; i++) {
        int S = uniint(gen);
        int T = uniint(gen);
        out << S << " " << T << std::endl;
    }
    out.close();
}

}  // namespace gtree

#endif  // GTREE_INCLUDE_LIBGTREE_GENE_QUERY_H_

