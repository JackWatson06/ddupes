#include <unistd.h>

#include <cassert>
#include <cstring>

#include "../../src/dupes/extract.cpp"
#include "../../src/dupes/extract_output.cpp"
#include "../../src/lib.cpp"
#include "../../src/sqlite/operators.cpp"
#include "../data.cpp"

/* -------------------------------------------------------------------------- */
/*                                    Mocks                                   */
/* -------------------------------------------------------------------------- */

/* ---------------------------- File System Mocks --------------------------- */

/*

- Root Path: /home/testing/desktop/

- Path One: ./dir1/
- Path Two: ../documents/dir2/

- Files:
/home/testing/desktop/dir1/testing/test/example_one.txt
/home/testing/desktop/dir1/testing/test/example_two.txt
/home/testing/desktop/dir1/testing/example_three.txt
/home/testing/desktop/dir1/testing/example_four.txt
/home/testing/desktop/dir1/example_five.txt

/home/testing/documents/dir2/example_one.txt
/home/testing/documents/dir2/testing/example_two.txt
/home/testing/documents/dir2/dir3/testing/test/example_three.txt
/home/testing/documents/dir2/testing/example_four.txt
*/

std::string qualifyRelativeURL(std::string &relative_path) {
  if (relative_path == "./dir1/") {
    return "/home/testing/desktop/dir1";
  }

  if (relative_path == "../documents/dir2/") {
    return "/home/testing/documents/dir2";
  }

  return "/home/testing/desktop/" + relative_path;
}

void visitFiles(const std::string &directory_path,
                file_visitor_callback visitor_callback, void *context) {
  if (directory_path == "./dir1/") {
    unsigned int num_of_files = 12;
    std::string paths[num_of_files]{
        "./dir1/testing",
        "./dir1/testing/nested_one",
        "./dir1/testing/nested_one/nested_two",
        "./dir1/testing/nested_one/nested_two/nested_three",
        "./dir1/testing/nested_one/nested_two/nested_three/nested_four",
        "./dir1/testing/nested_one/nested_two/nested_three/nested_four/"
        "example_five.txt",
        "./dir1/testing/test",
        "./dir1/testing/test/example_one.txt",
        "./dir1/testing/test/example_two.txt",
        "./dir1/testing/example_three.txt",
        "./dir1/testing/example_four.txt",
        "./dir1/example_five.txt"};
    enum file_type types[num_of_files]{
        FILE_TYPE_DIRECTORY, FILE_TYPE_DIRECTORY, FILE_TYPE_DIRECTORY,
        FILE_TYPE_DIRECTORY, FILE_TYPE_DIRECTORY, FILE_TYPE_FILE,
        FILE_TYPE_DIRECTORY, FILE_TYPE_FILE,      FILE_TYPE_FILE,
        FILE_TYPE_FILE,      FILE_TYPE_FILE,      FILE_TYPE_FILE};
    for (int i = 0; i < num_of_files; ++i) {
      visitor_callback(paths[i], types[i], context);
    }
  }

  if (directory_path == "../documents/dir2/") {
    unsigned int num_of_files = 8;
    std::string paths[num_of_files]{
        "../documents/dir2/example_one.txt",
        "../documents/dir2/testing",
        "../documents/dir2/testing/example_two.txt",
        "../documents/dir2/testing/example_three.txt",
        "../documents/dir2/dir3",
        "../documents/dir2/dir3/testing",
        "../documents/dir2/dir3/testing/test",
        "../documents/dir2/dir3/testing/test/example_four.txt",
    };
    enum file_type types[num_of_files]{
        FILE_TYPE_FILE,      FILE_TYPE_DIRECTORY, FILE_TYPE_FILE,
        FILE_TYPE_FILE,      FILE_TYPE_DIRECTORY, FILE_TYPE_DIRECTORY,
        FILE_TYPE_DIRECTORY, FILE_TYPE_FILE,
    };
    for (int i = 0; i < num_of_files; ++i) {
      visitor_callback(paths[i], types[i], context);
    }
    return;
  }
}

void extractHash(uint8_t *hash, std::string path) {
  for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
    hash[i] = 255;
  }
}

/* ------------------------------ Database Mock ----------------------------- */

const directory_table_row::rows MOCK_DIRECTORIES{
    {1, "dir1", -1},   {2, "dir2", -1},  {3, "oranges", 1},
    {4, "oranges", 2}, {5, "apples", 1},
};
const hash_table_row::rows MOCK_HASHES{
    {1, "testing.txt", uniqueTestHash()},
    {3, "testing_two.txt", uniqueTestHash()},
    {4, "testing_three.txt", uniqueTestHash()}};

