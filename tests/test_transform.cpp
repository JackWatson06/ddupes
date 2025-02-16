#include <cassert>
#include <cstdint>

#include "../transform.h"

/* -------------------------------------------------------------------------- */
/*                                    Mocks                                   */
/* -------------------------------------------------------------------------- */

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

Hash uniqueTestHash(uint8_t first_byte = 255) {
  Hash full_hash = {first_byte, 255, 255, 255, 255, 255, 255, 255,
                    255,        255, 255, 255, 255, 255, 255, 255};
  return full_hash;
}

uint8_t* uniqueTestBlob(uint8_t first_byte = 255) {
  return new uint8_t[MD5_DIGEST_LENGTH]{first_byte, 255, 255, 255, 255, 255,
                                        255,        255, 255, 255, 255, 255,
                                        255,        255, 255, 255};
}

/* -------------------------------------------------------------------------- */
/*                                    Tests                                   */
/* -------------------------------------------------------------------------- */

/* ------------------------------- computeHash ------------------------------ */
void testHashingAListOfHashes() {
  // Arrange
  Hashes test_hashes{uniqueTestHash(), uniqueTestHash(), uniqueTestHash()};

  // Act
  Hash actual_hash = computeHash(test_hashes);

  // Assert
  Hash expected_hash{91, 83,  0,   15, 77, 131, 26, 67,
                     48, 112, 248, 0,  4,  38,  41, 16};
  assert(actual_hash == expected_hash);
}

void testTwoHashesAreDifferent() {
  // Arrange
  Hashes test_one_hashes{uniqueTestHash(), uniqueTestHash(), uniqueTestHash()};
  Hashes test_two_hashes{uniqueTestHash(), uniqueTestHash()};

  // Act
  Hash actual_hash_one = computeHash(test_one_hashes);
  Hash actual_hash_two = computeHash(test_two_hashes);

  // Assert
  assert(actual_hash_one != actual_hash_two);
}

/* ------------------------- buildDirectoryRowIdMap ------------------------- */
void testBuildingDirectoryRowIdMap() {
  const DirectoryTableRow::Rows test_directory_results = {{1, "/", -1},
                                                          {2, "home", 1},
                                                          {3, "dir1", 2},
                                                          {4, "dir2", 2},
                                                          {5, "sub_dir", 4}};

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
}

/* --------------------------- buildDirectoryTree --------------------------- */
void testDuplicateDirectoryTreeCreation() {
  // Arrange
  const DirectoryTableRow::Rows test_directory_results = {{1, "/", -1},
                                                          {2, "home", 1},
                                                          {3, "dir1", 2},
                                                          {4, "dir2", 2},
                                                          {5, "sub_dir", 4}};
  const DirectoryRowIdMap test_directory_results_id_map{
      {1, &test_directory_results[0]}, {2, &test_directory_results[1]},
      {3, &test_directory_results[2]}, {4, &test_directory_results[3]},
      {5, &test_directory_results[4]},
  };

  const HashTableRow::Rows test_hash_results = {
      {5, "testing1.txt", uniqueTestBlob(128)},
      {4, "testing2.txt", uniqueTestBlob(196)},
      {4, "testing3.txt", uniqueTestBlob(200)},
      {3, "testing4.txt", uniqueTestBlob(255)}};

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
                                FileNode{"testing3.txt", uniqueTestHash(200)},
                                FileNode{"testing2.txt", uniqueTestHash(196)},
                            },
                            {DirectoryNode{"sub_dir",
                                           {FileNode{"testing1.txt",
                                                     uniqueTestHash(128)}}}}},
              DirectoryNode{
                  "dir1", {FileNode{"testing4.txt", uniqueTestHash(255)}}, {}},
          }}}};

  assert(actual_directory_tree == expected_directory_tree);
}

