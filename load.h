#include <ostream>

#include "./transform_output.h"

typedef std::vector<SVector> DuplicateINodeOutput;

std::string joinVectorString(const SVector &vector_string, char delimiter);
DuplicateINodeOutput joinDuplicateINodePaths(
    const DuplicateINodesSet &duplicate_i_nodes_set, char delimiter);
void printDuplicateINodeSet(
    std::ostream &console,
    const DuplicateINodeOutput &duplicate_i_nodes_output);
int countShortestPath(const Paths &paths);
bool comparePath(const SVector &path_one, const SVector &path_two);
bool shortestPathAndLeastCount(const Paths &paths_one, const Paths &paths_two);
void sortPaths(Paths &paths);
void sortDuplicateINodesSet(DuplicateINodesSet &duplicate_i_nodes_set);
void load(std::ostream &console,
          DuplicateINodesSet &duplicate_i_nodes_set);