sqlite3 *MOCK_DB = nullptr;

bool reset_db = false;
std::vector<directory_input> created_directories{};
std::vector<hash_input> created_hashes{};
int created_directory_id = 0;

void resetDB(sqlite3 *db) { reset_db = true; }

directory_table_row::rows fetchAllDirectories(sqlite3 *db) {
  return MOCK_DIRECTORIES;
}

int createDirectory(sqlite3 *db, directory_input const &directory_table_input) {
  created_directories.push_back(
      directory_input{.parent_id = directory_table_input.parent_id,
                      .name = stringDup(directory_table_input.name)});
  return ++created_directory_id;
}

hash_table_row::rows fetchAllHashes(sqlite3 *db) { return MOCK_HASHES; }

void createHash(sqlite3 *db, hash_input const &hash_table_input) {
  uint8_t *hash_buffer = new uint8_t[MD5_DIGEST_LENGTH];
  std::memcpy(hash_buffer, hash_table_input.hash, MD5_DIGEST_LENGTH);

  created_hashes.push_back(
      hash_input{.directory_id = hash_table_input.directory_id,
                 .name = stringDup(hash_table_input.name),
                 .hash = hash_buffer});
}

void resetMockStates() {
  created_directory_id = 0;
  reset_db = false;
  created_directories.clear();
  created_hashes.clear();
}

/* -------------------------------------------------------------------------- */
/*                                    Tests                                   */
/* -------------------------------------------------------------------------- */
/* --------------------------------- extract -------------------------------- */
void testExtract() {
  // Act
  file_hash_rows actual_file_hash_rows = extractUsingCache(MOCK_DB);

  // Assert
  file_hash_rows expected_file_hash_rows{MOCK_DIRECTORIES, MOCK_HASHES};
  assert(actual_file_hash_rows == expected_file_hash_rows);
}

/* ------------------------------- buildCache ------------------------------- */
void testBuildCacheResetsDB() {
  // Arrange
  resetMockStates();
  std::vector<std::string> test_paths = {"./dir1/", "../documents/dir2/"};

  // Act
  buildCache(test_paths, MOCK_DB);

  // Assert
  assert(reset_db);
}

void testBuildCacheCreatesDirectories() {
  // home/testing/desktop/./dir1/
  // home/testing/desktop/../documents/dir2/

  // 1, testing -1
  // 2, desktop 1
  // 3, dir1, 2
  // 4, testing, 3
  // 5, nested_one, 4
  // 6, nested_two, 5
  // 7, nested_three, 6
  // 8, nested_four, 7
  // 9, test, 4
  // 10, documents, 1,
  // 11, dir2, 10,
  // 12, testing, 11,
  // 13, dir3, 11,
  // 14, testing, 13,
  // 15, test, 14

  // Arrange
  resetMockStates();
  std::vector<std::string> test_paths = {"./dir1/", "../documents/dir2/"};

  // Act
  buildCache(test_paths, MOCK_DB);

  // Assert
  std::vector<directory_input> expected_created_directories{
      {-1, "testing"},     {1, "desktop"},     {2, "dir1"},
      {3, "testing"},      {4, "nested_one"},  {5, "nested_two"},
      {6, "nested_three"}, {7, "nested_four"}, {4, "test"},
      {1, "documents"},    {10, "dir2"},       {11, "testing"},
      {11, "dir3"},        {13, "testing"},    {14, "test"},
  };

  assert(expected_created_directories.size() == created_directories.size());
  for (int i = 0; i < expected_created_directories.size(); ++i) {
    assert(expected_created_directories[i].parent_id ==
           created_directories[i].parent_id);
    assert(compareStrings(expected_created_directories[i].name,
                          created_directories[i].name));
  }
}