void testWeGetEmptyDirectoryTreeWithEmptyDirectoryRows() {
  // Arrange
  const DirectoryRowIdMap test_directory_results = {};
  const HashTableRow::Rows test_hash_results = {
      {5, "testing1.txt", uniqueTestBlob(128)},
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
      {2, "home", 1},
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
}

/* ------------------------------- blobToHash ------------------------------- */
void testBlobToHash() {
  // Arrange
  void* test_blob = uniqueTestBlob();

  // Act
  Hash actual_hash = blobToHash(test_blob);

  // Assert
  assert(actual_hash == uniqueTestHash());
}

/* --------------------------- buildFileNodeBranch -------------------------- */
void testBuildingSingleFileNodeBranch() {
  // Arrange
  const DirectoryTableRow::Rows test_directory_results = {
      {1, "/", -1}, {2, "home", 1}, {3, "dir1", 2}, {4, "sub_dir", 3}};
  const DirectoryRowIdMap test_directory_results_id_map{
      {1, &test_directory_results[0]},
      {2, &test_directory_results[1]},
      {3, &test_directory_results[2]},
      {4, &test_directory_results[3]},
  };
  const HashTableRow test_hash_result{4, "testing1.txt", uniqueTestBlob(128)};

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
                                                     uniqueTestHash(128)}}}}},
          }}}};

  assert(expected_directory_tree == actual_directory_tree);
}

