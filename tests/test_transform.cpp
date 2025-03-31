#include <cassert>
#include <cstdint>

#include "../src/transform.h"
#include "./data.cpp"

/* -------------------------------------------------------------------------- */
/*                                    Mocks                                   */
/* -------------------------------------------------------------------------- */
void assertDuplicatePathSegments(duplicate_path_segments &segment_one,
                                 duplicate_path_segments &segment_two) {
  for (int i = 0; i < segment_one.size(); ++i) {
    for (int j = 0; j < segment_one[i].size(); ++j) {
      assert(compareStrings(segment_one[i][j], segment_two[i][j]));
    }
  }
}

void assertDuplicatePathSegmentsSet(duplicate_path_seg_set &segment_set_one,
                                    duplicate_path_seg_set &segment_set_two) {
  for (int i = 0; i < segment_set_one.size(); ++i) {
    assertDuplicatePathSegments(segment_set_one[i], segment_set_two[i]);
  }
}

bool compareHashToInodes(hash_inode_map *one, hash_inode_map *two) {
  hash_inode_map &one_reference = *one;
  hash_inode_map &two_reference = *one;

  for (auto iter = one_reference.begin(); iter != one_reference.end(); ++iter) {
    if (one_reference[iter->first] != two_reference[iter->first]) {
      return false;
    }
  }

  for (auto iter = two_reference.begin(); iter != two_reference.end(); ++iter) {
    if (two_reference[iter->first] != one_reference[iter->first]) {
      return false;
    }
  }
  return true;
}

bool compareParentDirectoryMap(parent_directory_map_const map_one,
                               parent_directory_map_const map_two,
                               int map_one_size, int map_two_size) {
  if (map_one_size != map_two_size) {
    return false;
  }

  for (int i = 0; i < map_one_size; ++i) {
    std::vector<directory_table_row const *> const directory_one_nodes =
        map_one[i];
    std::vector<directory_table_row const *> const directory_two_nodes =
        map_two[i];

    if (directory_one_nodes != directory_two_nodes) {
      return false;
    }
  }

  return true;
}

bool compareParentHashMap(parent_hash_map_const map_one,
                          parent_hash_map_const map_two, int map_one_size,
                          int map_two_size) {
  if (map_one_size != map_two_size) {
    return false;
  }

  for (int i = 0; i < map_one_size; ++i) {
    std::vector<hash_table_row const *> const hash_one_nodes = map_one[i];
    std::vector<hash_table_row const *> const hash_two_nodes = map_two[i];

    if (hash_one_nodes != hash_two_nodes) {
      return false;
    }
  }

  return true;
}

inode *createTestBranch(std::vector<char const *> path) {
  // Create the branch.
  inode cur_node = {static_cast<int>(path.size()),
                    path[path.size() - 1],
                    uniqueTestHash(),
                    {}};
  path.pop_back();

  while (path.size() != 0) {
    inode next_node = {static_cast<int>(path.size()),
                       path[path.size() - 1],
                       uniqueTestHash(),
                       {cur_node}};
    path.pop_back();
    cur_node = next_node;
  }

  // Link the parent pointer.
  inode *node_to_return = new inode{cur_node};
  while (node_to_return->inodes.size() != 0) {
    inode const *parent_pointer = node_to_return;

    node_to_return = &node_to_return->inodes[0];
    node_to_return->parent_node = parent_pointer;
  }

  return node_to_return;
}

/* -------------------------------------------------------------------------- */
/*                                    Tests                                   */
/* -------------------------------------------------------------------------- */

/* ------------------------- buildParentDirectoryMap ------------------------ */
void testBuildingParentDirectoryMap() {
  // Arrange
  directory_table_row::rows test_directory_rows{{1, "/", -1},
                                                {2, "home", 1},
                                                {3, "dir1", 2},
                                                {4, "dir2", 2},
                                                {5, "sub_dir", 4}};

  // Act
  parent_directory_map actual_directory_map_results =
      buildParentDirectoryMap(test_directory_rows);

  parent_directory_map expected_directory_map_results =
      new std::vector<directory_table_row const *>[6]{
          {},
          {&test_directory_rows[1]},
          {&test_directory_rows[2], &test_directory_rows[3]},
          {},
          {&test_directory_rows[4]},
          {}};
  assert(compareParentDirectoryMap(expected_directory_map_results,
                                   actual_directory_map_results, 6, 6));
}