void testBuildCacheCreatesHashes() {
  // Arrange
  resetMockStates();
  std::vector<std::string> test_paths = {"./dir1/", "../documents/dir2/"};

  // Act
  buildCache(test_paths, MOCK_DB);

  // Assert
  std::vector<hash_input> expected_created_hashes{
      {8, "example_five.txt", uniqueTestHash()},
      {9, "example_one.txt", uniqueTestHash()},
      {9, "example_two.txt", uniqueTestHash()},
      {4, "example_three.txt", uniqueTestHash()},
      {4, "example_four.txt", uniqueTestHash()},
      {3, "example_five.txt", uniqueTestHash()},
      {11, "example_one.txt", uniqueTestHash()},
      {12, "example_two.txt", uniqueTestHash()},
      {12, "example_three.txt", uniqueTestHash()},
      {15, "example_four.txt", uniqueTestHash()},
  };
  assert(expected_created_hashes.size() == created_hashes.size());
  for (int i = 0; i < expected_created_hashes.size(); ++i) {
    assert(expected_created_hashes[i].directory_id ==
           created_hashes[i].directory_id);
    assert(compareStrings(expected_created_hashes[i].name,
                          created_hashes[i].name));
    assert(
        compareHashes(expected_created_hashes[i].hash, created_hashes[i].hash));
  }
}

/* ---------------------------- tokenizeRootPath ---------------------------- */
void testTokenizingPathWithRoot() {
  // Arrange
  std::string test_path = "/";

  // Act
  std::vector<std::string> tokenized_path = tokenizeRootPath(test_path);

  // Arrange
  std::vector<std::string> expected_tokenized_path{"/"};
  assert(tokenized_path == expected_tokenized_path);
}

void testTokenizingPathWithRootFolder() {
  // Arrange
  std::string test_path = "/testing";

  // Act
  std::vector<std::string> tokenized_path = tokenizeRootPath(test_path);

  // Arrange
  std::vector<std::string> expected_tokenized_path{"/", "testing"};
  assert(tokenized_path == expected_tokenized_path);
}

void testTokenizingPathWithFile() {
  // Arrange
  std::string test_path = "/testing/testing.txt";

  // Act
  std::vector<std::string> tokenized_path = tokenizeRootPath(test_path);

  // Arrange
  std::vector<std::string> expected_tokenized_path{"/", "testing",
                                                   "testing.txt"};
  assert(tokenized_path == expected_tokenized_path);
}

void testTokenizingPathWithNestedFolders() {
  // Arrange
  std::string test_path = "/testing/test";

  // Act
  std::vector<std::string> tokenized_path = tokenizeRootPath(test_path);

  // Arrange
  std::vector<std::string> expected_tokenized_path{"/", "testing", "test"};
  assert(tokenized_path == expected_tokenized_path);
}

void testTokenizingWithTrailingSlash() {
  // Arrange
  std::string test_path = "/testing/test/";

  // Act
  std::vector<std::string> tokenized_path = tokenizeRootPath(test_path);

  // Arrange
  std::vector<std::string> expected_tokenized_path{"/", "testing", "test"};
  assert(tokenized_path == expected_tokenized_path);
}

void testTokenizingNotAtRoot() {
  // Arrange
  std::string test_path = "testing/test/example_one.txt";

  // Act
  std::vector<std::string> tokenized_path = tokenizeRootPath(test_path);

  // Arrange
  std::vector<std::string> expected_tokenized_path{"/", "testing", "test",
                                                   "example_one.txt"};
  assert(tokenized_path == expected_tokenized_path);
}

void testTokenizingEmptyRoot() {
  // Arrange
  std::string test_path = "";

  // Act
  std::vector<std::string> tokenized_path = tokenizeRootPath(test_path);

  // Arrange
  std::vector<std::string> expected_tokenized_path{};
  assert(tokenized_path == expected_tokenized_path);
}

/* -------------------------- tokenizeRelativePath -------------------------- */
void testTokenizingRelativePath() {
  // Arrange
  std::string test_path = "testing/test/example_one.txt";

  // Act
  std::vector<std::string> tokenized_path = tokenizeRelativePath(test_path);

  // Arrange
  std::vector<std::string> expected_tokenized_path{"testing", "test",
                                                   "example_one.txt"};
  assert(tokenized_path == expected_tokenized_path);
}

void testTokenizingRelativePathWithRootSlash() {
  // Arrange
  std::string test_path = "/testing/test/example_one.txt";

  // Act
  std::vector<std::string> tokenized_path = tokenizeRelativePath(test_path);

  // Arrange
  std::vector<std::string> expected_tokenized_path{"testing", "test",
                                                   "example_one.txt"};
  assert(tokenized_path == expected_tokenized_path);
}

void testTokenizingEmptyPath() {
  // Arrange
  std::string test_path = "";

  // Act
  std::vector<std::string> tokenized_path = tokenizeRelativePath(test_path);

  // Arrange
  std::vector<std::string> expected_tokenized_path{};
  assert(tokenized_path == expected_tokenized_path);
}

