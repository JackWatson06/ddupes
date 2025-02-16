#include <cassert>
#include <sstream>

#include "../load.h"

/* -------------------------------------------------------------------------- */
/*                                    Tests                                   */
/* -------------------------------------------------------------------------- */

/* ---------------------------- joinVectorString ---------------------------- */
void testJoiningAVectorString() {
  // Arrange
  SVector test_vector_string = {"test", "test_two", "test_three",
                                "example_one.txt"};

  // Act
  std::string actual_joined_string = joinVectorString(test_vector_string, '/');

  // Assert
  std::string expected_joined_string =
      "test/test_two/test_three/example_one.txt";
  assert(actual_joined_string == expected_joined_string);
}

/* ------------------------- joinDuplicateINodePaths ------------------------ */
void testJoiningPathsWithDelimiter() {
  // Arrange
  DuplicateINodesSet test_duplicate_i_nodes_set = {
      {{"test", "example_one.txt"}, {"test", "sub-dir", "example_one.txt"}},
      {{"test", "dir1"}, {"test", "dir2"}}};

  // Act
  DuplicateINodeOutput actual_join_output =
      joinDuplicateINodePaths(test_duplicate_i_nodes_set, '/');

  // Assert
  DuplicateINodeOutput expected_join_output = {
      {"test/example_one.txt", "test/sub-dir/example_one.txt"},
      {"test/dir1", "test/dir2"}};
  assert(expected_join_output == actual_join_output);
}

/* ------------------------- printDuplicateINodeSet ------------------------- */
void testPrintingDuplicateToScreen() {
  // Arrange
  std::ostringstream mock_cout{};
  DuplicateINodeOutput test_duplicate_i_node_output = {
      {"test/example_one.txt", "test/sub-dir/example_one.txt"},
      {"test/dir1", "test/dir2"}};

  // Act
  printDuplicateINodeSet(mock_cout, test_duplicate_i_node_output);
  std::string actual_output = mock_cout.str();

  // Assert
  std::string expected_output =
      "2 Sets of Duplicates Found:\n"
      "\n"
      "test/example_one.txt\n"
      "test/sub-dir/example_one.txt\n"
      "\n"
      "test/dir1\n"
      "test/dir2\n";

  assert(actual_output == expected_output);
}

/* ------------------------------- comparePath ------------------------------ */
void testComparingPathReturnsBefore() {
  // Arrange
  SVector path_one = {"testing", "apples", "apple",          "dir3",
                      "dir4",    "dir5",   "example_two.txt"};
  SVector path_two = {"testing", "apples", "test", "example_two.txt"};

  // Act
  bool actual_sort_result = comparePath(path_one, path_two);

  // Assert
  assert(actual_sort_result == true);
}

void testComparingSizeWhenBothPathsHaveEqualParts() {
  // Arrange
  SVector path_one = {"testing", "apples", "example_two",    "dir3",
                      "dir4",    "dir5",   "example_two.txt"};
  SVector path_two = {"testing", "apples", "example_two"};

  // Act
  bool actual_sort_result = comparePath(path_one, path_two);

  // Assert
  assert(actual_sort_result == false);
}

/* ------------------------ shortestPathAndLeastCount ----------------------- */
void testComparingPathsReturnsBefore() {
  // Arrange
  Paths paths_one = {
      {"testing", "apples", "dir2", "dir3", "dir4", "dir5", "example_two.txt"},
      {"test", "oranges", "dir2", "example_one.txt"},
  };
  Paths paths_two = {
      {"testing", "apples", "dir2", "dir3", "dir4", "dir5", "example_two.txt"},
      {"test", "oranges", "dir2", "dir3", "example_two.txt"},
  };

  // Act
  bool actual_sort_result = shortestPathAndLeastCount(paths_one, paths_two);

  // Assert
  assert(actual_sort_result == true);
}

void testComparingPathCountReturnsBefore() {
  // Arrange
  Paths paths_one = {
      {"testing", "apples", "dir2", "dir3", "dir4", "dir5", "example_two.txt"},
      {"test", "oranges", "dir2", "example_one.txt"},
      {"test", "oranges", "dir2", "example_one.txt"},
  };
  Paths paths_two = {
      {"testing", "apples", "dir2", "dir3", "dir4", "dir5", "example_two.txt"},
      {"test", "oranges", "dir2", "example_one.txt"},
      {"test", "oranges", "dir2", "example_one.txt"},
      {"test", "oranges", "dir2", "example_one.txt"},
  };

  // Act
  bool actual_sort_result = shortestPathAndLeastCount(paths_one, paths_two);

  // Assert
  assert(actual_sort_result == true);
}