void testBuildingParentDirectoryMapWithParentIdOverflow() {
  // Arrange
  directory_table_row::rows test_directory_rows{{1, "/", -1},
                                                {2, "home", 1},
                                                {3, "dir1", 2},
                                                {4, "dir2", 2},
                                                {5, "sub_dir", 10}};

  // Act
  parent_directory_map actual_directory_map_results =
      buildParentDirectoryMap(test_directory_rows);

  parent_directory_map expected_directory_map_results =
      new std::vector<directory_table_row const *>[6]{
          {},
          {&test_directory_rows[1]},
          {&test_directory_rows[2], &test_directory_rows[3]},
          {},
          {},
          {}};
  assert(compareParentDirectoryMap(expected_directory_map_results,
                                   actual_directory_map_results, 6, 6));
}

void testBuildingParentDirectoryMapWithParentIdEqualToSize() {
  // Arrange
  directory_table_row::rows test_directory_rows{
      {1, "/", -1},   {2, "home", 1},    {3, "dir1", 2},
      {4, "dir2", 2}, {5, "sub_dir", 6}, {6, "sub_dir", 4}};

  // Act
  parent_directory_map actual_directory_map_results =
      buildParentDirectoryMap(test_directory_rows);

  parent_directory_map expected_directory_map_results =
      new std::vector<directory_table_row const *>[7]{
          {},
          {&test_directory_rows[1]},
          {&test_directory_rows[2], &test_directory_rows[3]},
          {},
          {&test_directory_rows[5]},
          {},
          {&test_directory_rows[4]}};
  assert(compareParentDirectoryMap(expected_directory_map_results,
                                   actual_directory_map_results, 6, 6));
}

/* --------------------------- buildParentHashMap --------------------------- */
void testBuildingParentHashMap() {
  // Arrange
  hash_table_row::rows test_hash_rows{
      {1, "example.txt", uniqueTestHash()},
      {1, "example_two.txt", uniqueTestHash()},
      {2, "example_three.txt", uniqueTestHash()},
      {3, "example_four.txt", uniqueTestHash()},
      {4, "example_five.txt", uniqueTestHash()}};

  // Act
  parent_hash_map actual_hash_map_results =
      buildParentHashMap(test_hash_rows, 5);

  parent_hash_map expected_hash_map_result =
      new std::vector<hash_table_row const *>[6]{
          {},
          {&test_hash_rows[0], &test_hash_rows[1]},
          {&test_hash_rows[2]},
          {&test_hash_rows[3]},
          {&test_hash_rows[4]},
          {}};
  assert(compareParentHashMap(expected_hash_map_result, actual_hash_map_results,
                              6, 6));
}

void testBuildingParentHashMapWithDirectoryIdOverflow() {
  // Arrange
  hash_table_row::rows test_hash_rows{
      {1, "example.txt", uniqueTestHash()},
      {1, "example_two.txt", uniqueTestHash()},
      {2, "example_three.txt", uniqueTestHash()},
      {3, "example_four.txt", uniqueTestHash()},
      {11, "example_five.txt", uniqueTestHash()}};

  // Act
  parent_hash_map actual_hash_map_results =
      buildParentHashMap(test_hash_rows, 5);

  parent_hash_map expected_hash_map_result =
      new std::vector<hash_table_row const *>[6]{
          {},
          {&test_hash_rows[0], &test_hash_rows[1]},
          {&test_hash_rows[2]},
          {&test_hash_rows[3]},
          {},
          {}};
  assert(compareParentHashMap(expected_hash_map_result, actual_hash_map_results,
                              6, 6));
}
void testBuildingParentHashMapWithDirectoryIdEqualToSize() {
  // Arrange
  hash_table_row::rows test_hash_rows{
      {1, "example.txt", uniqueTestHash()},
      {1, "example_two.txt", uniqueTestHash()},
      {2, "example_three.txt", uniqueTestHash()},
      {3, "example_four.txt", uniqueTestHash()},
      {5, "example_five.txt", uniqueTestHash()}};

  // Act
  parent_hash_map actual_hash_map_results =
      buildParentHashMap(test_hash_rows, 5);

  parent_hash_map expected_hash_map_result =
      new std::vector<hash_table_row const *>[6]{
          {},
          {&test_hash_rows[0], &test_hash_rows[1]},
          {&test_hash_rows[2]},
          {&test_hash_rows[3]},
          {},
          {&test_hash_rows[4]}};
  assert(compareParentHashMap(expected_hash_map_result, actual_hash_map_results,
                              6, 6));
}

/* ----------------------------- buildINodeTree ----------------------------- */

