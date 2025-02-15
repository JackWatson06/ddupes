#include "load.h"

std::string joinVectorString(const SVector &vector_string, char delimiter) {
  std::string combined_string{""};

  auto one_before_end{--vector_string.cend()};
  for (auto iter = vector_string.cbegin(); iter != vector_string.cend();
       iter++) {
    combined_string += *iter;
    if (iter != one_before_end) {
      combined_string += delimiter;
    }
  }

  return combined_string;
}

DuplicateINodeOutput joinDuplicateINodePaths(
    const DuplicateINodesSet &duplicate_i_nodes_set, char delimiter) {
  DuplicateINodeOutput duplicate_output;

  for (const Paths &paths : duplicate_i_nodes_set) {
    SVector joined_paths;

    for (const SVector &path : paths) {
      joined_paths.push_back(joinVectorString(path, delimiter));
    }

    duplicate_output.push_back(joined_paths);
  }

  return duplicate_output;
}

void printDuplicateINodeSet(
    std::ostream &console,
    const DuplicateINodeOutput &duplicate_i_nodes_output) {
  console << duplicate_i_nodes_output.size()
          << " Sets of Duplicates Found:\n\n";

  auto one_before_end{--duplicate_i_nodes_output.end()};
  for (auto iter = duplicate_i_nodes_output.cbegin();
       iter != duplicate_i_nodes_output.end(); iter++) {
    for (auto path_iter = iter->cbegin(); path_iter != iter->cend();
         ++path_iter) {
      console << *path_iter << "\n";
    }

    if (iter != one_before_end) {
      console << "\n";
    }
  }
}

void load(std::ostream &console,
          const DuplicateINodesSet &duplicate_i_nodes_set) {
  const DuplicateINodeOutput duplicate_i_nodes_output =
      joinDuplicateINodePaths(duplicate_i_nodes_set, '/');
  printDuplicateINodeSet(console, duplicate_i_nodes_output);
}