/* ---------------------------- countShortestPath --------------------------- */
void testCountShortestPath() {
  // Arrange
  std::vector<std::vector<std::string>> shortest_paths{
      {"/", "testing", "test"},
      {"/", "testing"},
      {"/", "testing", "test", "testing_this"}};

  // Act
  int shortest_path = countShortestTokenizePath(shortest_paths);

  // Arrange
  assert(shortest_path == 2);
}

void testCountShortestPathWithEmptyDir() {
  // Arrange
  std::vector<std::vector<std::string>> shortest_paths{
      {"/", "testing", "test"}, {}, {"/", "testing", "test", "testing_this"}};

  // Act
  int shortest_path = countShortestTokenizePath(shortest_paths);

  // Arrange
  assert(shortest_path == 0);
}

/* ------------------------------ calcRootPath ------------------------------ */
void testCalcRootPath() {
  // Arrange
  std::vector<std::string> test_arguments{"dir1/testing/test", "dir2/testing"};

  // Act
  root_calc_result actual_argument_paths =
      calcRootPath(test_arguments, MOCK_DB);

  // Assert
  root_calc_result expected_argument_paths{
      .root_path = "desktop",
      {{"dir1/testing/test", {"dir1", "testing", "test"}},
       {"dir2/testing", {"dir2", "testing"}}}};
  assert(actual_argument_paths == expected_argument_paths);
}

void testCalcRootWithEmpty() {
  // Arrange
  std::vector<std::string> test_arguments{"", "dir2/testing"};

  // Act
  root_calc_result actual_argument_paths =
      calcRootPath(test_arguments, MOCK_DB);

  // Assert
  root_calc_result expected_argument_paths{
      .root_path = "desktop",
      {{"", {}}, {"dir2/testing", {"dir2", "testing"}}}};
  assert(actual_argument_paths == expected_argument_paths);
}

/* ------------------------------ ArgumentPath ------------------------------ */
void testArgumentPathsEquality() {
  // Arrange
  argument_path test_args_one{"dir1/testing/test", {"dir1", "testing", "test"}};
  argument_path test_args_two{"dir1/testing/test", {"dir1", "testing", "test"}};

  // Act
  bool equality_check = test_args_one == test_args_two;

  // Assert
  assert(equality_check);
}

/* ----------------------------- RootCalcResult ----------------------------- */
void testRootCalcResultEquality() {
  // Arrange
  root_calc_result test_paths_one{
      .root_path = "desktop",
      {{"dir1/testing/test", {"dir1", "testing", "test"}},
       {"dir2/testing", {"dir2", "testing"}}}};
  root_calc_result test_paths_two{
      .root_path = "desktop",
      {{"dir1/testing/test", {"dir1", "testing", "test"}},
       {"dir2/testing", {"dir2", "testing"}}}};

  // Act
  bool equality_check = test_paths_one == test_paths_two;

  // Assert
  assert(equality_check);
}

/* ------------------------ removeLeadingRelativePath ----------------------- */
void testRemoveLeadingRelativePath() {
  // Assert
  std::string test_leading_relative_path = "../dir1/./dir2";
  std::string test_file_path = "../dir1/./dir2/testing/this/out";

  // Act
  std::string actual_path_without_leading =
      removeLeadingRelativePath(test_leading_relative_path, test_file_path);

  // Assert
  std::string expected_path = "/testing/this/out";
  assert(expected_path == actual_path_without_leading);
}

/* -------------------------------------------------------------------------- */
/*                                    Main                                    */
/* -------------------------------------------------------------------------- */
int main() {
  testExtract();
  testBuildCacheResetsDB();
  testBuildCacheCreatesDirectories();
  testBuildCacheCreatesHashes();
  testTokenizingPathWithRoot();
  testTokenizingPathWithRootFolder();
  testTokenizingPathWithFile();
  testTokenizingPathWithNestedFolders();
  testTokenizingWithTrailingSlash();
  testTokenizingNotAtRoot();
  testTokenizingEmptyRoot();
  testTokenizingRelativePathWithRootSlash();
  testTokenizingRelativePath();
  testTokenizingEmptyPath();
  testCountShortestPath();
  testCountShortestPathWithEmptyDir();
  testCalcRootPath();
  testCalcRootWithEmpty();
  testArgumentPathsEquality();
  testRootCalcResultEquality();
  testRemoveLeadingRelativePath();
}