void testBuildINodeTree() {
  // Arrange
  directory_table_row::rows test_directory_rows{{1, "/", -1},
                                                {2, "home", 1},
                                                {3, "dir1", 2},
                                                {4, "dir2", 2},
                                                {5, "sub_dir", 4}};

  hash_table_row::rows test_hash_rows = {
      {5, "testing1.txt", uniqueTestHash(128)},
      {4, "testing2.txt", uniqueTestHash(196)},
      {4, "testing3.txt", uniqueTestHash(200)},
      {3, "testing4.txt", uniqueTestHash(255)}};

  parent_directory_map test_directory_maps =
      new std::vector<directory_table_row const *>[6]{
          {},
          {&test_directory_rows[1]},
          {&test_directory_rows[2], &test_directory_rows[3]},
          {},
          {&test_directory_rows[4]},
          {}};

  parent_hash_map test_hash_maps = new std::vector<hash_table_row const *>[6]{
      {},
      {},
      {},
      {&test_hash_rows[3]},
      {&test_hash_rows[1], &test_hash_rows[2]},
      {&test_hash_rows[0]}};

  // Act
  inode actual_directory_tree = buildINodeTree(
      test_directory_maps, test_hash_maps, &test_directory_rows[0]);

  // Assert
  std::vector<inode> dir2_inode;

  inode expected_directory_tree{
      1,
      "/",
      nullptr,
      {inode{
          2,
          "home",
          nullptr,
          {
              inode{3,
                    "dir1",
                    nullptr,
                    {inode{4, "testing4.txt", uniqueTestHash(255)}}},
              inode{3,
                    "dir2",
                    nullptr,
                    {
                        inode{4, "testing2.txt", uniqueTestHash(196)},
                        inode{4, "testing3.txt", uniqueTestHash(200)},
                        inode{4,
                              "sub_dir",
                              nullptr,
                              {inode{5, "testing1.txt", uniqueTestHash(128)}}},
                    }},
          }}}};

  assert(actual_directory_tree == expected_directory_tree);
}

/* ----------------------------- calculateHashes ---------------------------- */
void testCalculateHashes() {
  // Arrange
  hash testing1_hash = uniqueTestHash(128);
  hash testing2_hash = uniqueTestHash(255);
  hash testing3_hash = uniqueTestHash(255);
  hash testing4_hash = uniqueTestHash(255);

  inode test_inode_tree = {
      1,
      "/",
      nullptr,
      {inode{2,
             "home",
             nullptr,
             {
                 inode{3,
                       "dir1",
                       nullptr,
                       {inode{4, "testing4.txt", testing4_hash}}},
                 inode{3,
                       "dir2",
                       nullptr,
                       {
                           inode{4, "testing2.txt", testing2_hash},
                           inode{4, "testing3.txt", testing3_hash},
                           inode{4,
                                 "sub_dir",
                                 nullptr,
                                 {inode{5, "testing1.txt", testing1_hash}}},
                       }},
             }}}};

  // Act
  hash_inode_map *actual_hash_inode_map = calculateHashes(test_inode_tree);

  // Assert
  inode *root = &test_inode_tree;
  inode *home = &root->inodes[0];
  inode *dir1 = &home->inodes[0];
  inode *dir2 = &home->inodes[1];
  inode *sub_dir = &dir2->inodes[2];
  inode *testing1 = &sub_dir->inodes[0];
  inode *testing2 = &dir2->inodes[0];
  inode *testing3 = &dir2->inodes[1];
  inode *testing4 = &dir1->inodes[0];

  hash root_hash = new uint8_t[MD5_DIGEST_LENGTH]{
      139, 187, 250, 74, 123, 89, 161, 14, 121, 7, 151, 95, 141, 249, 141, 0};
  hash home_hash = new uint8_t[MD5_DIGEST_LENGTH]{
      82, 85, 237, 207, 252, 9, 108, 132, 58, 114, 118, 2, 226, 6, 116, 158};
  hash dir1_hash =
      new uint8_t[MD5_DIGEST_LENGTH]{141, 121, 203, 201, 164, 236, 221, 225,
                                     18,  252, 145, 186, 98,  91,  19,  194};
  hash dir2_hash =
      new uint8_t[MD5_DIGEST_LENGTH]{101, 127, 58, 62,  169, 162, 34, 100,
                                     174, 254, 42, 142, 217, 195, 33, 252};
  hash sub_dir_hash = new uint8_t[MD5_DIGEST_LENGTH]{
      87, 173, 90, 229, 249, 51, 197, 41, 238, 239, 72, 163, 217, 145, 137, 37};

  inode expected_transformed_inode_tree = {
      1,
      "/",
      root_hash,
      {inode{
          2,
          "home",
          home_hash,
          {
              inode{3,
                    "dir1",
                    dir1_hash,
                    {inode{4, "testing4.txt", testing4_hash, {}, dir1}},
                    home},
              inode{
                  3,
                  "dir2",
                  dir2_hash,
                  {
                      inode{4, "testing2.txt", testing2_hash, {}, dir2},
                      inode{4, "testing3.txt", testing3_hash, {}, dir2},
                      inode{4,
                            "sub_dir",
                            sub_dir_hash,
                            {inode{
                                5, "testing1.txt", testing1_hash, {}, sub_dir}},
                            dir2},
                  },
                  home},
          },
          root}},
      nullptr};

  hash_inode_map *expected_hash_inode_map =
      new hash_inode_map{{dir1_hash, {dir1}},
                         {testing2_hash, {testing2, testing3, testing4}},
                         {sub_dir_hash, {sub_dir}},
                         {testing1_hash, {testing1}},
                         {dir2_hash, {dir2}},
                         {home_hash, {home}},
                         {root_hash, {root}}};

  assert(expected_transformed_inode_tree == test_inode_tree);
  assert(compareHashToInodes(expected_hash_inode_map, actual_hash_inode_map));
}

