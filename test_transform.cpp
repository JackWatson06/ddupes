#include <cassert>
#include <cstdint>

#include "./transform.h"

const Hash TEST_HASH{255, 255, 255, 255, 255, 255, 255, 255,
                     255, 255, 255, 255, 255, 255, 255, 255};

bool compareHashToDuplicateNotes(DuplicatePathsMap& one,
                                 DuplicatePathsMap& two) {
  for (auto iter = one.begin(); iter != one.end(); ++iter) {
    if (one[iter->first] != two[iter->first]) {
      return false;
    }
  }

  for (auto iter = two.begin(); iter != two.end(); ++iter) {
    if (two[iter->first] != one[iter->first]) {
      return false;
    }
  }
  return true;
}

void testCountingShortestVectorLength() {
  // Arrange
  Paths paths_one = {
      {"testing", "apples", "dir2", "dir3", "dir4", "dir5", "example_two.txt"},
      {"test", "oranges", "dir2", "example_one.txt"},
  };

  // Act
  int actual_path_count = countShortestVector(paths_one);

  // Assert
  assert(actual_path_count == 4);
}

void testCountingVectorsWithLengthZeroReturnsZero() {
  // Arrange
  Paths paths_one = {};

  // Act
  int actual_path_count = countShortestVector(paths_one);

  // Assert
  assert(actual_path_count == 0);
}

void testHashingAListOfHashes() {
  // Arrange
  Hashes test_hashes{TEST_HASH, TEST_HASH, TEST_HASH};

  // Act
  Hash actual_hash = computeHash(test_hashes);

  // Assert
  Hash expected_hash{91, 83,  0,   15, 77, 131, 26, 67,
                     48, 112, 248, 0,  4,  38,  41, 16};
  assert(actual_hash == expected_hash);
}

void testTwoHashesAreDifferent() {
  // Arrange
  Hashes test_one_hashes{TEST_HASH, TEST_HASH, TEST_HASH};
  Hashes test_two_hashes{TEST_HASH, TEST_HASH};

  // Act
  Hash actual_hash_one = computeHash(test_one_hashes);
  Hash actual_hash_two = computeHash(test_two_hashes);

  // Assert
  assert(actual_hash_one != actual_hash_two);
}

void testBuildingDirectoryRowIdMap() {
  const DirectoryTableRow::Rows test_directory_results = {
      {1, "/", nullptr},
      {2, "home", new unsigned int(1)},
      {3, "dir1", new unsigned int(2)},
      {4, "dir2", new unsigned int(2)},
      {5, "sub_dir", new unsigned int(4)}};

  // Act
  DirectoryRowIdMap actual_directory_result_id_map =
      buildDirectoryRowIdMap(test_directory_results);

  // Assert
  const DirectoryRowIdMap expected_directory_result_id_map{
      {1, &test_directory_results[0]}, {2, &test_directory_results[1]},
      {3, &test_directory_results[2]}, {4, &test_directory_results[3]},
      {5, &test_directory_results[4]},
  };
  assert(expected_directory_result_id_map == actual_directory_result_id_map);

  // Cleanup
  for (const DirectoryTableRow test_directory_result : test_directory_results) {
    delete test_directory_result.parent_id;
  }
}

void testDuplicateDirectoryTreeCreation() {
  // Arrange
  const DirectoryTableRow::Rows test_directory_results = {
      {1, "/", nullptr},
      {2, "home", new unsigned int(1)},
      {3, "dir1", new unsigned int(2)},
      {4, "dir2", new unsigned int(2)},
      {5, "sub_dir", new unsigned int(4)}};
  const DirectoryRowIdMap test_directory_results_id_map{
      {1, &test_directory_results[0]}, {2, &test_directory_results[1]},
      {3, &test_directory_results[2]}, {4, &test_directory_results[3]},
      {5, &test_directory_results[4]},
  };
  const HashTableRow::Rows test_hash_results = {
      {5, "testing1.txt", Hash{128, 255, 255}},
      {4, "testing2.txt", Hash{196, 255, 255}},
      {4, "testing3.txt", Hash{200, 255, 255}},
      {3, "testing4.txt", Hash{255, 255, 255}}};

  // Act
  DirectoryNode actual_directory_tree =
      buildDirectoryTree(test_hash_results, test_directory_results_id_map);

  // Assert
  DirectoryNode expected_directory_tree{
      "/",
      {},
      {DirectoryNode{
          "home",
          {},
          {
              DirectoryNode{"dir2",
                            {
                                FileNode{"testing3.txt", Hash{200, 255, 255}},
                                FileNode{"testing2.txt", Hash{196, 255, 255}},
                            },
                            {DirectoryNode{"sub_dir",
                                           {FileNode{"testing1.txt",
                                                     Hash{128, 255, 255}}}}}},
              DirectoryNode{
                  "dir1", {FileNode{"testing4.txt", Hash{255, 255, 255}}}, {}},
          }}}};

  assert(actual_directory_tree == expected_directory_tree);

  // Cleanup
  for (const DirectoryTableRow test_directory_result : test_directory_results) {
    delete test_directory_result.parent_id;
  }
}

