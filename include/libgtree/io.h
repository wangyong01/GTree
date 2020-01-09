//
// Created by 王勇 on 2018/7/7.
//

#ifndef GTREE_INCLUDE_LIBGTREE_IO_H_
#define GTREE_INCLUDE_LIBGTREE_IO_H_

#include <fstream>
#include <unordered_map>
#include "gtree.h"
#include "matrix.h"

using namespace std;

namespace gtree {

template<typename T>
inline void write_value(std::ofstream &os, const T &value) {
    os.write((const char *) &value, sizeof value);
}

template<typename T>
inline void read_value(std::ifstream &is, T &value) {
    is.read((char *) &value, sizeof value);
}


//vector
template<typename T>
void write_value(std::ofstream &os, const std::vector<T> &vec) {
    write_value(os, vec.size());
    for (auto &e:vec)
        write_value(os, e);
}

template<typename T>
void read_value(std::ifstream &is, std::vector<T> &vec) {
    typename std::vector<T>::size_type vsize;
    read_value(is, vsize);
    vec = std::vector<T>(vsize);
    for (auto &e:vec)
        read_value(is, e);
}

template<typename A, typename B>
void write_value(std::ofstream &os, const std::unordered_map<A, B> &m) {
    write_value(os, m.size());
    for (auto &e:m) {
        write_value(os, e.first);
        write_value(os, e.second);
    }
};

template<typename A, typename B>
void read_value(std::ifstream &is, std::unordered_map<A, B> &m) {
    typename std::unordered_map<A, B>::size_type msize;
    read_value(is, msize);
    for (int i = 0; i < msize; i++) {
        std::pair<A, B> e;
        read_value(is, e.first);
        read_value(is, e.second);
        m.insert(e);
    }
};


void write_matrix(std::ofstream &os, Matrix &mat) {
    write_value(os, mat.n);
    for (int i = 0; i < mat.n; i++)
        for (int j = 0; j < mat.n; j++)
            write_value(os, mat[i][j]);
}

void read_matrix(std::ifstream &is, Matrix &mat) {
    read_value(is, mat.n);
    mat.init(mat.n);
    for (int i = 0; i < mat.n; i++)
        for (int j = 0; j < mat.n; j++)
            read_value(is, mat[i][j]);
}

void write_tree_node(std::ofstream &os, Node &node) {
    write_value(os, node.id);
    write_value(os, node.father);
    write_value(os, node.depth);

    write_matrix(os, node.matrix);
    write_matrix(os, node.interv);

    write_value(os, node.children);
    write_value(os, node.gid2bid);
    write_value(os, node.bid_f);
    write_value(os, node.bid_c);
    write_value(os, node.bid2gid);
}

void read_tree_node(std::ifstream &is, Node &node) {
    read_value(is, node.id);
    read_value(is, node.father);
    read_value(is, node.depth);

    read_matrix(is, node.matrix);
    read_matrix(is, node.interv);

    read_value(is, node.children);
    read_value(is, node.gid2bid);
    read_value(is, node.bid_f);
    read_value(is, node.bid_c);
    read_value(is, node.bid2gid);
}

void write_GTree(GTree &tree) {
    std::ofstream os(index_path, ios::binary);
    assert(os.is_open());
    write_value(os, FANOUT);
    write_value(os, MAX_LEAF_SIZE);
    write_value(os, tree.nodes_num);
    write_value(os, tree.tree_build_time);
    write_value(os, tree.gid2leafid);

    for (int i = 1; i <= tree.nodes_num; i++) {
        write_tree_node(os, tree.nodes[i]);
    }
    os.close();
}

void read_GTree(GTree &tree) {
    std::ifstream is(index_path, ios::binary);
    read_value(is, FANOUT);
    read_value(is, MAX_LEAF_SIZE);
    read_value(is, tree.nodes_num);
    read_value(is, tree.tree_build_time);
    read_value(is, tree.gid2leafid);

    tree.nodes = new Node[tree.nodes_num + 1];
    for (int i = 1; i <= tree.nodes_num; i++)
        read_tree_node(is, tree.nodes[i]);
    is.close();
}

}  // namespace gtree

#endif  // GTREE_INCLUDE_LIBGTREE_IO_H_