void testCalculateHashesErrorWhenLeafNodeDoesNotHaveHash() {
  // Arrange
  inode test_inode_tree = {1, "/", nullptr, {}};

  // Act
  hash_inode_map *actual_hash_inode_map = calculateHashes(test_inode_tree);

  // Assert
  hash empty_hash = new uint8_t[MD5_DIGEST_LENGTH]{};
  inode expected_transformed_inode_tree = {1, "/", empty_hash, {}, nullptr};
  hash_inode_map *expected_hash_inode_map =
      new hash_inode_map{{empty_hash, {&test_inode_tree}}};

  assert(expected_transformed_inode_tree == test_inode_tree);
  assert(compareHashToInodes(expected_hash_inode_map, actual_hash_inode_map));
}

/* --------------------------- countShortestDepth --------------------------- */
void testCountShortestDepth() {
  // Arrange
  inode *test_path_one =
      createTestBranch({"apple", "orange", "pineapple"}); // Returns tail.
  inode *test_path_two =
      createTestBranch({"apple", "cherry", "cocounut", "testing"});

  // Act
  int actual_path_count = countShortestDepth({test_path_one, test_path_two});

  // Assert
  assert(actual_path_count == 3);
}

void testCountShortestDepthWithLengthZeroReturnsZero() {
  // Arrange
  std::vector<inode const *> test_duplicate_inodes = {};

  // Act
  int actual_path_count = countShortestDepth(test_duplicate_inodes);

  // Assert
  assert(actual_path_count == 0);
}

/* ----------------------- fastForwardINodeReferences ----------------------- */

void testFastForwardingINodeReferencesToShallowestDepth() {
  // Arrange
  inode *test_path_one =
      createTestBranch({"apple", "orange", "pineapple"}); // Returns tail.
  inode *test_path_two =
      createTestBranch({"apple", "cherry", "cocounut", "testing"});
  inode *test_path_three = createTestBranch({"apple", "cherry"});

  // Act
  std::vector<inode const *> actual_fast_forwarded_refs =
      fastForwardINodeReferences(
          {test_path_one, test_path_two, test_path_three});

  // Assert
  std::vector<inode const *> expected_fast_forwarded_refs{
      test_path_one->parent_node->parent_node,
      test_path_two->parent_node->parent_node->parent_node,
      test_path_three->parent_node};

  assert(actual_fast_forwarded_refs == expected_fast_forwarded_refs);
}

void testFastForwardingINodeReferencesToRoot() {
  // Arrange
  inode *test_path_one = createTestBranch({"apple"}); // Returns tail.
  inode *test_path_two =
      createTestBranch({"apple", "cherry", "cocounut", "testing"});
  inode *test_path_three = createTestBranch({"apple", "cherry"});

  // Act
  std::vector<inode const *> actual_fast_forwarded_refs =
      fastForwardINodeReferences(
          {test_path_one, test_path_two, test_path_three});

  // Assert
  std::vector<inode const *> expected_fast_forwarded_refs{nullptr, nullptr,
                                                          nullptr};

  assert(actual_fast_forwarded_refs == expected_fast_forwarded_refs);
}