void testWeGetEmptyDirectoryTreeWithEmptyDirectoryRows() {
  // Arrange
  const DirectoryRowIdMap test_directory_results = {};
  const HashTableRow::Rows test_hash_results = {
      {5, "testing1.txt", Hash{128, 255, 255}},
  };

  // Act
  DirectoryNode actual_directory_tree =
      buildDirectoryTree(test_hash_results, test_directory_results);

  // Assert
  assert(actual_directory_tree == DirectoryNode());
}

void testWeGetEmptyDirectoryTreeWithEmptyHashRows() {
  // Arrange
  const DirectoryTableRow::Rows test_directory_results = {
      {2, "home", new unsigned int(1)},
  };
  const DirectoryRowIdMap test_directory_results_id_map{
      {1, &test_directory_results[0]},
  };
  const HashTableRow::Rows test_hash_results = {};

  // Act
  DirectoryNode actual_directory_tree =
      buildDirectoryTree(test_hash_results, test_directory_results_id_map);

  // Assert
  assert(actual_directory_tree == DirectoryNode());
  for (const DirectoryTableRow test_directory_result : test_directory_results) {
    delete test_directory_result.parent_id;
  }
}

void testBuildingSingleFileNodeBranch() {
  // Arrange
  const DirectoryTableRow::Rows test_directory_results = {
      {1, "/", nullptr},
      {2, "home", new unsigned int(1)},
      {3, "dir1", new unsigned int(2)},
      {4, "sub_dir", new unsigned int(3)}};
  const DirectoryRowIdMap test_directory_results_id_map{
      {1, &test_directory_results[0]},
      {2, &test_directory_results[1]},
      {3, &test_directory_results[2]},
      {4, &test_directory_results[3]},
  };
  const HashTableRow test_hash_result{4, "testing1.txt", Hash{128, 255, 255}};

  // Act
  DirectoryNode actual_directory_tree =
      buildFileNodeBranch(test_hash_result, test_directory_results_id_map);

  // Assert
  DirectoryNode expected_directory_tree{
      "/",
      {},
      {DirectoryNode{
          "home",
          {},
          {
              DirectoryNode{"dir1",
                            {},
                            {DirectoryNode{"sub_dir",
                                           {FileNode{"testing1.txt",
                                                     Hash{128, 255, 255}}}}}},
          }}}};

  assert(expected_directory_tree == actual_directory_tree);

  // Cleanup
  for (const DirectoryTableRow test_directory_result : test_directory_results) {
    delete test_directory_result.parent_id;
  }
}

void testMergingTwoBranches() {
  // Arrange
  DirectoryNode testing_directory_tree_one{
      "/",
      {},
      {DirectoryNode{
          "home",
          {FileNode{"testing1.txt", Hash{128, 255, 255}}},
          {
              DirectoryNode{"dir1",
                            {},
                            {DirectoryNode{"sub_dir",
                                           {FileNode{"testing1.txt",
                                                     Hash{128, 255, 255}}}}}},
          }}}};

  DirectoryNode testing_directory_tree_two{
      "/",
      {},
      {DirectoryNode{
          "home",
          {},
          {
              DirectoryNode{"dir1",
                            {{FileNode{"testing1.txt", Hash{128, 255, 255}}}},
                            {}},
              DirectoryNode{"dir2",
                            {},
                            {DirectoryNode{"sub_dir",
                                           {FileNode{"testing1.txt",
                                                     Hash{128, 255, 255}}}}}},
          }}}};

  // Act
  DirectoryNode actual_directory_tree = mergeTwoDirectoryNodes(
      testing_directory_tree_one, testing_directory_tree_two);

  // Assert
  DirectoryNode expected_directory_tree{
      "/",
      {},
      {DirectoryNode{
          "home",
          {FileNode{"testing1.txt", Hash{128, 255, 255}}},
          {DirectoryNode{"dir1",
                         {FileNode{"testing1.txt", Hash{128, 255, 255}}},
                         {DirectoryNode{
                             "sub_dir",
                             {FileNode{"testing1.txt", Hash{128, 255, 255}}}}}},
           {
               DirectoryNode{"dir2",
                             {},
                             {DirectoryNode{"sub_dir",
                                            {FileNode{"testing1.txt",
                                                      Hash{128, 255, 255}}}}}},
           }}}}};

  assert(expected_directory_tree == actual_directory_tree);
}

