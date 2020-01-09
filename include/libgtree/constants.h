//
// Created by Yong on 2019/5/7.
//

#ifndef GTREE_INCLUDE_LIBGTREE_CONSTANTS_H_
#define GTREE_INCLUDE_LIBGTREE_CONSTANTS_H_

#include <string>
#include <vector>

namespace gtree {

// two graph and query demo
int data_id = 0;
std::vector<std::string> datasets = {"CAL", "FLA", "W"};
std::string graph_path = "../data/" + datasets[data_id] + ".gr";
std::string query_path = "../data/" + datasets[data_id] + ".query";
std::string index_path = "../data/" + datasets[data_id] + ".gtree";


unsigned long MAX_LEAF_SIZE = 33;   //vertices# threshold for continuing partition
unsigned long FANOUT = 4;   // partition fanout

bool VEX_ID_START_WITH_ZERO = false;// if smallest vex id is zero, true, is 1 then false
bool DIRECTED_GRAPH = true; // if input file has (u, v) and (v, u) then true, only one direction then false

bool Forest_Speed_Up_Query = true;  //need higher query speed, choose larger index size

bool VERBOSE = false; //print building process
unsigned long DISP_FREQ = 100;      // display the building result every 100 nodes.

}  // namespace gtree

#endif  // GTREE_INCLUDE_LIBGTREE_CONSTANTS_H_