void testFastForwardingINodeReferencesWithZeroDepth() {
  // Act
  std::vector<inode const *> actual_fast_forwarded_refs =
      fastForwardINodeReferences({});

  // Assert
  std::vector<inode const *> expected_fast_forwarded_refs{};
  assert(actual_fast_forwarded_refs == expected_fast_forwarded_refs);
}

/* ---------------------- filterNonDupsAndNestedHashes ---------------------- */
void testFilteringNonDuplicatesAndNestedHashes() {
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

  /* *Arrange* */
  // Create Inode Tree.
  hash one_hash = uniqueTestHash(1);
  hash two_hash = uniqueTestHash(1);
  hash three_hash = uniqueTestHash(5);
  hash four_hash = uniqueTestHash(5);
  hash five_hash = uniqueTestHash(2);
  hash six_hash = uniqueTestHash(3);
  hash seven_hash = uniqueTestHash(4);
  hash eight_hash = uniqueTestHash(2);
  hash nine_hash = uniqueTestHash(3);
  hash ten_hash = uniqueTestHash(4);
  hash eleven_hash = uniqueTestHash(4);
  hash twelve_hash = uniqueTestHash(4);

  hash apple_hash = new uint8_t[MD5_DIGEST_LENGTH]{
      10, 112, 89, 243, 242, 28, 192, 154, 125, 20, 255, 3, 116, 155, 29, 63};
  hash banana_hash = new uint8_t[MD5_DIGEST_LENGTH]{
      21, 103, 17, 55, 215, 245, 250, 8, 180, 101, 65, 34, 0, 74, 83, 207};
  hash cherry_hash = new uint8_t[MD5_DIGEST_LENGTH]{
      213, 94, 197, 224, 217, 186, 123, 63, 63, 225, 242, 197, 2, 203, 182, 31};
  hash c_cocounut_hash =
      new uint8_t[MD5_DIGEST_LENGTH]{211, 102, 18,  105, 210, 207, 45,  254,
                                     155, 243, 217, 116, 128, 9,   213, 160};
  hash c_pear_hash =
      new uint8_t[MD5_DIGEST_LENGTH]{20,  75, 166, 96,  112, 230, 220, 200,
                                     172, 35, 249, 245, 255, 131, 78,  138};
  hash orange_hash = new uint8_t[MD5_DIGEST_LENGTH]{
      213, 94, 197, 224, 217, 186, 123, 63, 63, 225, 242, 197, 2, 203, 182, 31};
  hash o_cocounut_hash =
      new uint8_t[MD5_DIGEST_LENGTH]{211, 102, 18,  105, 210, 207, 45,  254,
                                     155, 243, 217, 116, 128, 9,   213, 160};
  hash o_pear_hash =
      new uint8_t[MD5_DIGEST_LENGTH]{20,  75, 166, 96,  112, 230, 220, 200,
                                     172, 35, 249, 245, 255, 131, 78,  138};
  hash pineapple_hash = new uint8_t[MD5_DIGEST_LENGTH]{
      184, 59, 189, 15, 41, 100, 155, 8, 143, 48, 98, 241, 166, 52, 125, 220};
  hash dragonfruit_hash = new uint8_t[MD5_DIGEST_LENGTH]{
      128, 198, 29, 30, 68, 74, 125, 69, 86, 148, 229, 167, 3, 33, 59, 170};
  hash grapefruit_hash =
      new uint8_t[MD5_DIGEST_LENGTH]{20,  75, 166, 96,  112, 230, 220, 200,
                                     172, 35, 249, 245, 255, 131, 78,  138};

  inode one{2, "one.txt", one_hash, {}};
  inode two{3, "two.txt", two_hash, {}};
  inode three{3, "three.txt", three_hash, {}};
  inode four{3, "four.txt", four_hash, {}};
  inode five{4, "five.txt", five_hash, {}};
  inode six{4, "six.txt", six_hash, {}};
  inode seven{4, "seven.txt", seven_hash, {}};
  inode eight{4, "eight.txt", eight_hash, {}};
  inode nine{4, "nine.txt", nine_hash, {}};
  inode ten{4, "ten.txt", ten_hash, {}};
  inode eleven{4, "eleven.txt", eleven_hash, {}};
  inode twelve{5, "twelve.txt", twelve_hash, {}};

  inode banana{2, "banana", banana_hash, {two, three, four}};
  inode cocounut{3, "coconut", c_cocounut_hash, {five, six}};
  inode pear{3, "pear", c_pear_hash, {seven}};
  inode cherry{2, "cherry", cherry_hash, {cocounut, pear}};
  cocounut = {3, "cocounut", o_cocounut_hash, {eight, nine}};
  pear = {3, "pear", o_pear_hash, {ten}};
  inode orange{2, "orange", orange_hash, {cocounut, pear}};
  inode grapefruit{4, "grapefruit", grapefruit_hash, {twelve}};
  inode dragonfruit{3, "dragonfruit", dragonfruit_hash, {eleven, grapefruit}};
  inode pineapple{2, "pineapple", pineapple_hash, {dragonfruit}};
  inode apple{1, "apple", apple_hash, {one, banana, cherry, orange, pineapple}};

  // Assign Parent Pointer.
  inode *apple_r = &apple;
  inode *one_r = &apple_r->inodes[0];
  inode *banana_r = &apple.inodes[1];
  inode *two_r = &banana_r->inodes[0];
  inode *three_r = &banana_r->inodes[1];
  inode *four_r = &banana_r->inodes[2];
  inode *cherry_r = &apple.inodes[2];
  inode *c_cocounut_r = &cherry_r->inodes[0];
  inode *five_r = &c_cocounut_r->inodes[0];
  inode *six_r = &c_cocounut_r->inodes[1];
  inode *c_pear_r = &cherry_r->inodes[1];
  inode *seven_r = &c_pear_r->inodes[0];
  inode *orange_r = &apple.inodes[3];
  inode *o_cocounut_r = &orange_r->inodes[0];
  inode *eight_r = &o_cocounut_r->inodes[0];
  inode *nine_r = &o_cocounut_r->inodes[1];
  inode *o_pear_r = &orange_r->inodes[1];
  inode *ten_r = &o_pear_r->inodes[0];
  inode *pineapple_r = &apple.inodes[4];
  inode *dragonfruit_r = &pineapple_r->inodes[0];
  inode *eleven_r = &dragonfruit_r->inodes[0];
  inode *grapefruit_r = &dragonfruit_r->inodes[1];
  inode *twelve_r = &grapefruit_r->inodes[0];

  one_r->parent_node = apple_r;
  banana_r->parent_node = apple_r;
  two_r->parent_node = banana_r;
  three_r->parent_node = banana_r;
  four_r->parent_node = banana_r;
  cherry_r->parent_node = apple_r;
  c_cocounut_r->parent_node = cherry_r;
  five_r->parent_node = c_cocounut_r;
  six_r->parent_node = c_cocounut_r;
  c_pear_r->parent_node = cherry_r;
  seven_r->parent_node = c_pear_r;
  orange_r->parent_node = apple_r;
  o_cocounut_r->parent_node = orange_r;
  eight_r->parent_node = o_cocounut_r;
  nine_r->parent_node = o_cocounut_r;
  o_pear_r->parent_node = orange_r;
  ten_r->parent_node = o_pear_r;
  pineapple_r->parent_node = apple_r;
  dragonfruit_r->parent_node = pineapple_r;
  eleven_r->parent_node = dragonfruit_r;
  grapefruit_r->parent_node = dragonfruit_r;
  twelve_r->parent_node = grapefruit_r;

  // Setup the Hash Map.
  hash_inode_map *test_hash_inode_map = new hash_inode_map{
      {one_hash, {one_r, two_r}},
      {three_hash, {three_r, four_r}},
      {five_hash, {five_r, eight_r}},
      {six_hash, {six_r, nine_r}},
      {seven_hash, {seven_r, ten_r, eleven_r, twelve_r}},
      {banana_hash, {banana_r}},
      {c_cocounut_hash, {c_cocounut_r, o_cocounut_r}},
      {c_pear_hash, {c_pear_r, o_pear_r, grapefruit_r}},
      {cherry_hash, {cherry_r, orange_r}},
      {pineapple_hash, {pineapple_r}},
      {dragonfruit_hash, {dragonfruit_r}},
      {dragonfruit_hash, {apple_r}},
  };

  /* *Act* */
  duplicate_path_seg_set actual_duplicate_inodes =
      filterNonDupsAndNestedHashes(apple, test_hash_inode_map);

  /* *Assert* */
  duplicate_path_seg_set expected_duplicate_inodes = {
      {{"apple", "one.txt"}, {"apple", "banana", "two.txt"}},
      {{"apple", "banana", "three.txt"}, {"apple", "banana", "four.txt"}},
      {{"apple", "cherry", "pear", "seven.txt"},
       {"apple", "orange", "pear", "ten.txt"},
       {"apple", "pineapple", "dragonfruit", "eleven.txt"},
       {"apple", "pineapple", "dragonfruit", "grapefruit", "twelve.txt"}},
      {{"apple", "cherry", "pear"},
       {"apple", "orange", "pear"},
       {"apple", "pineapple", "dragonfruit", "grapefruit"}},
      {{"apple", "cherry"}, {"apple", "orange"}},
  };

  assertDuplicatePathSegmentsSet(actual_duplicate_inodes,
                                 expected_duplicate_inodes);
}

