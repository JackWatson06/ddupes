
#include "../src/build/build.h"
#include "../src/cli.cpp"
#include "../src/dupes/dupes.h"
#include "../src/env/env.h"
#include "../src/lib.cpp"
#include "../src/update/update.h"
#include <cassert>

/* -------------------------------------------------------------------------- */
/*                                    Mocks                                   */
/* -------------------------------------------------------------------------- */

/* ------------------------------- Environment ------------------------------ */
std::string fetch_home_directory_return = "/home/test";

const char *CACHE_DIR = "/.cache/ddupes";

char *fetchHomeDirectory() {
  if (fetch_home_directory_return.size() == 0) {
    return nullptr;
  }
  return fetch_home_directory_return.data();
}

/* ------------------------------- FileSystem ------------------------------- */
std::string last_create_directory_path{};
std::vector<std::string> last_join_path_path_segments{};

std::string joinPath(std::vector<std::string> const &path_segments) {
  last_join_path_path_segments = path_segments;
  return "/home/test/.cache/ddupes/testing.db";
}

bool fileExists(std::string const &file_path) { return false; }

void createDirectory(std::string const &path) {
  last_create_directory_path = path;
}
/* ---------------------------- Command Services ---------------------------- */
std::string last_dupes_cache_path{};
std::vector<std::string> last_build_paths;
std::string last_build_cache_path{};
std::string last_update_cache_path{};

void dupes(std::string cache_path, std::ostream &console) {
  last_dupes_cache_path = cache_path;
}

void build(std::vector<std::string> paths, std::string cache_path,
           std::ostream &console) {
  last_build_paths = paths;
  last_build_cache_path = cache_path;
}

void update(std::string cache_path, std::ostream &console) {
  last_update_cache_path = cache_path;
}

void resetMocks() {
  fetch_home_directory_return = "/home/test";
  last_join_path_path_segments = {};
  last_dupes_cache_path = {};
  last_build_paths = {};
  last_build_cache_path = {};
  last_update_cache_path = {};
  last_create_directory_path = {};
}

/* --------------------------------- process -------------------------------- */
void testProcessCallsDupesWithCorrectArgs() {
  // Arrange
  resetMocks();
  char test_file_name[] = "ddupes";
  char test_command_name[] = "dupes";
  char test_cache_option[] = "--cache";
  char test_cache_value[] = "testing";
  char *args[4] = {test_file_name, test_command_name, test_cache_option,
                   test_cache_value};

  // Act
  process(4, args);

  // Assert
  assert(last_dupes_cache_path == "/home/test/.cache/ddupes/testing.db");
}

void testProcessCallsBuildWithCorrectArgs() {
  // Arrange
  resetMocks();
  char test_file_name[] = "ddupes";
  char test_command_name[] = "build";
  char test_cache_option[] = "--cache";
  char test_cache_value[] = "testing";
  char test_path_one[] = "path_one";
  char test_path_two[] = "path_two";
  char *args[6] = {test_file_name,   test_command_name, test_cache_option,
                   test_cache_value, test_path_one,     test_path_two};

  // Act
  process(6, args);

  // Assert
  std::vector<std::string> expected_build_paths{"path_one", "path_two"};
  assert(last_build_paths == expected_build_paths);
  assert(last_build_cache_path == "/home/test/.cache/ddupes/testing.db");
}

void testProcessCallsBuildWithCorrectArgsWhenBeforeCache() {
  // Arrange
  resetMocks();
  char test_file_name[] = "ddupes";
  char test_command_name[] = "build";
  char test_path_one[] = "path_one";
  char test_path_two[] = "path_two";
  char test_cache_option[] = "--cache";
  char test_cache_value[] = "testing";
  char *args[6] = {test_file_name, test_command_name, test_path_one,
                   test_path_two,  test_cache_option, test_cache_value};

  // Act
  process(6, args);

  // Assert
  std::vector<std::string> expected_build_paths{"path_one", "path_two"};
  assert(last_build_paths == expected_build_paths);
  assert(last_build_cache_path == "/home/test/.cache/ddupes/testing.db");
}

