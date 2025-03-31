#include <ostream>
#include <string>

#include "./transform_output.h"

typedef std::vector<std::string> str_path_segments;
typedef std::vector<str_path_segments> str_duplicate_path_segments;
typedef std::vector<str_duplicate_path_segments> str_duplicate_path_seg_set;
typedef std::vector<std::vector<std::string>> duplicate_inode_output;

str_duplicate_path_seg_set convertToStrings(duplicate_path_seg_set const &set);
std::string joinVectorString(str_path_segments const &vector_string,
                             char delimiter);
duplicate_inode_output
joinDuplicateINodePaths(str_duplicate_path_seg_set const &duplicate_i_nodes_set,
                        char delimiter);
void printDuplicateINodeSet(
    std::ostream &console,
    duplicate_inode_output const &duplicate_i_nodes_output);
int countShortestPath(str_duplicate_path_segments const &paths);
bool comparePath(str_path_segments const &path_one,
                 str_path_segments const &path_two);
bool shortestPathAndLeastCount(str_duplicate_path_segments const &paths_one,
                               str_duplicate_path_segments const &paths_two);
void sortPaths(str_duplicate_path_segments &paths);
void sortDuplicateINodesSet(str_duplicate_path_seg_set &duplicate_i_nodes_set);
void load(std::ostream &console, duplicate_path_seg_set &duplicate_i_nodes_set);