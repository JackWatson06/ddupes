#include <unistd.h>

#include <cassert>
#include <cstring>

#include "../src/constants.h"
#include "../src/extract.h"
#include "../src/fs/file_system.h"
#include "../src/sqlite/sqlite.h"

/* -------------------------------------------------------------------------- */
/*                                    Mocks                                   */
/* -------------------------------------------------------------------------- */

uint8_t* uniqueTestBlob(uint8_t first_byte = 255) {
  return new uint8_t[MD5_DIGEST_LENGTH]{first_byte, 255, 255, 255, 255, 255,
                                        255,        255, 255, 255, 255, 255,
                                        255,        255, 255, 255};
}

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

std::string mockQualifyRelativePath(std::string& path) {
  if (path == "./dir1/") {
    return "/home/testing/desktop/dir1";
  }

  if (path == "../documents/dir2/") {
    return "/home/testing/documents/dir2";
  }

  return "/home/testing/desktop/" + path;
}

void mockVisitFiles(const std::string& directory_path,
                    FileVisitorCallback visitor_callback, void* context) {
  if (directory_path == "./dir1/") {
    unsigned int num_of_files = 7;
    std::string paths[num_of_files]{"./dir1/testing",
                                    "./dir1/testing/test",
                                    "./dir1/testing/test/example_one.txt",
                                    "./dir1/testing/test/example_two.txt",
                                    "./dir1/testing/example_three.txt",
                                    "./dir1/testing/example_four.txt",
                                    "./dir1/example_five.txt"};
    enum FileType types[num_of_files]{FILE_TYPE_DIRECTORY, FILE_TYPE_DIRECTORY,
                                      FILE_TYPE_FILE,      FILE_TYPE_FILE,
                                      FILE_TYPE_FILE,      FILE_TYPE_FILE,
                                      FILE_TYPE_FILE};
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
    enum FileType types[num_of_files]{
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

void mockExtractHash(uint8_t* hash, std::string path) {
  for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
    hash[i] = 255;
  }
}

CreateCacheFileSystemServices mockCacheFileSystemServices() {
  return {mockQualifyRelativePath, mockVisitFiles, mockExtractHash};
}

/* ------------------------------ Database Mock ----------------------------- */

const DirectoryTableRow::Rows MOCK_DIRECTORIES{
    {1, "dir1", -1},   {2, "dir2", -1},  {3, "oranges", 1},
    {4, "oranges", 2}, {5, "apples", 1},
};
const HashTableRow::Rows MOCK_HASHES{
    {1, "testing.txt", uniqueTestBlob()},
    {3, "testing_two.txt", uniqueTestBlob()},
    {4, "testing_three.txt", uniqueTestBlob()}};

bool reset_db = false;
std::vector<DirectoryInput> created_directories{};
std::vector<HashInput> created_hashes{};
int created_directory_id = 0;

void mockResetDB(sqlite3* db) { reset_db = true; }

unsigned int mockFetchDirectoryIdByName(sqlite3* db, const std::string& name) {
  return 1;
}

DirectoryTableRow::Rows mockFetchAllDirectories(sqlite3* db) {
  return MOCK_DIRECTORIES;
}

int mockCreateDirectory(sqlite3* db,
                        const DirectoryInput& directory_table_input) {
  created_directories.push_back(directory_table_input);
  return ++created_directory_id;
}

HashTableRow::Rows mockFetchAllHashes(sqlite3* db) { return MOCK_HASHES; }

void mockCreateHash(sqlite3* db, const HashInput& hash_table_input) {
  uint8_t* hash_buffer = new uint8_t[MD5_DIGEST_LENGTH];
  std::memcpy(hash_buffer, hash_table_input.hash, MD5_DIGEST_LENGTH);

  created_hashes.push_back(
      HashInput{.directory_id = hash_table_input.directory_id,
                .name = hash_table_input.name,
                .hash = hash_buffer});
}

CreateCacheDatabaseServices mockCacheDatabaseServices() {
  return {nullptr, mockResetDB, mockCreateDirectory, mockCreateHash};
}

ExtractDatabaseServices mockExtractDatabaseServices() {
  return {nullptr, mockFetchAllDirectories, mockFetchAllHashes};
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
  FileHashRows actual_file_hash_rows =
      extractUsingCache(mockExtractDatabaseServices());

  // Assert
  FileHashRows expected_file_hash_rows{MOCK_DIRECTORIES, MOCK_HASHES};
  assert(actual_file_hash_rows == expected_file_hash_rows);
}

/* ------------------------------- buildCache ------------------------------- */
void testBuildCacheResetsDB() {
  // Arrange
  resetMockStates();
  SVector test_paths = {"./dir1/", "../documents/dir2/"};

  // Act
  buildCache(test_paths, mockCacheDatabaseServices(),
             mockCacheFileSystemServices());

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
  // 5, test, 4
  // 6, documents, 1,
  // 7, dir2, 6,
  // 8, testing, 7,
  // 9  dir3, 7,
  // 10, testing, 9,
  // 11, test, 10

  // Arrange
  resetMockStates();
  SVector test_paths = {"./dir1/", "../documents/dir2/"};

  // Act
  buildCache(test_paths, mockCacheDatabaseServices(),
             mockCacheFileSystemServices());

  // Assert
  std::vector<DirectoryInput> expected_created_directories{
      {-1, "testing"}, {1, "desktop"},   {2, "dir1"},  {3, "testing"},
      {4, "test"},     {1, "documents"}, {6, "dir2"},  {7, "testing"},
      {7, "dir3"},     {9, "testing"},   {10, "test"},
  };
  assert(expected_created_directories == created_directories);
}

void testBuildCacheCreatesHashes() {
  // Arrange
  resetMockStates();
  SVector test_paths = {"./dir1/", "../documents/dir2/"};

  // Act
  buildCache(test_paths, mockCacheDatabaseServices(),
             mockCacheFileSystemServices());

  // Assert
  std::vector<HashInput> expected_created_hashes{
      {5, "example_one.txt", uniqueTestBlob()},
      {5, "example_two.txt", uniqueTestBlob()},
      {4, "example_three.txt", uniqueTestBlob()},
      {4, "example_four.txt", uniqueTestBlob()},
      {3, "example_five.txt", uniqueTestBlob()},
      {7, "example_one.txt", uniqueTestBlob()},
      {8, "example_two.txt", uniqueTestBlob()},
      {8, "example_three.txt", uniqueTestBlob()},
      {11, "example_four.txt", uniqueTestBlob()},
  };
  assert(expected_created_hashes == created_hashes);
}

/* ---------------------------- tokenizeRootPath ---------------------------- */
void testTokenizingPathWithRoot() {
  // Arrange
  std::string test_path = "/";

  // Act
  SVector tokenized_path = tokenizeRootPath(test_path);

  // Arrange
  SVector expected_tokenized_path{"/"};
  assert(tokenized_path == expected_tokenized_path);
}

void testTokenizingPathWithRootFolder() {
  // Arrange
  std::string test_path = "/testing";

  // Act
  SVector tokenized_path = tokenizeRootPath(test_path);

  // Arrange
  SVector expected_tokenized_path{"/", "testing"};
  assert(tokenized_path == expected_tokenized_path);
}

void testTokenizingPathWithFile() {
  // Arrange
  std::string test_path = "/testing/testing.txt";

  // Act
  SVector tokenized_path = tokenizeRootPath(test_path);

  // Arrange
  SVector expected_tokenized_path{"/", "testing", "testing.txt"};
  assert(tokenized_path == expected_tokenized_path);
}

void testTokenizingPathWithNestedFolders() {
  // Arrange
  std::string test_path = "/testing/test";

  // Act
  SVector tokenized_path = tokenizeRootPath(test_path);

  // Arrange
  SVector expected_tokenized_path{"/", "testing", "test"};
  assert(tokenized_path == expected_tokenized_path);
}

void testTokenizingWithTrailingSlash() {
  // Arrange
  std::string test_path = "/testing/test/";

  // Act
  SVector tokenized_path = tokenizeRootPath(test_path);

  // Arrange
  SVector expected_tokenized_path{"/", "testing", "test"};
  assert(tokenized_path == expected_tokenized_path);
}

void testTokenizingNotAtRoot() {
  // Arrange
  std::string test_path = "testing/test/example_one.txt";

  // Act
  SVector tokenized_path = tokenizeRootPath(test_path);

  // Arrange
  SVector expected_tokenized_path{"/", "testing", "test", "example_one.txt"};
  assert(tokenized_path == expected_tokenized_path);
}

void testTokenizingEmptyRoot() {
  // Arrange
  std::string test_path = "";

  // Act
  SVector tokenized_path = tokenizeRootPath(test_path);

  // Arrange
  SVector expected_tokenized_path{};
  assert(tokenized_path == expected_tokenized_path);
}

/* -------------------------- tokenizeRelativePath -------------------------- */
void testTokenizingRelativePath() {
  // Arrange
  std::string test_path = "testing/test/example_one.txt";

  // Act
  SVector tokenized_path = tokenizeRelativePath(test_path);

  // Arrange
  SVector expected_tokenized_path{"testing", "test", "example_one.txt"};
  assert(tokenized_path == expected_tokenized_path);
}

void testTokenizingRelativePathWithRootSlash() {
  // Arrange
  std::string test_path = "/testing/test/example_one.txt";

  // Act
  SVector tokenized_path = tokenizeRelativePath(test_path);

  // Arrange
  SVector expected_tokenized_path{"testing", "test", "example_one.txt"};
  assert(tokenized_path == expected_tokenized_path);
}

void testTokenizingEmptyPath() {
  // Arrange
  std::string test_path = "";

  // Act
  SVector tokenized_path = tokenizeRelativePath(test_path);

  // Arrange
  SVector expected_tokenized_path{};
  assert(tokenized_path == expected_tokenized_path);
}

/* ---------------------------- countShortestPath --------------------------- */
void testCountShortestPath() {
  // Arrange
  std::vector<SVector> shortest_paths{{"/", "testing", "test"},
                                      {"/", "testing"},
                                      {"/", "testing", "test", "testing_this"}};

  // Act
  int shortest_path = countShortestTokenizePath(shortest_paths);

  // Arrange
  assert(shortest_path == 2);
}

void testCountShortestPathWithEmptyDir() {
  // Arrange
  std::vector<SVector> shortest_paths{
      {"/", "testing", "test"}, {}, {"/", "testing", "test", "testing_this"}};

  // Act
  int shortest_path = countShortestTokenizePath(shortest_paths);

  // Arrange
  assert(shortest_path == 0);
}

/* ------------------------------ calcRootPath ------------------------------ */
void testCalcRootPath() {
  // Arrange
  SVector test_arguments{"dir1/testing/test", "dir2/testing"};

  // Act
  RootCalcResult actual_argument_paths =
      calcRootPath(test_arguments, mockCacheFileSystemServices());

  // Assert
  RootCalcResult expected_argument_paths{
      .root_path = "desktop",
      {{"dir1/testing/test", {"dir1", "testing", "test"}},
       {"dir2/testing", {"dir2", "testing"}}}};
  assert(actual_argument_paths == expected_argument_paths);
}

void testCalcRootWithEmpty() {
  // Arrange
  SVector test_arguments{"", "dir2/testing"};

  // Act
  RootCalcResult actual_argument_paths =
      calcRootPath(test_arguments, mockCacheFileSystemServices());

  // Assert
  RootCalcResult expected_argument_paths{
      .root_path = "desktop",
      {{"", {}}, {"dir2/testing", {"dir2", "testing"}}}};
  assert(actual_argument_paths == expected_argument_paths);
}

/* ------------------------------ ArgumentPath ------------------------------ */
void testArgumentPathsEquality() {
  // Arrange
  ArgumentPath test_args_one{"dir1/testing/test", {"dir1", "testing", "test"}};
  ArgumentPath test_args_two{"dir1/testing/test", {"dir1", "testing", "test"}};

  // Act
  bool equality_check = test_args_one == test_args_two;

  // Assert
  assert(equality_check);
}

/* ----------------------------- RootCalcResult ----------------------------- */
void testRootCalcResultEquality() {
  // Arrange
  RootCalcResult test_paths_one{
      .root_path = "desktop",
      {{"dir1/testing/test", {"dir1", "testing", "test"}},
       {"dir2/testing", {"dir2", "testing"}}}};
  RootCalcResult test_paths_two{
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