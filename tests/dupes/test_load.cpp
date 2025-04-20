#include <cassert>
#include <sstream>

#include "../../src/dupes/load.cpp"

/* -------------------------------------------------------------------------- */
/*                                    Tests                                   */
/* -------------------------------------------------------------------------- */
/* ---------------------------- convertToStrings ---------------------------- */
void testConvertingToStrings() {
  // Arrange
  duplicate_path_seg_set test_sorted_duplicate_inodes_set = {
      {{"test", "example_one.txt"}, {"test", "sub-dir", "example_one.txt"}},
      {{"test", "dir1"}, {"test", "dir2"}},
      {{"test", "sub-dir", "example_four.txt"},
       {"test", "sub-dir", "example_three.txt"}},
      {{"test", "dir1", "dir2", "dir3", "example_two.txt"},
       {"testing", "dir1", "dir2", "dir3", "dir4", "example_two.txt"},
       {"testing", "dir1", "dir2", "dir3", "dir4", "dir5", "example_two.txt"}}};

  // Act
  str_duplicate_path_seg_set actual_sorted_duplicate_inodes_set_string =
      convertToStrings(test_sorted_duplicate_inodes_set);

  // Assert
  str_duplicate_path_seg_set expected_sorted_duplicate_inodes_set_string = {
      {{"test", "example_one.txt"}, {"test", "sub-dir", "example_one.txt"}},
      {{"test", "dir1"}, {"test", "dir2"}},
      {{"test", "sub-dir", "example_four.txt"},
       {"test", "sub-dir", "example_three.txt"}},
      {{"test", "dir1", "dir2", "dir3", "example_two.txt"},
       {"testing", "dir1", "dir2", "dir3", "dir4", "example_two.txt"},
       {"testing", "dir1", "dir2", "dir3", "dir4", "dir5", "example_two.txt"}}};
  assert(actual_sorted_duplicate_inodes_set_string ==
         expected_sorted_duplicate_inodes_set_string);
}

/* ---------------------------- joinVectorString ---------------------------- */
void testJoiningAVectorString() {
  // Arrange
  str_path_segments test_vector_string = {"test", "test_two", "test_three",
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
  str_duplicate_path_seg_set test_duplicate_i_nodes_set = {
      {{"test", "example_one.txt"}, {"test", "sub-dir", "example_one.txt"}},
      {{"test", "dir1"}, {"test", "dir2"}}};

  // Act
  duplicate_inode_output actual_join_output =
      joinDuplicateINodePaths(test_duplicate_i_nodes_set, '/');

  // Assert
  duplicate_inode_output expected_join_output = {
      {"test/example_one.txt", "test/sub-dir/example_one.txt"},
      {"test/dir1", "test/dir2"}};
  assert(expected_join_output == actual_join_output);
}

/* ------------------------- printDuplicateINodeSet ------------------------- */
void testPrintingDuplicateToScreen() {
  // Arrange
  std::ostringstream mock_cout{};
  duplicate_inode_output test_duplicate_i_node_output = {
      {"test/example_one.txt", "test/sub-dir/example_one.txt"},
      {"test/dir1", "test/dir2"}};

  // Act
  printDuplicateINodeSet(mock_cout, test_duplicate_i_node_output);
  std::string actual_output = mock_cout.str();

  // Assert
  std::string expected_output = "2 Sets of Duplicates Found:\n"
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
  str_path_segments path_one = {"testing", "apples", "apple",          "dir3",
                                "dir4",    "dir5",   "example_two.txt"};
  str_path_segments path_two = {"testing", "apples", "test", "example_two.txt"};

  // Act
  bool actual_sort_result = comparePath(path_one, path_two);

  // Assert
  assert(actual_sort_result == true);
}

void testComparingSizeWhenBothPathsHaveEqualParts() {
  // Arrange
  str_path_segments path_one = {"testing", "apples", "example_two",    "dir3",
                                "dir4",    "dir5",   "example_two.txt"};
  str_path_segments path_two = {"testing", "apples", "example_two"};

  // Act
  bool actual_sort_result = comparePath(path_one, path_two);

  // Assert
  assert(actual_sort_result == false);
}

/* ------------------------ shortestPathAndLeastCount ----------------------- */
void testComparingPathsReturnsBefore() {
  // Arrange
  str_duplicate_path_segments paths_one = {
      {"testing", "apples", "dir2", "dir3", "dir4", "dir5", "example_two.txt"},
      {"test", "oranges", "dir2", "example_one.txt"},
  };
  str_duplicate_path_segments paths_two = {
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
  str_duplicate_path_segments paths_one = {
      {"testing", "apples", "dir2", "dir3", "dir4", "dir5", "example_two.txt"},
      {"test", "oranges", "dir2", "example_one.txt"},
      {"test", "oranges", "dir2", "example_one.txt"},
  };
  str_duplicate_path_segments paths_two = {
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
  str_duplicate_path_segments paths_one = {
      {"testing", "apples", "dir2", "dir3", "dir4", "dir5", "example_two.txt"},
      {"test", "oranges", "dir2", "example_one.txt"},
  };
  str_duplicate_path_segments paths_two = {
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
  str_duplicate_path_segments test_duplicate_paths = {
      {"testing", "apples", "dir2", "dir3", "dir4", "dir5", "example_two.txt"},
      {"test", "oranges", "dir2", "dir3", "example_two.txt"},
      {"test", "oranges", "apples", "dir3", "example_two.txt"},
      {"test", "oranges", "potatoe", "dir3", "example_two.txt"},
      {"testing", "cats", "dir2", "dir3", "dir4", "example_two.txt"}};

  // Act
  sortPaths(test_duplicate_paths);

  // Assert
  str_duplicate_path_segments expected_duplicate_paths = {
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
  str_duplicate_path_seg_set test_duplicate_i_nodes_set = {
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
  str_duplicate_path_seg_set expected_sorted_duplicate_i_nodes_set = {
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
  testConvertingToStrings();
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