void testWeGetEmptyDirectoryTreeWhenMergingTreesWithoutSameRoot() {
  // Arrange
  DirectoryNode testing_directory_tree_one{"testing", {}, {}};

  DirectoryNode testing_directory_tree_two{"/", {}, {}};

  // Act
  DirectoryNode actual_directory_tree = mergeTwoDirectoryNodes(
      testing_directory_tree_one, testing_directory_tree_two);

  // Assert
  DirectoryNode expected_directory_tree{"", {}, {}};

  assert(expected_directory_tree == actual_directory_tree);
}

void testMergingFileLists() {
  // Arrange
  FileNode::Files test_files_one = {
      FileNode{"testing1.txt", Hash{128, 255, 255}},
      FileNode{"testing2.txt", Hash{128, 255, 255}},
  };

  FileNode::Files test_files_two = {
      FileNode{"testing1.txt", Hash{128, 255, 255}},
      FileNode{"testing3.txt", Hash{128, 255, 255}},
  };

  // Act
  FileNode::Files actual_files =
      mergeTwoFileLists(test_files_one, test_files_two);

  // Assert
  FileNode::Files expected_files{
      FileNode{"testing1.txt", Hash{128, 255, 255}},
      FileNode{"testing2.txt", Hash{128, 255, 255}},
      FileNode{"testing3.txt", Hash{128, 255, 255}},
  };

  assert(actual_files == expected_files);
}

void testMergingDirectories() {
  // Arrange
  DirectoryNode::Directories test_directories_one = {
      DirectoryNode{
          "dir1", {FileNode{"testing1.txt", Hash{128, 255, 255}}}, {}},
      DirectoryNode{
          "dir2",
          {},
          {DirectoryNode{"sub_dir",
                         {FileNode{"testing1.txt", Hash{128, 255, 255}}}}}},
  };

  DirectoryNode::Directories test_directories_two = {
      DirectoryNode{
          "dir2",
          {FileNode{"testing1.txt", Hash{128, 255, 255}}},
          {DirectoryNode{"sub_dir",
                         {FileNode{"testing2.txt", Hash{128, 255, 255}}}}}},
      DirectoryNode{
          "dir3",
          {},
          {DirectoryNode{"sub_dir",
                         {FileNode{"testing1.txt", Hash{128, 255, 255}}}}}},
  };

  // Act
  DirectoryNode::Directories actual_directories =
      mergeTwoDirectoryLists(test_directories_one, test_directories_two);

  // Assert
  DirectoryNode::Directories expected_directories = {
      DirectoryNode{
          "dir1", {FileNode{"testing1.txt", Hash{128, 255, 255}}}, {}},
      DirectoryNode{
          "dir2",
          {FileNode{"testing1.txt", Hash{128, 255, 255}}},
          {DirectoryNode{"sub_dir",
                         {FileNode{"testing1.txt", Hash{128, 255, 255}},
                          FileNode{"testing2.txt", Hash{128, 255, 255}}}}}},
      DirectoryNode{
          "dir3",
          {},
          {DirectoryNode{"sub_dir",
                         {FileNode{"testing1.txt", Hash{128, 255, 255}}}}}},
  };

  assert(actual_directories == expected_directories);
}

void testHashNodeEqualOperator() {
  // Arrange
  const HashNode test_hash_node_one = {
      "src",
      Hash{255, 255, 255},
      {HashNode{"testing1.txt", Hash{255, 255, 255}},
       HashNode{
           "app",
           Hash{255, 255, 255},
           {HashNode{"testing1.txt", Hash{255, 255, 255}}},
       }}};

  const HashNode test_hash_node_two = {
      "src",
      Hash{255, 255, 255},
      {HashNode{"testing1.txt", Hash{255, 255, 255}},
       HashNode{
           "app",
           Hash{255, 255, 255},
           {HashNode{"testing1.txt", Hash{255, 255, 255}}},
       }}};

  // Act
  bool equality_test = test_hash_node_one == test_hash_node_two;

  // Assert
  assert(equality_test);
}