/* ----------------------- buildDuplicatePathSegments ----------------------- */
void testBuildDuplicatePathSegments() {
  // Arrange
  inode *test_path_one =
      createTestBranch({"apple", "cherry", "cocounut", "testing"});
  inode *test_path_two = createTestBranch({"apple"});
  inode *test_path_three = createTestBranch({"apple", "cherry", "cocounut"});

  // Act
  duplicate_path_segments actual_path_segments = buildDuplicatePathSegments(
      {test_path_one, test_path_two, test_path_three});

  // Assert
  duplicate_path_segments expected_path_segments{
      {"apple", "cherry", "cocounut", "testing"},
      {"apple"},
      {"apple", "cherry", "cocounut"}};
  assertDuplicatePathSegments(actual_path_segments, expected_path_segments);
}

/* -------------------------------- inode_== -------------------------------- */
void testINodeEquality() {
  // Arrange
  inode test_inode_one{
      1,
      "/",
      nullptr,
      {inode{2,
             "home",
             nullptr,
             {inode{3,
                    "dir2",
                    nullptr,
                    {
                        inode{4, "testing3.txt", uniqueTestHash(200)},
                        inode{4, "testing2.txt", uniqueTestHash(196)},
                        inode{4,
                              "sub_dir",
                              nullptr,
                              {inode{5, "testing1.txt", uniqueTestHash(128)}}},
                    }},
              inode{3,
                    "dir1",
                    nullptr,
                    {inode{4, "testing4.txt", uniqueTestHash(255)}}}}}}};

  inode test_inode_two{
      1,
      "/",
      nullptr,
      {inode{2,
             "home",
             nullptr,
             {inode{3,
                    "dir2",
                    nullptr,
                    {
                        inode{4, "testing3.txt", uniqueTestHash(200)},
                        inode{4, "testing2.txt", uniqueTestHash(196)},
                        inode{4,
                              "sub_dir",
                              nullptr,
                              {inode{5, "testing1.txt", uniqueTestHash(128)}}},
                    }},
              inode{3,
                    "dir1",
                    nullptr,
                    {inode{4, "testing4.txt", uniqueTestHash(255)}}}}}}};

  // Act
  bool equality_check = test_inode_one == test_inode_two;

  // Assert
  assert(equality_check);
}

