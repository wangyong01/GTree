//
// Created by Yong on 2019/4/25.
//

#ifndef GTREE_INCLUDE_LIBGTREE_MISC_H_
#define GTREE_INCLUDE_LIBGTREE_MISC_H_

#include <iostream>
#include <stack>
#include <vector>
#include "constants.h"

#define PRINT_INFO(info) if(VERBOSE) std::cout<<__TIME__ <<"\t"<< info << "\t" <<std::endl;
#define PRINT_BUILD(info)  std::cout<<__TIME__ <<"\t"<< info << std::endl;

namespace gtree {

void recover_path_from_prevs(std::ostream &out, int s, int d, std::vector<int> &prevs) {
    assert(prevs.size());
    std::stack<int> path;
    int vex = d;
    while (vex != s) {
        path.push(vex);
        vex = prevs[vex];
    }
    path.push(vex);
    while (!path.empty()) {
        out << path.top() << " ";
        path.pop();
    }
    out << std::endl;
}

void recover_path_from_prevs(int s, int d, std::vector<int> &prevs, std::vector<int> path_record) {
    assert(prevs.size());
    std::stack<int> path;
    int vex = d;
    while (vex != s) {
        path.push(vex);
        vex = prevs[vex];
    }
    path.push(vex);
    while (!path.empty()) {
        path_record.push_back(path.top());
        path.pop();
    }
}

template<typename T>
std::ostream &operator<<(std::ostream &out, std::vector<T> &vec) {
    for (auto &e:vec)
        out << e << ' ';
    return out;
}

}  // namespace gtree

#endif  // GTREE_INCLUDE_LIBGTREE_MISC_H_