void testFileNodeEqualOperator() {
  // Arrange
  const FileNode test_file_node_one{"testing1.txt", Hash{255, 255, 255}};

  const FileNode test_file_node_two{"testing1.txt", Hash{255, 255, 255}};

  // Act
  bool equality_test = test_file_node_one == test_file_node_two;

  // Assert
  assert(equality_test);
}

void testDirectoryNodeEqualOperator() {
  // Arrange
  const DirectoryNode test_directory_node_one{
      "dir2",
      {FileNode{"testing1.txt", Hash{255, 255, 255}}},
      {DirectoryNode{"sub_dir",
                     {FileNode{"testing2.txt", Hash{255, 255, 255}}}}}};

  const DirectoryNode test_directory_node_two{
      "dir2",
      {FileNode{"testing1.txt", Hash{255, 255, 255}}},
      {DirectoryNode{"sub_dir",
                     {FileNode{"testing2.txt", Hash{255, 255, 255}}}}}};

  // Act
  bool equality_result = test_directory_node_one == test_directory_node_two;

  // Assert
  assert(equality_result);
}

void testCalculatingHashFromHashNodes() {
  // Arrange
  HashNode::HashedNodes test_hash_nodes = {
      HashNode{"sub-dir", TEST_HASH}, HashNode{"src", TEST_HASH},
      FileNode{"testing1.txt", TEST_HASH}, FileNode{"testing2.txt", TEST_HASH},
      FileNode{"testing3.txt", TEST_HASH}};

  // Act
  Hash actual_hash = computeHashNodesHash(test_hash_nodes);

  // Assert
  Hash expected_hash = Hash{50, 156, 223, 92, 68,  8,  141, 47,
                            29, 100, 118, 98, 196, 85, 15,  151};
  assert(expected_hash == actual_hash);
}

void testConvertingFileNodesToHashNodes() {
  // Arrange
  FileNode::Files test_files = {FileNode{"testing1.txt", TEST_HASH},
                                FileNode{"testing2.txt", TEST_HASH},
                                FileNode{"testing3.txt", TEST_HASH}};

  // Act
  HashNode::HashedNodes actual_hash_nodes = buildHashNodes(test_files);

  // Assert
  HashNode::HashedNodes expected_hash_nodes = {
      HashNode{"testing1.txt", TEST_HASH}, HashNode{"testing2.txt", TEST_HASH},
      HashNode{"testing3.txt", TEST_HASH}};
  assert(expected_hash_nodes == actual_hash_nodes);
}

void testConvertingLeafDirectoryNodeToHashNode() {
  // Arrange
  DirectoryNode test_directory_node = {
      "src",
      {FileNode{"testing1.txt", TEST_HASH}, FileNode{"testing2.txt", TEST_HASH},
       FileNode{"testing3.txt", TEST_HASH}}};

  // Act
  HashNode actual_directory_hash_node = buildHashNode(test_directory_node);

  // Assert
  HashNode expected_directory_hash_node = HashNode{
      "src",
      Hash{91, 83, 0, 15, 77, 131, 26, 67, 48, 112, 248, 0, 4, 38, 41, 16},
      {HashNode{"testing1.txt", TEST_HASH}, HashNode{"testing2.txt", TEST_HASH},
       HashNode{"testing3.txt", TEST_HASH}}};
  assert(actual_directory_hash_node == expected_directory_hash_node);
}

void testConvertingNestedDirectoryNodeToHashNode() {
  // Arrange
  DirectoryNode test_directory_node = {
      "src",
      {FileNode{"testing1.txt", TEST_HASH}, FileNode{"testing2.txt", TEST_HASH},
       FileNode{"testing3.txt", TEST_HASH}},
      {DirectoryNode{"app", {FileNode{"testing1.txt", TEST_HASH}}},
       DirectoryNode{"test", {FileNode{"testing1.txt", TEST_HASH}}}}};

  // Act
  HashNode actual_directory_hash_node = buildHashNode(test_directory_node);

  // Assert
  HashNode expected_directory_hash_node = HashNode{
      "src",
      Hash{123, 92, 18, 77, 21, 122, 197, 183, 213, 222, 95, 218, 111, 255, 61,
           33},
      {HashNode{"testing1.txt", TEST_HASH}, HashNode{"testing2.txt", TEST_HASH},
       HashNode{"testing3.txt", TEST_HASH},
       HashNode{"app",
                Hash{141, 121, 203, 201, 164, 236, 221, 225, 18, 252, 145, 186,
                     98, 91, 19, 194},
                {HashNode{"testing1.txt", TEST_HASH}}},
       HashNode{"test",
                Hash{141, 121, 203, 201, 164, 236, 221, 225, 18, 252, 145, 186,
                     98, 91, 19, 194},
                {HashNode{"testing1.txt", TEST_HASH}}}},
  };
  assert(actual_directory_hash_node == expected_directory_hash_node);
}