void testINodeEqualityNotEqual() {
  // Arrange
  inode test_inode_one{
      1,
      "/",
      nullptr,
      {inode{2,
             "home",
             nullptr,
             {inode{3,
                    "dir2",
                    nullptr,
                    {
                        inode{4, "testing3.txt", uniqueTestHash(200)},
                        inode{4, "testing2.txt", uniqueTestHash(196)},
                        inode{4,
                              "sub_dir",
                              nullptr,
                              {inode{5, "testing1.txt", uniqueTestHash(128)}}},
                    }},
              inode{3,
                    "dir1",
                    nullptr,
                    {inode{4, "testing4.txt", uniqueTestHash(255)}}}}}}};

  inode test_inode_two{
      1,
      "/",
      nullptr,
      {inode{2,
             "home",
             nullptr,
             {inode{3,
                    "dir2",
                    nullptr,
                    {
                        inode{4, "testing3.txt", uniqueTestHash(200)},
                        inode{4, "testing2.txt", uniqueTestHash(196)},
                        inode{4,
                              "sub_dir",
                              nullptr,
                              {inode{5, "testing1.txt", uniqueTestHash(128)}}},
                    }},
              inode{3,
                    "dir1",
                    nullptr,
                    {inode{4, "testing4.txt", uniqueTestHash(254)}}}}}}};

  // Act
  bool equality_check = test_inode_one == test_inode_two;

  // Assert
  assert(equality_check == false);
}

