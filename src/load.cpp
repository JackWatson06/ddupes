#include "load.h"

#include <algorithm>

str_duplicate_path_seg_set convertToStrings(duplicate_path_seg_set const &set) {
  str_duplicate_path_seg_set set_of_duplicate_path_seg_strings{set.size()};

  for (int i = 0; i < set.size(); ++i) {
    str_duplicate_path_segments set_of_duplicate_path_strings{set[i].size()};

    for (int j = 0; j < set[i].size(); ++j) {
      str_path_segments path_segments_string{set[i][j].size()};

      for (int x = 0; x < set[i][j].size(); ++x) {
        path_segments_string[x] = set[i][j][x];
      }
      set_of_duplicate_path_strings[j] = path_segments_string;
    }

    set_of_duplicate_path_seg_strings[i] = set_of_duplicate_path_strings;
  }

  return set_of_duplicate_path_seg_strings;
}

std::string joinVectorString(str_path_segments const &vector_string,
                             char delimiter) {
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

duplicate_inode_output
joinDuplicateINodePaths(str_duplicate_path_seg_set const &duplicate_i_nodes_set,
                        char delimiter) {
  duplicate_inode_output duplicate_output;

  for (auto &paths : duplicate_i_nodes_set) {
    std::vector<std::string> joined_paths;

    for (auto &path : paths) {
      joined_paths.push_back(joinVectorString(path, delimiter));
    }

    duplicate_output.push_back(joined_paths);
  }

  return duplicate_output;
}

void printDuplicateINodeSet(
    std::ostream &console,
    duplicate_inode_output const &duplicate_i_nodes_output) {
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
int countShortestPath(str_duplicate_path_segments const &paths) {
  if (paths.size() == 0) {
    return 0;
  }

  int shortest_vector = paths[0].size();
  for (auto &vector : paths) {
    if (vector.size() < shortest_vector) {
      shortest_vector = vector.size();
    }
  }

  return shortest_vector;
}

bool comparePath(str_path_segments const &path_one,
                 str_path_segments const &path_two) {
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

bool shortestPathAndLeastCount(str_duplicate_path_segments const &paths_one,
                               str_duplicate_path_segments const &paths_two) {
  int path_one_shortest_count = countShortestPath(paths_one);
  int path_two_shortest_count = countShortestPath(paths_two);

  if (path_one_shortest_count == path_two_shortest_count) {
    return paths_one.size() < paths_two.size();
  }

  return path_one_shortest_count < path_two_shortest_count;
}

void sortPaths(str_duplicate_path_segments &paths) {
  std::sort(paths.begin(), paths.end(), comparePath);
}

void sortDuplicateINodesSet(str_duplicate_path_seg_set &duplicate_i_nodes_set) {
  sort(duplicate_i_nodes_set.begin(), duplicate_i_nodes_set.end(),
       shortestPathAndLeastCount);

  for (auto &paths : duplicate_i_nodes_set) {
    sortPaths(paths);
  }
}

void load(std::ostream &console,
          duplicate_path_seg_set &duplicate_i_nodes_set) {
  str_duplicate_path_seg_set duplicate_path_seg_set_str =
      convertToStrings(duplicate_i_nodes_set);
  sortDuplicateINodesSet(duplicate_path_seg_set_str);
  duplicate_inode_output duplicate_i_nodes_output =
      joinDuplicateINodePaths(duplicate_path_seg_set_str, '/');
  printDuplicateINodeSet(console, duplicate_i_nodes_output);
}