void testHashPathSegmentEqualOperator() {
  // Arrange
  HashPathSegment test_one{"testing", "test"};
  HashPathSegment test_two{"testing", "test"};

  // Act
  bool equality_test = test_one == test_two;

  // Assert
  assert(equality_test == true);
}

void testSearchingTreeForDuplicateNodes() {
  // Arrange
  HashNode test_directory_node = {
      "apple",
      Hash{1},
      {HashNode{
           "orange",
           Hash{2},
           {HashNode{"pineapple", Hash{3}, {HashNode{"one.txt", Hash{4}}}},
            HashNode{"two.txt", Hash{5}}},
       },
       HashNode{"cherry",
                Hash{2},
                {HashNode{"coconut", Hash{3}, {HashNode{"three.txt", Hash{4}}}},
                 HashNode{"four.txt", Hash{5}}}},
       HashNode{
           "banana",
           Hash{6},
           {HashNode{"five.txt", Hash{4}}, HashNode{"six.txt", Hash{5}}}}}};

  // Act
  HashToDuplicateNodes actual_duplicate_nodes =
      buildHashToDuplicateNodesMap(test_directory_node);

  // Assert
  HashToDuplicateNodes expected_duplicate_nodes{
      {{"\x01", {{{"apple", "\x01"}}}},

       {"\x02",
        {{{"apple", "\x01"}, {"orange", "\x02"}},
         {{"apple", "\x01"}, {"cherry", "\x02"}}}},

       {"\x03",
        {{{"apple", "\x01"}, {"orange", "\x02"}, {"pineapple", "\x03"}},
         {{"apple", "\x01"}, {"cherry", "\x02"}, {"coconut", "\x03"}}}},

       {"\x05",
        {{{"apple", "\x01"}, {"orange", "\x02"}, {"two.txt", "\x05"}},
         {{"apple", "\x01"}, {"cherry", "\x02"}, {"four.txt", "\x05"}},
         {{"apple", "\x01"}, {"banana", "\x06"}, {"six.txt", "\x05"}}}},

       {"\x06", {{{"apple", "\x01"}, {"banana", "\x06"}}}},
       {"\x04",
        {{{"apple", "\x01"}, {"banana", "\x06"}, {"five.txt", "\x04"}},
         {{"apple", "\x01"},
          {"orange", "\x02"},
          {"pineapple", "\x03"},
          {"one.txt", "\x04"}},
         {{"apple", "\x01"},
          {"cherry", "\x02"},
          {"coconut", "\x03"},
          {"three.txt", "\x04"}}}}},
      {"\x01", "\x02", "\x06", "\x03", "\x05", "\x04"}};

  assert(expected_duplicate_nodes.order == actual_duplicate_nodes.order);
  compareHashToDuplicateNotes(expected_duplicate_nodes.map,
                              actual_duplicate_nodes.map);
}

void testConvertingHashToString() {
  // Arrange
  Hash hash{200, 255, 255};

  // Act
  std::string actual_string = buildStringFromHash(hash);

  // Assert
  uint8_t testing[3]{200, 255, 255};
  std::string expected_string = std::string((char*)testing);
  assert(actual_string == expected_string);
}