/* ------------------------- mergeTwoDirectoryNodes ------------------------- */
void testMergingTwoBranches() {
  // Arrange
  DirectoryNode testing_directory_tree_one{
      "/",
      {},
      {DirectoryNode{
          "home",
          {FileNode{"testing1.txt", uniqueTestHash(128)}},
          {
              DirectoryNode{"dir1",
                            {},
                            {DirectoryNode{"sub_dir",
                                           {FileNode{"testing1.txt",
                                                     uniqueTestHash(128)}}}}},
          }}}};

  DirectoryNode testing_directory_tree_two{
      "/",
      {},
      {DirectoryNode{
          "home",
          {},
          {
              DirectoryNode{"dir1",
                            {{FileNode{"testing1.txt", uniqueTestHash(128)}}},
                            {}},
              DirectoryNode{"dir2",
                            {},
                            {DirectoryNode{"sub_dir",
                                           {FileNode{"testing1.txt",
                                                     uniqueTestHash(128)}}}}},
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
          {FileNode{"testing1.txt", uniqueTestHash(128)}},
          {DirectoryNode{"dir1",
                         {FileNode{"testing1.txt", uniqueTestHash(128)}},
                         {DirectoryNode{
                             "sub_dir",
                             {FileNode{"testing1.txt", uniqueTestHash(128)}}}}},
           {
               DirectoryNode{"dir2",
                             {},
                             {DirectoryNode{"sub_dir",
                                            {FileNode{"testing1.txt",
                                                      uniqueTestHash(128)}}}}},
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

/* ---------------------------- mergeTwoFileLists --------------------------- */
void testMergingFileLists() {
  // Arrange
  FileNode::Files test_files_one = {
      FileNode{"testing1.txt", uniqueTestHash(128)},
      FileNode{"testing2.txt", uniqueTestHash(128)},
  };

  FileNode::Files test_files_two = {
      FileNode{"testing1.txt", uniqueTestHash(128)},
      FileNode{"testing3.txt", uniqueTestHash(128)},
  };

  // Act
  FileNode::Files actual_files =
      mergeTwoFileLists(test_files_one, test_files_two);

  // Assert
  FileNode::Files expected_files{
      FileNode{"testing1.txt", uniqueTestHash(128)},
      FileNode{"testing2.txt", uniqueTestHash(128)},
      FileNode{"testing3.txt", uniqueTestHash(128)},
  };

  assert(actual_files == expected_files);
}

/* ------------------------- mergeTwoDirectoryLists ------------------------- */
void testMergingDirectories() {
  // Arrange
  DirectoryNode::Directories test_directories_one = {
      DirectoryNode{
          "dir1", {FileNode{"testing1.txt", uniqueTestHash(128)}}, {}},
      DirectoryNode{
          "dir2",
          {},
          {DirectoryNode{"sub_dir",
                         {FileNode{"testing1.txt", uniqueTestHash(128)}}}}},
  };

  DirectoryNode::Directories test_directories_two = {
      DirectoryNode{
          "dir2",
          {FileNode{"testing1.txt", uniqueTestHash(128)}},
          {DirectoryNode{"sub_dir",
                         {FileNode{"testing2.txt", uniqueTestHash(128)}}}}},
      DirectoryNode{
          "dir3",
          {},
          {DirectoryNode{"sub_dir",
                         {FileNode{"testing1.txt", uniqueTestHash(128)}}}}},
  };

  // Act
  DirectoryNode::Directories actual_directories =
      mergeTwoDirectoryLists(test_directories_one, test_directories_two);

  // Assert
  DirectoryNode::Directories expected_directories = {
      DirectoryNode{
          "dir1", {FileNode{"testing1.txt", uniqueTestHash(128)}}, {}},
      DirectoryNode{
          "dir2",
          {FileNode{"testing1.txt", uniqueTestHash(128)}},
          {DirectoryNode{"sub_dir",
                         {FileNode{"testing1.txt", uniqueTestHash(128)},
                          FileNode{"testing2.txt", uniqueTestHash(128)}}}}},
      DirectoryNode{
          "dir3",
          {},
          {DirectoryNode{"sub_dir",
                         {FileNode{"testing1.txt", uniqueTestHash(128)}}}}},
  };

  assert(actual_directories == expected_directories);
}

/* -------------------------------- HashNode -------------------------------- */
void testHashNodeEqualOperator() {
  // Arrange
  const HashNode test_hash_node_one = {
      "src",
      uniqueTestHash(255),
      {HashNode{"testing1.txt", uniqueTestHash(255)},
       HashNode{
           "app",
           uniqueTestHash(255),
           {HashNode{"testing1.txt", uniqueTestHash(255)}},
       }}};

  const HashNode test_hash_node_two = {
      "src",
      uniqueTestHash(255),
      {HashNode{"testing1.txt", uniqueTestHash(255)},
       HashNode{
           "app",
           uniqueTestHash(255),
           {HashNode{"testing1.txt", uniqueTestHash(255)}},
       }}};

  // Act
  bool equality_test = test_hash_node_one == test_hash_node_two;

  // Assert
  assert(equality_test);
}

/* -------------------------------- FileNode -------------------------------- */
void testFileNodeEqualOperator() {
  // Arrange
  const FileNode test_file_node_one{"testing1.txt", uniqueTestHash(255)};

  const FileNode test_file_node_two{"testing1.txt", uniqueTestHash(255)};

  // Act
  bool equality_test = test_file_node_one == test_file_node_two;

  // Assert
  assert(equality_test);
}

/* ------------------------------ DirectoryNode ----------------------------- */
void testDirectoryNodeEqualOperator() {
  // Arrange
  const DirectoryNode test_directory_node_one{
      "dir2",
      {FileNode{"testing1.txt", uniqueTestHash(255)}},
      {DirectoryNode{"sub_dir",
                     {FileNode{"testing2.txt", uniqueTestHash(255)}}}}};

  const DirectoryNode test_directory_node_two{
      "dir2",
      {FileNode{"testing1.txt", uniqueTestHash(255)}},
      {DirectoryNode{"sub_dir",
                     {FileNode{"testing2.txt", uniqueTestHash(255)}}}}};

  // Act
  bool equality_result = test_directory_node_one == test_directory_node_two;

  // Assert
  assert(equality_result);
}

/* ---------------------------- computeHashNode ---------------------------- */
void testCalculatingHashFromHashNodes() {
  // Arrange
  HashNode::HashedNodes test_hash_nodes = {
      HashNode{"sub-dir", uniqueTestHash()}, HashNode{"src", uniqueTestHash()},
      FileNode{"testing1.txt", uniqueTestHash()},
      FileNode{"testing2.txt", uniqueTestHash()},
      FileNode{"testing3.txt", uniqueTestHash()}};

  // Act
  Hash actual_hash = computeHashNodesHash(test_hash_nodes);

  // Assert
  Hash expected_hash = Hash{50, 156, 223, 92, 68,  8,  141, 47,
                            29, 100, 118, 98, 196, 85, 15,  151};
  assert(expected_hash == actual_hash);
}

/* ----------------------------- buildHashNodes ----------------------------- */
void testConvertingFileNodesToHashNodes() {
  // Arrange
  FileNode::Files test_files = {FileNode{"testing1.txt", uniqueTestHash()},
                                FileNode{"testing2.txt", uniqueTestHash()},
                                FileNode{"testing3.txt", uniqueTestHash()}};

  // Act
  HashNode::HashedNodes actual_hash_nodes = buildHashNodes(test_files);

  // Assert
  HashNode::HashedNodes expected_hash_nodes = {
      HashNode{"testing1.txt", uniqueTestHash()},
      HashNode{"testing2.txt", uniqueTestHash()},
      HashNode{"testing3.txt", uniqueTestHash()}};
  assert(expected_hash_nodes == actual_hash_nodes);
}

/* ------------------------------ buildHashNode ----------------------------- */
void testConvertingLeafDirectoryNodeToHashNode() {
  // Arrange
  DirectoryNode test_directory_node = {
      "src",
      {FileNode{"testing1.txt", uniqueTestHash()},
       FileNode{"testing2.txt", uniqueTestHash()},
       FileNode{"testing3.txt", uniqueTestHash()}}};

  // Act
  HashNode actual_directory_hash_node = buildHashNode(test_directory_node);

  // Assert
  HashNode expected_directory_hash_node = HashNode{
      "src",
      Hash{91, 83, 0, 15, 77, 131, 26, 67, 48, 112, 248, 0, 4, 38, 41, 16},
      {HashNode{"testing1.txt", uniqueTestHash()},
       HashNode{"testing2.txt", uniqueTestHash()},
       HashNode{"testing3.txt", uniqueTestHash()}}};
  assert(actual_directory_hash_node == expected_directory_hash_node);
}

void testConvertingNestedDirectoryNodeToHashNode() {
  // Arrange
  DirectoryNode test_directory_node = {
      "src",
      {FileNode{"testing1.txt", uniqueTestHash()},
       FileNode{"testing2.txt", uniqueTestHash()},
       FileNode{"testing3.txt", uniqueTestHash()}},
      {DirectoryNode{"app", {FileNode{"testing1.txt", uniqueTestHash()}}},
       DirectoryNode{"test", {FileNode{"testing1.txt", uniqueTestHash()}}}}};

  // Act
  HashNode actual_directory_hash_node = buildHashNode(test_directory_node);

  // Assert
  HashNode expected_directory_hash_node = HashNode{
      "src",
      Hash{123, 92, 18, 77, 21, 122, 197, 183, 213, 222, 95, 218, 111, 255, 61,
           33},
      {HashNode{"testing1.txt", uniqueTestHash()},
       HashNode{"testing2.txt", uniqueTestHash()},
       HashNode{"testing3.txt", uniqueTestHash()},
       HashNode{"app",
                Hash{141, 121, 203, 201, 164, 236, 221, 225, 18, 252, 145, 186,
                     98, 91, 19, 194},
                {HashNode{"testing1.txt", uniqueTestHash()}}},
       HashNode{"test",
                Hash{141, 121, 203, 201, 164, 236, 221, 225, 18, 252, 145, 186,
                     98, 91, 19, 194},
                {HashNode{"testing1.txt", uniqueTestHash()}}}},
  };
  assert(actual_directory_hash_node == expected_directory_hash_node);
}

/* ----------------------------- HashPathSegment ---------------------------- */
void testHashPathSegmentEqualOperator() {
  // Arrange
  HashPathSegment test_one{"testing", "test"};
  HashPathSegment test_two{"testing", "test"};

  // Act
  bool equality_test = test_one == test_two;

  // Assert
  assert(equality_test == true);
}

/* ---------------------- buildHashToDuplicateNodesMap ---------------------- */
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

/* --------------------------- buildStringFromHash -------------------------- */
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

/* ------------------- filterNonDuplicatesFromDupNodesMap ------------------- */
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

/* --------------------------- countShortestVector -------------------------- */
void testCountingDuplicatePathsLength() {
  // Arrange
  DuplicatePaths test_duplicate_paths{
      {{"apple", "\x01"}, {"orange", "\x02"}, {"pineapple", "\x03"}},
      {{"apple", "\x01"},
       {"cherry", "\x02"},
       {"coconut", "\x03"},
       {"testing", "\x03"}}};

  // Act
  int actual_path_count = countShortestDuplicatePath(test_duplicate_paths);

  // Assert
  assert(actual_path_count == 3);
}

void testCountingDuplicatePathsWithLengthZeroReturnsZero() {
  // Arrange
  DuplicatePaths test_duplicate_paths = {};

  // Act
  int actual_path_count = countShortestDuplicatePath(test_duplicate_paths);

  // Assert
  assert(actual_path_count == 0);
}

/* ----------------------------- hasSharedParent ---------------------------- */
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

/* -------------------------- filterSharedHashNodes ------------------------- */
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

/* ------------------------- buildPathsFromHashPath ------------------------- */
void testBuildingSVectorFromPathSegment() {
  // Arrange
  HashPath test_path_segment = {{"apple", "\x01"}, {"orange", "\x02"}};

  // Act
  SVector actual_paths = buildPathsFromHashPath(test_path_segment);

  // Assert
  SVector expected_paths = {"apple", "orange"};
  assert(actual_paths == expected_paths);
}

/* ------------------------- buildDuplicateINodeSet ------------------------- */
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

/* -------------------------------- transform ------------------------------- */
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
      {1, "apple", -1},       {2, "banana", 1},      {3, "cherry", 1},
      {4, "orange", 1},       {5, "pineapple", 1},   {6, "coconut", 3},
      {7, "pear", 3},         {8, "coconut", 4},     {9, "pear", 4},
      {10, "dragonfruit", 5}, {11, "grapefruit", 10}};

  const HashTableRow::Rows test_hash_results = {
      {1, "one.txt", uniqueTestBlob(1)},
      {2, "two.txt", uniqueTestBlob(1)},
      {2, "three.txt", uniqueTestBlob(5)},
      {2, "four.txt", uniqueTestBlob(5)},
      {6, "five.txt", uniqueTestBlob(2)},
      {6, "six.txt", uniqueTestBlob(3)},
      {7, "seven.txt", uniqueTestBlob(4)},
      {8, "eight.txt", uniqueTestBlob(2)},
      {8, "nine.txt", uniqueTestBlob(3)},
      {9, "ten.txt", uniqueTestBlob(4)},
      {10, "eleven.txt", uniqueTestBlob(4)},
      {11, "twelve.txt", uniqueTestBlob(4)}};

  const FileHashRows test_file_hash_rows = {
      .directory_rows = test_directory_results, .hash_rows = test_hash_results};

  // Act
  DuplicateINodesSet actual_duplicate_i_nodes_set =
      transform(test_file_hash_rows);

  // Assert
  DuplicateINodesSet expected_duplicate_i_nodes_set = {
      {{"apple", "banana", "three.txt"}, {"apple", "banana", "four.txt"}},
      {{"apple", "orange", "pear"},
       {"apple", "cherry", "pear"},
       {"apple", "pineapple", "dragonfruit", "grapefruit"}},
      {{"apple", "pineapple", "dragonfruit", "eleven.txt"},
       {"apple", "orange", "pear", "ten.txt"},
       {"apple", "cherry", "pear", "seven.txt"},
       {"apple", "pineapple", "dragonfruit", "grapefruit", "twelve.txt"}},
      {{"apple", "orange"}, {"apple", "cherry"}},
      {{"apple", "one.txt"}, {"apple", "banana", "two.txt"}}};

  assert(expected_duplicate_i_nodes_set == actual_duplicate_i_nodes_set);
}

/* -------------------------------------------------------------------------- */
/*                                    Main                                    */
/* -------------------------------------------------------------------------- */
int main() {
  testHashingAListOfHashes();
  testTwoHashesAreDifferent();
  testBuildingDirectoryRowIdMap();
  testDuplicateDirectoryTreeCreation();
  testWeGetEmptyDirectoryTreeWithEmptyDirectoryRows();
  testWeGetEmptyDirectoryTreeWithEmptyHashRows();
  testBlobToHash();
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
  testCountingDuplicatePathsLength();
  testCountingDuplicatePathsWithLengthZeroReturnsZero();
  testHasSharedParent();
  testDoesNotHaveSharedParent();
  testDoesNotHaveSharedParentWhenEmpty();
  testFilteringSharedHashNodes();
  testBuildingSVectorFromPathSegment();
  testBuildingDuplicateINotesSet();
  testTransforming();
}