void testProcessCallsUpdateWithCorrectArgs() {
  // Arrange
  resetMocks();
  char test_file_name[] = "ddupes";
  char test_command_name[] = "update";
  char test_cache_option[] = "--cache";
  char test_cache_value[] = "testing";
  char *args[4] = {test_file_name, test_command_name, test_cache_option,
                   test_cache_value};

  // Act
  process(4, args);

  // Assert
  assert(last_update_cache_path == "/home/test/.cache/ddupes/testing.db");
}

void testProcessErrorsWithLessThanTwoArgs() {
  // Arrange
  resetMocks();
  char test_file_name[] = "ddupes";
  char *args[1] = {test_file_name};

  try {
    // Act
    process(1, args);
    assert(false);
  } catch (command_error &e) {
    // Assert
    assert(true);
  }
}

void testProcessErrorsWhenCallingBuildWithNoPaths() {
  // Arrange
  resetMocks();
  char test_file_name[] = "ddupes";
  char test_command_name[] = "build";
  char test_cache_option[] = "--cache";
  char test_cache_value[] = "testing";
  char *args[4] = {test_file_name, test_command_name, test_cache_option,
                   test_cache_value};

  try {
    // Act
    process(4, args);
    assert(false);
  } catch (command_error &e) {
    // Assert
    assert(true);
  }
}

void testProcessErrorsWithMissingCacheCommand() {
  // Arrange
  resetMocks();
  char test_file_name[] = "ddupes";
  char test_command_name[] = "dupes";
  char test_cache_value[] = "testing";
  char test_cache_option[] = "--cache";
  char *args[4] = {test_file_name, test_command_name, test_cache_value,
                   test_cache_option};

  try {
    // Act
    process(4, args);
    assert(false);
  } catch (command_error &e) {
    // Assert
    assert(true);
  }
}

void testProcessErrorsWithMissingHomeDirectory() {
  // Arrange
  resetMocks();
  fetch_home_directory_return = "";
  char test_file_name[] = "ddupes";
  char test_command_name[] = "dupes";
  char test_cache_option[] = "--cache";
  char test_cache_value[] = "testing";
  char *args[4] = {test_file_name, test_command_name, test_cache_option,
                   test_cache_value};

  try {
    // Act
    process(4, args);
    assert(false);
  } catch (command_error &e) {
    // Assert
    assert(true);
  }
}

void testProcessCreatesCacheDirIfNotExists() {
  // Arrange
  resetMocks();
  char test_file_name[] = "ddupes";
  char test_command_name[] = "dupes";
  char test_cache_option[] = "--cache";
  char test_cache_value[] = "testing";
  char *args[4] = {test_file_name, test_command_name, test_cache_option,
                   test_cache_value};

  // Act
  process(4, args);

  // Assert
  assert(last_create_directory_path == "/home/test/.cache/ddupes");
}

void testProcessJoinsPathToLocateDBFile() {
  // Arrange
  resetMocks();
  char test_file_name[] = "ddupes";
  char test_command_name[] = "dupes";
  char test_cache_option[] = "--cache";
  char test_cache_value[] = "testing";
  char *args[4] = {test_file_name, test_command_name, test_cache_option,
                   test_cache_value};

  // Act
  process(4, args);

  // Assert
  std::vector<std::string> expected_join_path_segments = {
      "/home/test/.cache/ddupes", "testing.db"};
  assert(last_join_path_path_segments == expected_join_path_segments);
}

int main() {
  testProcessCallsDupesWithCorrectArgs();
  testProcessCallsBuildWithCorrectArgs();
  testProcessCallsBuildWithCorrectArgsWhenBeforeCache();
  testProcessCallsUpdateWithCorrectArgs();
  testProcessErrorsWithLessThanTwoArgs();
  testProcessErrorsWhenCallingBuildWithNoPaths();
  testProcessErrorsWithMissingCacheCommand();
  testProcessErrorsWithMissingHomeDirectory();
  testProcessCreatesCacheDirIfNotExists();
  testProcessJoinsPathToLocateDBFile();
}