void testFilteringMapReturnsDuplicates() {
  // Arrange
  HashToDuplicateNodes test_duplicate_nodes{
      {{"\x01", {{{"apple", "\x01"}}}},

       {"\x02",
        {{{"apple", "\x01"}, {"orange", "\x02"}},
         {{"apple", "\x01"}, {"cherry", "\x02"}}}},

       {"\x03",
        {{{"apple", "\x01"}, {"orange", "\x02"}, {"pineapple", "\x03"}},
         {{"apple", "\x01"}, {"cherry", "\x02"}, {"coconut", "\x03"}}}},

       {"\x05",
        {{{"apple", "\x01"}, {"orange", "\x02"}, {"two.txt", "\x05"}},
         {{"apple", "\x01"}, {"cherry", "\x02"}, {"four.txt", "\x05"}},
         {{"apple", "\x01"}, {"banana", "\x06"}, {"six.txt", "\x05"}}}},

       {"\x06", {{{"apple", "\x01"}, {"banana", "\x06"}}}},
       {"\x04",
        {{{"apple", "\x01"}, {"banana", "\x06"}, {"five.txt", "\x04"}},
         {{"apple", "\x01"},
          {"orange", "\x02"},
          {"pineapple", "\x03"},
          {"one.txt", "\x04"}},
         {{"apple", "\x01"},
          {"cherry", "\x02"},
          {"coconut", "\x03"},
          {"three.txt", "\x04"}}}}},
      {"\x01", "\x02", "\x06", "\x03", "\x05", "\x04"}};

  // Act
  filterNonDuplicatesFromDupNodesMap(test_duplicate_nodes);

  // Assert
  HashToDuplicateNodes expected_duplicate_nodes{
      {{"\x02",
        {{{"apple", "\x01"}, {"orange", "\x02"}},
         {{"apple", "\x01"}, {"cherry", "\x02"}}}},

       {"\x03",
        {{{"apple", "\x01"}, {"orange", "\x02"}, {"pineapple", "\x03"}},
         {{"apple", "\x01"}, {"cherry", "\x02"}, {"coconut", "\x03"}}}},

       {"\x05",
        {{{"apple", "\x01"}, {"orange", "\x02"}, {"two.txt", "\x05"}},
         {{"apple", "\x01"}, {"cherry", "\x02"}, {"four.txt", "\x05"}},
         {{"apple", "\x01"}, {"banana", "\x06"}, {"six.txt", "\x05"}}}},
       {"\x04",
        {{{"apple", "\x01"}, {"banana", "\x06"}, {"five.txt", "\x04"}},
         {{"apple", "\x01"},
          {"orange", "\x02"},
          {"pineapple", "\x03"},
          {"one.txt", "\x04"}},
         {{"apple", "\x01"},
          {"cherry", "\x02"},
          {"coconut", "\x03"},
          {"three.txt", "\x04"}}}}},
      {"\x02", "\x03", "\x05", "\x04"}};

  assert(test_duplicate_nodes.order == expected_duplicate_nodes.order);
  compareHashToDuplicateNotes(test_duplicate_nodes.map,
                              expected_duplicate_nodes.map);
}

void testHasSharedParent() {
  // Arrange
  DuplicatePaths test_duplicate_paths{
      {{"apple", "\x01"}, {"orange", "\x02"}, {"pineapple", "\x03"}},
      {{"apple", "\x01"}, {"cherry", "\x02"}, {"coconut", "\x03"}}};

  DuplicatePathsMap test_duplicate_nodes_map{
      {"\x02",
       {{{"apple", "\x01"}, {"orange", "\x02"}},
        {{"apple", "\x01"}, {"cherry", "\x02"}}}}};

  // Act
  bool actual_has_shared_parent =
      hasSharedParent(test_duplicate_paths, test_duplicate_nodes_map);

  // Assert
  assert(actual_has_shared_parent == true);
}

void testDoesNotHaveSharedParent() {
  // Arrange
  DuplicatePaths test_duplicate_paths{
      {{"apple", "\x01"}, {"banana", "\x06"}, {"five.txt", "\x04"}},
      {{"apple", "\x01"},
       {"orange", "\x02"},
       {"pineapple", "\x03"},
       {"one.txt", "\x04"}},
      {{"apple", "\x01"},
       {"cherry", "\x02"},
       {"coconut", "\x03"},
       {"three.txt", "\x04"}}};

  DuplicatePathsMap test_duplicate_nodes_map{
      {"\x02",
       {{{"apple", "\x01"}, {"orange", "\x02"}},
        {{"apple", "\x01"}, {"cherry", "\x02"}}}},

      {"\x05",
       {{{"apple", "\x01"}, {"orange", "\x02"}, {"two.txt", "\x05"}},
        {{"apple", "\x01"}, {"cherry", "\x02"}, {"four.txt", "\x05"}},
        {{"apple", "\x01"}, {"banana", "\x06"}, {"six.txt", "\x05"}}}}};

  // Act
  bool actual_has_shared_parent =
      hasSharedParent(test_duplicate_paths, test_duplicate_nodes_map);

  // Assert
  assert(actual_has_shared_parent == false);
}

void testDoesNotHaveSharedParentWhenEmpty() {
  // Arrange
  DuplicatePaths test_duplicate_paths{{}};
  DuplicatePathsMap test_duplicate_nodes_map{};

  // Act
  bool actual_has_shared_parent =
      hasSharedParent(test_duplicate_paths, test_duplicate_nodes_map);

  // Assert
  assert(actual_has_shared_parent == false);
}

