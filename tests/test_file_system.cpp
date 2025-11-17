
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "../src/fs/file_system.cpp"
#include "./data.cpp"

struct FileNode {
  std::string file_name;
  file_type type;

  bool operator==(const FileNode &rhs) const {
    return rhs.file_name == file_name && rhs.type == type;
  }
};

typedef std::vector<FileNode> FileNodes;

void testFileVisitorCallback(const std::string path, const enum file_type type,
                             void *context) {
  FileNodes *visited_files = static_cast<FileNodes *>(context);

  visited_files->push_back({.file_name = path, .type = type});
}

/* --------------------------- qualifyRelativeURL --------------------------- */
void testAbsoluteFileResolution() {
  // Arrange
  std::string folder = "tests/testing_dirs/dir1";

  // Act
  std::string actual_absolute_path = qualifyRelativeURL(folder);

  // Assert
  assert(actual_absolute_path != folder);
}

void testAbsoluteFileResolutionWithDirectoryLinks() {
  // Arrange
  std::string folder = "./tests/testing_dirs/./../testing_dirs/dir1";

  // Act
  std::string actual_absolute_path = qualifyRelativeURL(folder);

  // Assert
  std::string expected_path = "tests/testing_dirs/dir1";
  assert(actual_absolute_path != folder);
}

/* ------------------------------- visitFiles ------------------------------- */
void testVisitingAllFilesAndDirectories() {
  // Arrange
  std::string folder = "./tests/testing_dirs/../testing_dirs/dir1";
  FileNodes visited_files{};

  // Act
  visitFiles(folder, testFileVisitorCallback, &visited_files);

  // Assert
  FileNodes expected_file_nodes{
      {"./tests/testing_dirs/../testing_dirs/dir1/testing_two",
       FILE_TYPE_DIRECTORY},
      {"./tests/testing_dirs/../testing_dirs/dir1/sub_dir_two",
       FILE_TYPE_DIRECTORY},
      {"./tests/testing_dirs/../testing_dirs/dir1/sub_dir_two/example8.txt",
       FILE_TYPE_FILE},
      {"./tests/testing_dirs/../testing_dirs/dir1/sub_dir_two/example7.txt",
       FILE_TYPE_FILE},
      {"./tests/testing_dirs/../testing_dirs/dir1/example2.txt",
       FILE_TYPE_FILE},
      {"./tests/testing_dirs/../testing_dirs/dir1/testing_one",
       FILE_TYPE_DIRECTORY},
      {"./tests/testing_dirs/../testing_dirs/dir1/testing_one/testing_three",
       FILE_TYPE_DIRECTORY},
      {"./tests/testing_dirs/../testing_dirs/dir1/"
       "testing_one/testing_three/testing_four",
       FILE_TYPE_DIRECTORY},
      {"./tests/testing_dirs/../testing_dirs/dir1/testing_one/"
       "testing_three/testing_four/example1.txt",
       FILE_TYPE_FILE},
      {"./tests/testing_dirs/../testing_dirs/dir1/example5.txt",
       FILE_TYPE_FILE},
      {"./tests/testing_dirs/../testing_dirs/dir1/example6.txt",
       FILE_TYPE_FILE},
      {"./tests/testing_dirs/../testing_dirs/dir1/example3.txt",
       FILE_TYPE_FILE},
      {"./tests/testing_dirs/../testing_dirs/dir1/sub_dir",
       FILE_TYPE_DIRECTORY},
      {"./tests/testing_dirs/../testing_dirs/dir1/sub_dir/example4.txt",
       FILE_TYPE_FILE},
      {"./tests/testing_dirs/../testing_dirs/dir1/sub_dir/example6.txt",
       FILE_TYPE_FILE},
      {"./tests/testing_dirs/../testing_dirs/dir1/example1.txt",
       FILE_TYPE_FILE}};
  assert(expected_file_nodes == visited_files);
}

void testVisitFileSkipsPermissionIssues() {
  // Arrange
  std::filesystem::permissions("tests/testing_dirs/dir1/sub_dir_two",
                               std::filesystem::perms::owner_write);
  std::string folder = "./tests/testing_dirs/../testing_dirs/dir1";
  FileNodes visited_files{};

  // Act
  visitFiles(folder, testFileVisitorCallback, &visited_files);

  // Assert
  FileNodes expected_file_nodes{
      {"./tests/testing_dirs/../testing_dirs/dir1/testing_two",
       FILE_TYPE_DIRECTORY},
      {"./tests/testing_dirs/../testing_dirs/dir1/example2.txt",
       FILE_TYPE_FILE},
      {"./tests/testing_dirs/../testing_dirs/dir1/testing_one",
       FILE_TYPE_DIRECTORY},
      {"./tests/testing_dirs/../testing_dirs/dir1/testing_one/testing_three",
       FILE_TYPE_DIRECTORY},
      {"./tests/testing_dirs/../testing_dirs/dir1/"
       "testing_one/testing_three/testing_four",
       FILE_TYPE_DIRECTORY},
      {"./tests/testing_dirs/../testing_dirs/dir1/testing_one/"
       "testing_three/testing_four/example1.txt",
       FILE_TYPE_FILE},
      {"./tests/testing_dirs/../testing_dirs/dir1/example5.txt",
       FILE_TYPE_FILE},
      {"./tests/testing_dirs/../testing_dirs/dir1/example6.txt",
       FILE_TYPE_FILE},
      {"./tests/testing_dirs/../testing_dirs/dir1/example3.txt",
       FILE_TYPE_FILE},
      {"./tests/testing_dirs/../testing_dirs/dir1/sub_dir",
       FILE_TYPE_DIRECTORY},
      {"./tests/testing_dirs/../testing_dirs/dir1/sub_dir/example4.txt",
       FILE_TYPE_FILE},
      {"./tests/testing_dirs/../testing_dirs/dir1/sub_dir/example6.txt",
       FILE_TYPE_FILE},
      {"./tests/testing_dirs/../testing_dirs/dir1/example1.txt",
       FILE_TYPE_FILE}};
  assert(expected_file_nodes == visited_files);
}