/* ----------------------------- inode_hasher_() ---------------------------- */
void testINodeHashes() {
  // Arrange
  inode_hasher inode_hashing_function{};

  // Act
  std::size_t actual_hash = inode_hashing_function(uniqueTestHash(200));

  // Assert
  std::size_t expected_hash = 0xC8FFFFFFFFFFFFFF;
  assert(actual_hash == expected_hash);
}

/* ----------------------------- inode_key_equal_() ----------------------------
 */
void testINodeHashesEqual() {
  // Arrange
  inode_key_equal inode_key_equal_function{};

  // Act
  bool equality_check =
      inode_key_equal_function(uniqueTestHash(200), uniqueTestHash(200));

  // Assert
  assert(equality_check == true);
}

void testINodeHashesNotEqual() {
  // Arrange
  inode_key_equal inode_key_equal_function{};

  // Act
  bool equality_check =
      inode_key_equal_function(uniqueTestHash(200), uniqueTestHash(212));

  // Assert
  assert(equality_check == false);
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
  directory_table_row::rows const test_directory_results = {
      {1, "apple", -1},       {2, "banana", 1},      {3, "cherry", 1},
      {4, "orange", 1},       {5, "pineapple", 1},   {6, "coconut", 3},
      {7, "pear", 3},         {8, "coconut", 4},     {9, "pear", 4},
      {10, "dragonfruit", 5}, {11, "grapefruit", 10}};

  hash_table_row::rows const test_hash_results = {
      {1, "one.txt", uniqueTestHash(1)},
      {2, "two.txt", uniqueTestHash(1)},
      {2, "three.txt", uniqueTestHash(5)},
      {2, "four.txt", uniqueTestHash(5)},
      {6, "five.txt", uniqueTestHash(2)},
      {6, "six.txt", uniqueTestHash(3)},
      {7, "seven.txt", uniqueTestHash(4)},
      {8, "eight.txt", uniqueTestHash(2)},
      {8, "nine.txt", uniqueTestHash(3)},
      {9, "ten.txt", uniqueTestHash(4)},
      {10, "eleven.txt", uniqueTestHash(4)},
      {11, "twelve.txt", uniqueTestHash(4)}};

  const file_hash_rows test_file_hash_rows = {
      .directory_rows = test_directory_results, .hash_rows = test_hash_results};

  // Act
  duplicate_path_seg_set actual_duplicate_inodes_set =
      transform(test_file_hash_rows);

  // Assert
  duplicate_path_seg_set expected_duplicate_inodes_set = {
      {{"apple", "one.txt"}, {"apple", "banana", "two.txt"}},
      {{"apple", "banana", "three.txt"}, {"apple", "banana", "four.txt"}},
      {{"apple", "cherry", "pear", "seven.txt"},
       {"apple", "orange", "pear", "ten.txt"},
       {"apple", "pineapple", "dragonfruit", "eleven.txt"},
       {"apple", "pineapple", "dragonfruit", "grapefruit", "twelve.txt"}},
      {{"apple", "cherry", "pear"},
       {"apple", "orange", "pear"},
       {"apple", "pineapple", "dragonfruit", "grapefruit"}},
      {{"apple", "cherry"}, {"apple", "orange"}},
  };

  assertDuplicatePathSegmentsSet(actual_duplicate_inodes_set,
                                 expected_duplicate_inodes_set);
}

/* -------------------------------------------------------------------------- */
/*                                    Main                                    */
/* -------------------------------------------------------------------------- */
int main() {
  testBuildingParentDirectoryMap();
  testBuildingParentDirectoryMapWithParentIdOverflow();
  testBuildingParentDirectoryMapWithParentIdEqualToSize();
  testBuildingParentHashMap();
  testBuildingParentHashMapWithDirectoryIdOverflow();
  testBuildingParentHashMapWithDirectoryIdEqualToSize();
  testBuildINodeTree();
  testCalculateHashes();
  testCalculateHashesErrorWhenLeafNodeDoesNotHaveHash();
  testCountShortestDepth();
  testCountShortestDepthWithLengthZeroReturnsZero();
  testFastForwardingINodeReferencesToShallowestDepth();
  testFastForwardingINodeReferencesToRoot();
  testFastForwardingINodeReferencesWithZeroDepth();
  testFilteringNonDuplicatesAndNestedHashes();
  testBuildDuplicatePathSegments();
  testINodeEquality();
  testINodeEqualityNotEqual();
  testINodeHashes();
  testINodeHashesEqual();
  testINodeHashesNotEqual();
  testTransforming();
}
