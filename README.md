# GTree
Indexing and Answering Single Pair Shortest Path Query for RoadNetworks

GTree can index directed and undirected graphs, and answering shortest path querying very fast, i.e., several 0.1 ms on very large graph datasets (1~20 million of vertices) from http://users.diag.uniroma1.it/challenge9/download.shtml.
Build index with buildTree(), and then use write_GTree() to write the index into index_file.
We only need to build index once, and then load index anytime for shortest path querying.

### Usage
Copy `include/libgtree.h`, `include/libgtree` folder into your
`CPLUS_INCLUDE_PATH` (on Linux, typically /usr/local/lib). Then simply include
libgtree.h in your own C++ project. See `src/` folder for examples.

### Input
Input Graph File:

  First line with n vertices and m edges, following m lines of u v w, i.e., edge from u to v with weight w.

See data/CAL.gr for an example

Example:

```
1070376 2712798
1 2 35469
2 1 35469
5 6 5497
6 5 5497
7 5 3641
5 7 3641
......
```