void testFilteringSharedHashNodes() {
  // Arrange
  HashToDuplicateNodes test_duplicate_nodes{
      {{"\x02",
        {{{"apple", "\x01"}, {"orange", "\x02"}},
         {{"apple", "\x01"}, {"cherry", "\x02"}}}},

       {"\x03",
        {{{"apple", "\x01"}, {"orange", "\x02"}, {"pineapple", "\x03"}},
         {{"apple", "\x01"}, {"cherry", "\x02"}, {"coconut", "\x03"}}}},

       {"\x05",
        {{{"apple", "\x01"}, {"orange", "\x02"}, {"two.txt", "\x05"}},
         {{"apple", "\x01"}, {"cherry", "\x02"}, {"four.txt", "\x05"}},
         {{"apple", "\x01"}, {"banana", "\x06"}, {"six.txt", "\x05"}}}},
       {"\x04",
        {{{"apple", "\x01"}, {"banana", "\x06"}, {"five.txt", "\x04"}},
         {{"apple", "\x01"},
          {"orange", "\x02"},
          {"pineapple", "\x03"},
          {"one.txt", "\x04"}},
         {{"apple", "\x01"},
          {"cherry", "\x02"},
          {"coconut", "\x03"},
          {"three.txt", "\x04"}}}}},
      {"\x02", "\x03", "\x05", "\x04"}};

  // Act
  filterSharedHashNodes(test_duplicate_nodes);

  // Assert
  HashToDuplicateNodes expected_duplicate_nodes{
      {{"\x02",
        {{{"apple", "\x01"}, {"orange", "\x02"}},
         {{"apple", "\x01"}, {"cherry", "\x02"}}}},
       {"\x05",
        {{{"apple", "\x01"}, {"orange", "\x02"}, {"two.txt", "\x05"}},
         {{"apple", "\x01"}, {"cherry", "\x02"}, {"four.txt", "\x05"}},
         {{"apple", "\x01"}, {"banana", "\x06"}, {"six.txt", "\x05"}}}},
       {"\x04",
        {{{"apple", "\x01"}, {"banana", "\x06"}, {"five.txt", "\x04"}},
         {{"apple", "\x01"},
          {"orange", "\x02"},
          {"pineapple", "\x03"},
          {"one.txt", "\x04"}},
         {{"apple", "\x01"},
          {"cherry", "\x02"},
          {"coconut", "\x03"},
          {"three.txt", "\x04"}}}}},
      {"\x02", "\x05", "\x04"}};

  assert(test_duplicate_nodes.order == expected_duplicate_nodes.order);
  compareHashToDuplicateNotes(test_duplicate_nodes.map,
                              expected_duplicate_nodes.map);
}

void testBuildingSVectorFromPathSegment() {
  // Arrange
  HashPath test_path_segment = {{"apple", "\x01"}, {"orange", "\x02"}};

  // Act
  SVector actual_paths = buildPathsFromHashPath(test_path_segment);

  // Assert
  SVector expected_paths = {"apple", "orange"};
  assert(actual_paths == expected_paths);
}

void testBuildingDuplicateINotesSet() {
  // Arrange
  DuplicatePathsMap test_duplicate_nodes{
      {"\x02",
       {{{"apple", "\x01"}, {"orange", "\x02"}},
        {{"apple", "\x01"}, {"cherry", "\x02"}}}},
      {"\x05",
       {{{"apple", "\x01"}, {"orange", "\x02"}, {"two.txt", "\x05"}},
        {{"apple", "\x01"}, {"cherry", "\x02"}, {"four.txt", "\x05"}},
        {{"apple", "\x01"}, {"banana", "\x06"}, {"six.txt", "\x05"}}}}};

  // Act
  DuplicateINodesSet actual_duplicate_i_nodes_set =
      buildDuplicateINodeSet(test_duplicate_nodes);

  // Assert
  DuplicateINodesSet expected_duplicate_i_node_set = {
      {{"apple", "orange", "two.txt"},
       {"apple", "cherry", "four.txt"},
       {"apple", "banana", "six.txt"}},
      {{"apple", "orange"}, {"apple", "cherry"}},
  };
  assert(actual_duplicate_i_nodes_set == expected_duplicate_i_node_set);
}

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
  bool actual_sort_result = comparePaths(paths_one, paths_two);

  // Assert
  assert(actual_sort_result == true);
}

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

