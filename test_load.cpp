#include <cassert>
#include <sstream>

#include "./load.h"

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

int main() {
  testJoiningAVectorString();
  testJoiningPathsWithDelimiter();
  testPrintingDuplicateToScreen();
}