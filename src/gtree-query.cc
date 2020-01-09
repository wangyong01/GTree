#include "../include/libgtree.h"

const int REQUIRED_ARGS = 3;

std::string arg1 = "";  // *.gtree file
int arg2 = -1;  // from vertex
int arg3 = -1;  // to vertex

void run() {
  gtree::GTree gtree;
  gtree::index_path = arg1;
  gtree::read_GTree(gtree);
  long dist = gtree.shortest_path_querying(arg2, arg3);
  std::vector<int> path;
  gtree.path_recovery(arg2, arg3, path);
  std::cout << arg2 << " to " << arg3 << "\tshortest distance\t" << dist << "\t";
  for (const int& i : path) {
    std::cout << i << " ";
  }
  std::cout << std::endl;
}

void print_help() {
  std::cout << "usage: gtree-query GTREE FROM TO\n";
}

int main(int argc, char**argv) {
  std::vector<std::string> args(argv, (argv + argc));
  if (argc < (REQUIRED_ARGS + 1)) {
    print_help();
  } else {
    // Required arguments
    int j = (argc - REQUIRED_ARGS);
    arg1 = args.at((j + 0));
    arg2 = std::stoi(args.at((j + 1)));
    arg3 = std::stoi(args.at((j + 2)));

    // Optional arguments
    // ...

    // Launch app
    std::cout
        << "set gtree file '" << arg1 << "'\n"
        << "set from '" << arg2 << "'\n"
        << "set to '" << arg3 << "'\n";
    run();
  }
  return 0;
}