/**
 * apple
 *  |- one.txt {1}
 *  |- banana
 *  |  |- two.txt {1}
 *  |  |- three.txt {5}
 *  |  |- four.txt {5}
 *  |- cherry
 *  |  |- coconut
 *  |  |  | - five.txt {2}
 *  |  |  | - six.txt {3}
 *  |  |- pear
 *  |  |  | - seven.txt {4}
 *  |- orange
 *  |  |- coconut
 *  |  |  |- eight.txt {2}
 *  |  |  |- nine.txt {3}
 *  |  |- pear
 *  |  |  |- ten.txt {4}
 *  |- pineapple
 *  |  |- dragonfruit
 *  |  |  |- eleven.txt {4}
 *  |  |  |- grapefruit
 *  |  |  |  |- twelve.txt {4}
 */
void testTransforming() {
  // Arrange
  const DirectoryTableRow::Rows test_directory_results = {
      {1, "apple", nullptr},
      {2, "banana", new unsigned int(1)},
      {3, "cherry", new unsigned int(1)},
      {4, "orange", new unsigned int(1)},
      {5, "pineapple", new unsigned int(1)},
      {6, "coconut", new unsigned int(3)},
      {7, "pear", new unsigned int(3)},
      {8, "coconut", new unsigned int(4)},
      {9, "pear", new unsigned int(4)},
      {10, "dragonfruit", new unsigned int(5)},
      {11, "grapefruit", new unsigned int(10)}};

  const HashTableRow::Rows test_hash_results = {
      {1, "one.txt", Hash{1}},     {2, "two.txt", Hash{1}},
      {2, "three.txt", Hash{5}},   {2, "four.txt", Hash{5}},
      {6, "five.txt", Hash{2}},    {6, "six.txt", Hash{3}},
      {7, "seven.txt", Hash{4}},   {8, "eight.txt", Hash{2}},
      {8, "nine.txt", Hash{3}},    {9, "ten.txt", Hash{4}},
      {10, "eleven.txt", Hash{4}}, {11, "twelve.txt", Hash{4}}};

  // Act
  DuplicateINodesSet actual_duplicate_i_nodes_set =
      transform(test_directory_results, test_hash_results);

  // Assert
  DuplicateINodesSet expected_duplicate_i_nodes_set = {
      {{"apple", "cherry"}, {"apple", "orange"}},
      {{"apple", "one.txt"}, {"apple", "banana", "two.txt"}},
      {{"apple", "cherry", "pear"},
       {"apple", "orange", "pear"},
       {"apple", "pineapple", "dragonfruit", "grapefruit"}},
      {{"apple", "banana", "four.txt"}, {"apple", "banana", "three.txt"}},
      {{"apple", "cherry", "pear", "seven.txt"},
       {"apple", "orange", "pear", "ten.txt"},
       {"apple", "pineapple", "dragonfruit", "eleven.txt"},
       {"apple", "pineapple", "dragonfruit", "grapefruit", "twelve.txt"}}};

  assert(expected_duplicate_i_nodes_set == actual_duplicate_i_nodes_set);

  // Cleanup
  for (const DirectoryTableRow test_directory_result : test_directory_results) {
    delete test_directory_result.parent_id;
  }
}

int main() {
  testCountingShortestVectorLength();
  testCountingVectorsWithLengthZeroReturnsZero();
  testHashingAListOfHashes();
  testTwoHashesAreDifferent();
  testBuildingDirectoryRowIdMap();
  testDuplicateDirectoryTreeCreation();
  testWeGetEmptyDirectoryTreeWithEmptyDirectoryRows();
  testWeGetEmptyDirectoryTreeWithEmptyHashRows();
  testBuildingSingleFileNodeBranch();
  testMergingTwoBranches();
  testWeGetEmptyDirectoryTreeWhenMergingTreesWithoutSameRoot();
  testMergingFileLists();
  testMergingDirectories();
  testHashNodeEqualOperator();
  testFileNodeEqualOperator();
  testDirectoryNodeEqualOperator();
  testCalculatingHashFromHashNodes();
  testConvertingFileNodesToHashNodes();
  testConvertingLeafDirectoryNodeToHashNode();
  testConvertingNestedDirectoryNodeToHashNode();
  testHashPathSegmentEqualOperator();
  testSearchingTreeForDuplicateNodes();
  testConvertingHashToString();
  testFilteringMapReturnsDuplicates();
  testHasSharedParent();
  testDoesNotHaveSharedParent();
  testDoesNotHaveSharedParentWhenEmpty();
  testFilteringSharedHashNodes();
  testBuildingSVectorFromPathSegment();
  testBuildingDuplicateINotesSet();
  testComparingPathReturnsBefore();
  testComparingSizeWhenBothPathsHaveEqualParts();
  testComparingPathsReturnsBefore();
  testSortingPaths();
  testSortingDuplicateINodesSet();
  testTransforming();
}