#include <ostream>

#include "./transform_output.h"

typedef std::vector<SVector> DuplicateINodeOutput;

std::string joinVectorString(const SVector &vector_string, char delimiter);
DuplicateINodeOutput joinDuplicateINodePaths(
    DuplicateINodesSet &duplicate_i_nodes_set, char seperator);
void printDuplicateINodeSet(std::ostream &console,
                            DuplicateINodeOutput &duplicate_i_nodes_output);