/* ------------------------------- extractHash ------------------------------ */
void testHashingAFile() {
  // Arrange
  std::string file = "tests/testing_dirs/dir1/example5.txt";

  // Act
  uint8_t actual_md5_digest[MD5_DIGEST_LENGTH];
  extractHash(actual_md5_digest, file);

  // Assert
  uint8_t expected_hash[MD5_DIGEST_LENGTH]{
      150, 69, 150, 207, 35, 44, 71, 46, 198, 59, 175, 2, 153, 240, 212, 80};
  for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
    assert(expected_hash[i] == actual_md5_digest[i]);
  }
}

void testHashingAFileThatDoesntExist() {
  // Arrange
  std::string file = "tests/testing_dirs/dir1/does_not_exist.txt";
  uint8_t actual_md5_digest[MD5_DIGEST_LENGTH];

  // Act
  try {
    extractHash(actual_md5_digest, file);

    // Assert
    assert(false);
  } catch (file_open_error &e) {
    assert(true);
  }
}

void testHashingAnEmptyFile() {
  // Arrange
  std::string file = "tests/testing_dirs/dir1/example6.txt";

  // Act
  uint8_t actual_md5_digest[MD5_DIGEST_LENGTH];
  extractHash(actual_md5_digest, file);

  // Assert
  uint8_t expected_hash[MD5_DIGEST_LENGTH]{0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0};
  for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
    assert(expected_hash[i] == actual_md5_digest[i]);
  }
}

/* ------------------------------- fileExists ------------------------------- */
void testFileExists() {
  // Arrange
  std::string file = "tests/testing_dirs/dir1/example6.txt";

  // Act
  bool exists = fileExists(file);

  // Assert
  assert(exists == true);
}

void testFileExistsCheckingFolder() {
  // Arrange
  std::string file = "tests/testing_dirs/dir1";

  // Act
  bool exists = fileExists(file);

  // Assert
  assert(exists == true);
}

void testFileExistsMissing() {
  // Arrange
  std::string file = "tests/testing_dirs/dir1/missing.txt";

  // Act
  bool exists = fileExists(file);

  // Assert
  assert(exists == false);
}

/* ----------------------------- createDirectory ---------------------------- */
void testCreateDirectory() {
  // Assert
  std::string test_directory = "tests/testing_dirs/test_create";

  // Act
  createDirectory(test_directory);

  // Assert
  assert(fileExists(test_directory));

  // Cleanup
  std::filesystem::remove(test_directory);
}

void testCreateDirectorySetCorrectPerms() {
  // Assert
  std::string test_directory = "tests/testing_dirs/test_create";

  // Act
  createDirectory(test_directory);

  // Assert
  std::filesystem::perms folder_perms =
      std::filesystem::status(test_directory).permissions();
  std::filesystem::perms expected_perms =
      std::filesystem::perms::owner_all | std::filesystem::perms::group_all |
      std::filesystem::perms::others_read | std::filesystem::perms::others_exec;
  assert(folder_perms == expected_perms);

  // Cleanup
  std::filesystem::remove(test_directory);
}

void testCreateDirectoryPermissionsError() {
  // Assert
  std::filesystem::permissions("tests/testing_dirs",
                               std::filesystem::perms::owner_read);

  try {
    // Act
    createDirectory("tests/testing_dirs/test_create");
    assert(false);
  } catch (create_directory_error &e) {
    // Assert
    assert(true);
  }

  // Cleanup
  std::filesystem::permissions("tests/testing_dirs",
                               std::filesystem::perms::owner_all |
                                   std::filesystem::perms::group_all |
                                   std::filesystem::perms::others_read |
                                   std::filesystem::perms::others_exec);
}

int main() {
  testAbsoluteFileResolution();
  testAbsoluteFileResolutionWithDirectoryLinks();
  testVisitingAllFilesAndDirectories();
  testVisitFileSkipsPermissionIssues();
  testHashingAFile();
  testHashingAFileThatDoesntExist();
  testHashingAnEmptyFile();
  testFileExists();
  testFileExistsMissing();
  testCreateDirectory();
  testCreateDirectorySetCorrectPerms();
  testCreateDirectoryPermissionsError();
}
