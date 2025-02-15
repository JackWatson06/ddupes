#include <ostream>

#include "./transform_output.h"

typedef std::vector<SVector> DuplicateINodeOutput;

std::string joinVectorString(const SVector &vector_string, char delimiter);
DuplicateINodeOutput joinDuplicateINodePaths(
    const DuplicateINodesSet &duplicate_i_nodes_set, char delimiter);
void printDuplicateINodeSet(
    std::ostream &console,
    const DuplicateINodeOutput &duplicate_i_nodes_output);
void load(std::ostream &console,
          const DuplicateINodesSet &duplicate_i_nodes_set);