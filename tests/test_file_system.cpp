
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "../src/constants.h"
#include "../src/fs/file_system.h"

struct FileNode {
  std::string file_name;
  FileType type;

  bool operator==(const FileNode& rhs) const {
    return rhs.file_name == file_name && rhs.type == type;
  }
};

typedef std::vector<FileNode> FileNodes;

void testFileVisitorCallback(const std::string path, const enum FileType type,
                             void* context) {
  FileNodes* visited_files = static_cast<FileNodes*>(context);

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
  // Regex assert
  // assert(false);
}

void testAbsoluteFileResolutionWithDirectoryLinks() {
  // Arrange
  std::string folder = "./tests/testing_dirs/./../testing_dirs/dir1";

  // Act
  std::string actual_absolute_path = qualifyRelativeURL(folder);

  // Assert
  std::string expected_path = "tests/testing_dirs/dir1";
  assert(actual_absolute_path != folder);
  // assert(false);
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
      {"./tests/testing_dirs/../testing_dirs/dir1/sub_dir_two",
       FILE_TYPE_DIRECTORY},
      {"./tests/testing_dirs/../testing_dirs/dir1/sub_dir_two/example8.txt",
       FILE_TYPE_FILE},
      {"./tests/testing_dirs/../testing_dirs/dir1/sub_dir_two/example7.txt",
       FILE_TYPE_FILE},
      {"./tests/testing_dirs/../testing_dirs/dir1/example2.txt",
       FILE_TYPE_FILE},
      {"./tests/testing_dirs/../testing_dirs/dir1/example5.txt",
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
  } catch (FileOpenError& e) {
    assert(true);
  }
}

int main() {
  testVisitingAllFilesAndDirectories();
  testHashingAFile();
  testHashingAFileThatDoesntExist();
  testAbsoluteFileResolution();
  testAbsoluteFileResolutionWithDirectoryLinks();
}
