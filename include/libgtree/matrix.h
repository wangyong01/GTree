//
// Created by Yong on 2019/5/6.
//

#ifndef GTREE_INCLUDE_LIBGTREE_MATRIX_H_
#define GTREE_INCLUDE_LIBGTREE_MATRIX_H_

#include <memory>
#include <vector>

namespace gtree {

struct Matrix {
    unsigned long n;  //TMatrix size is n*n
    long *a;//TMatrix
    Matrix() : n(0), a(nullptr) {}

    ~Matrix() { delete[] a; }

    void init(unsigned long n, long default_value = INT_MAX) {//wondering any use of the diag
        this->n = n;
        a = new long[n * n];

        for (int i = 0; i < n * n; i++) {
            a[i] = INT_MAX;
        }
        for (int i = 0; i < n; i++) {
            a[i * n + i] = 0;
        }
    }

    long *operator[](int x) { return a + x * n; }
};

}

#endif  // GTREE_INCLUDE_LIBGTREE_MATRIX_H_

