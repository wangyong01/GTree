#include "../include/libgtree.h"

const int REQUIRED_ARGS = 1;

std::string arg1 = "";  // edges *.edges file
std::string opt_o = "out.gtree";  // output
int opt_I = 0;  // 0-indexed or 1-indexed

void run() {
  gtree::GTree gtree;
  gtree::graph_path = arg1;
  gtree::index_path = opt_o;
  gtree::VEX_ID_START_WITH_ZERO = (opt_I == 0);
  gtree.buildTree();
  gtree::write_GTree(gtree);
}

void print_help() {
  std::cout << "usage: gtree-build [-I INDEXING] [-o OUTPUT] EDGES\n";
}

int main(int argc, char**argv) {
  std::vector<std::string> args(argv, (argv + argc));
  if (argc < (REQUIRED_ARGS + 1)) {
    print_help();
  } else {
    // Required arguments
    int j = (argc - REQUIRED_ARGS);
    arg1 = args.at((j + 0));

    // Optional arguments
    int i = 0;
    while (++i < j) {
      if (args.at(i) == "-o") {
        opt_o = args.at((i + 1));
      } else if (args.at(i) == "-I") {
        opt_I = std::stoi(args.at((i + 1)));
      }
    }

    // Launch app
    std::cout
        << "set edges file '" << arg1 << "'\n"
        << "  opt. -o (output) '" << opt_o << "'\n";
    run();
  }
  return 0;
}

