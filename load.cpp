#include "load.h"

#include <algorithm>

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

bool comparePath(const SVector &path_one, const SVector &path_two) {
  for (int i = 0; i < path_one.size() - 1 && i < path_two.size() - 1; i++) {
    if (path_one[i] == path_two[i]) {
      continue;
    }

    return path_one[i] < path_two[i];
  }

  // The same except for the last file.
  if (path_one.size() == path_two.size()) {
    return path_one[path_one.size() - 1] < path_two[path_two.size() - 1];
  }

  return path_one.size() < path_two.size();
}

int countShortestPath(const Paths &paths) {
    if (paths.size() == 0) {
      return 0;
    }
  
    int shortest_vector = paths[0].size();
    for (const SVector &vector : paths) {
      if (vector.size() < shortest_vector) {
        shortest_vector = vector.size();
      }
    }
  
    return shortest_vector;
  }

bool shortestPathAndLeastCount(const Paths &paths_one, const Paths &paths_two) {
  int path_one_shortest_count = countShortestPath(paths_one);
  int path_two_shortest_count = countShortestPath(paths_two);

  if (path_one_shortest_count == path_two_shortest_count) {
    return paths_one.size() < paths_two.size();
  }

  return path_one_shortest_count < path_two_shortest_count;
}

void sortPaths(Paths &paths) {
  std::sort(paths.begin(), paths.end(), comparePath);
}

void sortDuplicateINodesSet(DuplicateINodesSet &duplicate_i_nodes_set) {
  sort(duplicate_i_nodes_set.begin(), duplicate_i_nodes_set.end(),
       shortestPathAndLeastCount);

  for (Paths &paths : duplicate_i_nodes_set) {
    sortPaths(paths);
  }
}

void load(std::ostream &console,
          DuplicateINodesSet &duplicate_i_nodes_set) {
  sortDuplicateINodesSet(duplicate_i_nodes_set);
  const DuplicateINodeOutput duplicate_i_nodes_output =
      joinDuplicateINodePaths(duplicate_i_nodes_set, '/');
  printDuplicateINodeSet(console, duplicate_i_nodes_output);
}