void testComparingLessPathsButLonger() {
  // Arrange
  Paths paths_one = {
      {"testing", "apples", "dir2", "dir3", "dir4", "dir5", "example_two.txt"},
      {"test", "oranges", "dir2", "example_one.txt"},
  };
  Paths paths_two = {
      {"testing", "apples", "dir2", "dir3", "dir4", "dir5", "example_two.txt"},
      {"test", "example_one.txt"},
      {"test", "example_one.txt"},
  };

  // Act
  bool actual_sort_result = shortestPathAndLeastCount(paths_one, paths_two);

  // Assert
  assert(actual_sort_result == false);
}

/* -------------------------------- sortPaths ------------------------------- */
void testSortingPaths() {
  // Arrange
  Paths test_duplicate_paths = {
      {"testing", "apples", "dir2", "dir3", "dir4", "dir5", "example_two.txt"},
      {"test", "oranges", "dir2", "dir3", "example_two.txt"},
      {"test", "oranges", "apples", "dir3", "example_two.txt"},
      {"test", "oranges", "potatoe", "dir3", "example_two.txt"},
      {"testing", "cats", "dir2", "dir3", "dir4", "example_two.txt"}};

  // Act
  sortPaths(test_duplicate_paths);

  // Assert
  Paths expected_duplicate_paths = {
      {"test", "oranges", "apples", "dir3", "example_two.txt"},
      {"test", "oranges", "dir2", "dir3", "example_two.txt"},
      {"test", "oranges", "potatoe", "dir3", "example_two.txt"},
      {"testing", "apples", "dir2", "dir3", "dir4", "dir5", "example_two.txt"},
      {"testing", "cats", "dir2", "dir3", "dir4", "example_two.txt"}};

  assert(test_duplicate_paths == expected_duplicate_paths);
}

/* ------------------------- sortDuplicateINodesSet ------------------------- */
void testSortingDuplicateINodesSet() {
  // Arrange
  DuplicateINodesSet test_duplicate_i_nodes_set = {
      {{"test", "sub-dir", "example_three.txt"},
       {"test", "sub-dir", "example_four.txt"}},
      {{"test", "example_one.txt"}, {"test", "sub-dir", "example_one.txt"}},
      {{"test", "dir1"}, {"test", "dir2"}},
      {{"testing", "dir1", "dir2", "dir3", "dir4", "dir5", "example_two.txt"},
       {"test", "dir1", "dir2", "dir3", "example_two.txt"},
       {"testing", "dir1", "dir2", "dir3", "dir4", "example_two.txt"}}};

  // Act
  sortDuplicateINodesSet(test_duplicate_i_nodes_set);

  // Assert
  DuplicateINodesSet expected_sorted_duplicate_i_nodes_set = {
      {{"test", "example_one.txt"}, {"test", "sub-dir", "example_one.txt"}},
      {{"test", "dir1"}, {"test", "dir2"}},
      {{"test", "sub-dir", "example_four.txt"},
       {"test", "sub-dir", "example_three.txt"}},
      {{"test", "dir1", "dir2", "dir3", "example_two.txt"},
       {"testing", "dir1", "dir2", "dir3", "dir4", "example_two.txt"},
       {"testing", "dir1", "dir2", "dir3", "dir4", "dir5", "example_two.txt"}}};

  assert(test_duplicate_i_nodes_set == expected_sorted_duplicate_i_nodes_set);
}

/* -------------------------------------------------------------------------- */
/*                                    Main                                    */
/* -------------------------------------------------------------------------- */
int main() {
  testJoiningAVectorString();
  testJoiningPathsWithDelimiter();
  testPrintingDuplicateToScreen();
  testComparingPathReturnsBefore();
  testComparingSizeWhenBothPathsHaveEqualParts();
  testComparingPathsReturnsBefore();
  testComparingPathCountReturnsBefore();
  testComparingLessPathsButLonger();
  testSortingPaths();
  testSortingDuplicateINodesSet();
}