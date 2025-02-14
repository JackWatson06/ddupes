#include <unistd.h>

#include <cassert>

#include "extract.h"

/* -------------------------------------------------------------------------- */
/*                                    Mocks                                   */
/* -------------------------------------------------------------------------- */
class MockExecuteFileCommand : public ForkExecuteFileCommand {
 public:
  std::vector<std::string> commands_executed;
  MockExecuteFileCommand(std::vector<ExecuteFileResult> mock_results)
      : mock_results(mock_results) {};
  ExecuteFileResult execute(const std::string path,
                            std::vector<std::string> args) override;

 private:
  std::vector<ExecuteFileResult> mock_results;
};

ExecuteFileResult MockExecuteFileCommand::execute(
    const std::string path, const std::vector<std::string> args) {
  std::string command = path + " ";

  for (auto iter = args.begin(); iter != args.end() - 1; ++iter) {
    command += *iter + " ";
  }
  command += *(args.end() - 1);

  commands_executed.push_back(command);
  auto result = mock_results.back();
  mock_results.pop_back();
  return result;
}

/* -------------------------------------------------------------------------- */
/*                                    Tests                                   */
/* -------------------------------------------------------------------------- */

/* ---------------------------- findFDupesCommand --------------------------- */
void testFindingFDupes() {
  // Arrange
  MockExecuteFileCommand mock_command_shim{{{0, "/bin/testing"}}};

  // Act
  std::string actual_fdupes_file = findFDupesCommand(mock_command_shim);

  // Assert
  assert(actual_fdupes_file == "/bin/testing");
}

void testFindingFDupesStrips() {
  // Arrange
  MockExecuteFileCommand mock_command_shim{{{0, "/bin/testing       "}}};

  // Act
  std::string actual_fdupes_file = findFDupesCommand(mock_command_shim);

  // Assert
  assert(actual_fdupes_file == "/bin/testing");
}

void testFindingFDupesCallsCorrectCommand() {
  // Arrange
  MockExecuteFileCommand mock_command_shim{{{0, "/tests/fdupes"}}};

  // Act
  findFDupesCommand(mock_command_shim);

  // Assert
  std::vector<std::string> expected_commands{"/bin/which which fdupes"};
  assert(mock_command_shim.commands_executed == expected_commands);
}

void testExceptionOnMissingFDupes() {
  // Arrange
  MockExecuteFileCommand mock_command_shim{{{1, ""}}};

  // Act & Assert
  try {
    findFDupesCommand(mock_command_shim);
    assert(false);
  } catch (NoFDupesException& e) {
    assert(true);
  }
}

/* --------------------------- checkFDupesVersion --------------------------- */
void testExceptionOnFailedFDupesVersionCommand() {
  // Arrange
  MockExecuteFileCommand mock_command_shim{{{1, ""}}};

  // Act & Assert
  try {
    checkFDupesVersion("/bin/fdupes", mock_command_shim);
    assert(false);
  } catch (FDupesVersionException& e) {
    assert(true);
  }
}

void testExceptionOnInvalidVersionCommand() {
  // Arrange
  MockExecuteFileCommand mock_command_shim{{{0, "1.9.0"}}};

  // Act & Assert
  try {
    checkFDupesVersion("/bin/fdupes", mock_command_shim);
    assert(false);
  } catch (InvalidVersionException& e) {
    assert(true);
  }
}

void testCheckingVersionCallsCorrectCommands() {
  // Arrange
  MockExecuteFileCommand mock_command_shim{{{0, "2.0.0"}}};

  // Act
  checkFDupesVersion("/bin/fdupes", mock_command_shim);

  // Assert
  std::vector<std::string> expected_commands{{"/bin/fdupes fdupes --version"}};
  assert(mock_command_shim.commands_executed == expected_commands);
}

/* ------------------------ extractVersionFromString ------------------------ */
void testExtractingVersionFromString() {
  // Arrange
  std::string test_fdupes_output = "1.0.2";

  // Act
  SemVer actual_version_output = extractVersionFromString(test_fdupes_output);

  // Assert
  SemVer expected_version_output = {1, 0, 2};
  assert(actual_version_output == expected_version_output);
}

void testExtractingVersionWithToLittleNumbers() {
  // Arrange
  std::string test_fdupes_output = "1.0";

  // Act & Assert
  try {
    SemVer actual_version_output = extractVersionFromString(test_fdupes_output);
    assert(false);
  } catch (MissingVersionException& e) {
    assert(true);
  }
}

/* ---------------------------------- strip --------------------------------- */

void testStripingAString() {
  // Arrange
  std::string test_string = "    test    ";

  // Act
  std::string actual_string = strip(test_string);

  // Assert
  assert(actual_string == "test");
}

/* ------------------------------ SemVer ------------------------------ */
void testSemVerGreaterThanOrEqualTo() {
  // Arrange
  SemVer test_one{1, 0, 25};
  SemVer test_two{2, 0, 25};

  // Act & Assert
  assert(test_two >= test_one);
}

void testSemVerEquality() {
  // Arrange
  SemVer test_one{1, 20, 25};
  SemVer test_two{1, 20, 25};

  // Act & Assert
  assert(test_two == test_one);
}

/* ------------------------- executeFDupesCacheBuild ------------------------ */

void testExecuteFDupesCallsCorrectCommands() {
  // Arrange
  MockExecuteFileCommand mock_command_shim({{0, ""}});
  RelativePaths test_paths{"testing_dirs/dir1", "testing_dirs/dir2"};

  // Act
  executeFDupesCacheBuild("/bin/fdupes", test_paths, mock_command_shim);

  // Assert
  std::vector<std::string> expected_commands{
      "/bin/fdupes fdupes --cache testing_dirs/dir1 testing_dirs/dir2"};
  assert(mock_command_shim.commands_executed == expected_commands);
}

void testExceptionOnFDupesCacheBuild() {
  // Arrange
  MockExecuteFileCommand mock_command_shim({{1, ""}});
  RelativePaths test_paths{"testing_dirs/dir1"};

  // Act & Assert
  try {
    executeFDupesCacheBuild("/bin/fdupes", test_paths, mock_command_shim);
    assert(false);
  } catch (FailedFDupesCacheBuildException& e) {
    assert(true);
  }
}

/* ---------------------------- verifyCacheExists --------------------------- */
class MockFileExistenceCheck : public FileExistenceCheck {
 public:
  std::vector<std::string> files_checked;
  MockFileExistenceCheck(std::vector<bool> exists_results)
      : exists_results(exists_results) {};

  bool check(const std::string& file_name) override;

 private:
  std::vector<bool> exists_results;
};

bool MockFileExistenceCheck::check(const std::string& file_name) {
  files_checked.push_back(file_name);
  auto result = exists_results.back();
  exists_results.pop_back();
  return result;
}

void testCheckingTheFileForExistence() {
  // Arrange
  MockFileExistenceCheck mock_existence_check{{true}};

  // Act
  verifyCacheExists("/tmp/test.cache", mock_existence_check);

  // Assert
  std::vector<std::string> expected_checked_files{"/tmp/test.cache"};
  assert(mock_existence_check.files_checked == expected_checked_files);
}

void testExceptionOnNonExistingCache() {
  // Arrange
  MockFileExistenceCheck mock_existence_check{{false}};

  // Act & Assert
  try {
    verifyCacheExists("/tmp/test.cache", mock_existence_check);
    assert(false);
  } catch (CacheNotFoundException& e) {
    assert(true);
  }
}

/* ------------------------- loadDataFromSQLiteCache ------------------------ */
template <class T>
struct StepResults {
  bool done;
  T last_row_fetch_result;
};

template <class T>
class MockDatabaseView : public TableView<T> {
 public:
  std::vector<std::string> queries_executed;

  MockDatabaseView(std::vector<StepResults<T>> step_outputs)
      : step_results(step_outputs), queries_executed({}) {};

  void prepare(std::string query) { queries_executed.push_back(query); }

  bool step() {
    auto step_result = step_results.front();
    step_results.erase(step_results.begin());
    TableView<T>::last_row_fetched = step_result.last_row_fetch_result;
    return step_result.done;
  }

 private:
  std::vector<StepResults<T>> step_results;
};

typedef StepResults<DirectoryTableRow> DirectoryTableStep;
typedef StepResults<HashTableRow> HashTableStep;

const std::vector<StepResults<DirectoryTableRow>> mock_directory_view_responses{
    DirectoryTableStep{true, {1, (puchar) "test", 1}},
    DirectoryTableStep{true, {1, (puchar) "test_two", 1}},
    DirectoryTableStep{false, {}}};

const std::vector<StepResults<HashTableRow>> mock_hash_view_responses = {
    HashTableStep{true, {1, (puchar) "test", NULL}},
    HashTableStep{true, {1, (puchar) "test_two", NULL}},
    HashTableStep{false, {}}};

void testLoadDateFromSQLite() {
  // Arrange
  MockDatabaseView<DirectoryTableRow> mock_directory_view{
      mock_directory_view_responses};
  MockDatabaseView<HashTableRow> mock_hash_view{mock_hash_view_responses};

  // Act
  FileHashRows actual_rows = loadDataFromCache(
      (TableView<DirectoryTableRow>* const)&mock_directory_view,
      (TableView<HashTableRow>* const)&mock_hash_view);

  // Assert
  FileHashRows expected_rows{{DirectoryTableRow{1, (puchar) "test", 1},
                              DirectoryTableRow{1, (puchar) "test_two", 1}},
                             {HashTableRow{1, (puchar) "test", NULL},
                              HashTableRow{1, (puchar) "test_two", NULL}}};
  assert(actual_rows == expected_rows);
}

void testLoadDateFromSQLiteCallsCorrectDirectoryQuery() {
  // Arrange
  MockDatabaseView<DirectoryTableRow> mock_directory_view{
      mock_directory_view_responses};
  MockDatabaseView<HashTableRow> mock_hash_view{mock_hash_view_responses};

  // Act
  FileHashRows actual_rows = loadDataFromCache(
      (TableView<DirectoryTableRow>* const)&mock_directory_view,
      (TableView<HashTableRow>* const)&mock_hash_view);

  // Assert
  std::vector<std::string> expected_queries = {"SELECT * FROM Directories;"};
  assert(mock_directory_view.queries_executed == expected_queries);
}

void testLoadDateFromSQLiteCallsCorrectHashQuery() {
  // Arrange
  MockDatabaseView<DirectoryTableRow> mock_directory_view{
      mock_directory_view_responses};
  MockDatabaseView<HashTableRow> mock_hash_view{mock_hash_view_responses};

  // Act
  FileHashRows actual_rows = loadDataFromCache(
      (TableView<DirectoryTableRow>* const)&mock_directory_view,
      (TableView<HashTableRow>* const)&mock_hash_view);

  // Assert
  std::vector<std::string> expected_queries = {"SELECT * FROM Hashes;"};
  assert(mock_hash_view.queries_executed == expected_queries);
}

/* ---------------------------- DirectoryTableRow --------------------------- */
void testDirectoryTableRowEqualOperator() {
  // Arrange
  DirectoryTableRow test_one{1, (puchar) "test", 1};
  DirectoryTableRow test_two{1, (puchar) "test", 1};

  // Act
  bool equality_test = test_one == test_two;

  // Assert
  assert(equality_test);
}

/* ---------------------------- HashTableRow --------------------------- */
void testHashTableRowEqualOperator() {
  // Arrange
  HashTableRow test_one{1, (puchar) "test", NULL};
  HashTableRow test_two{1, (puchar) "test", NULL};

  // Act
  bool equality_test = test_one == test_two;

  // Assert
  assert(equality_test);
}

/* ------------------------------ FileHashRows ------------------------------ */
void testFileHashRowsEqualOperator() {
  // Arrange
  FileHashRows test_one{{DirectoryTableRow{1, (puchar) "test", 1}},
                        {HashTableRow{1, (puchar) "test", NULL}}};
  FileHashRows test_two{{DirectoryTableRow{1, (puchar) "test", 1}},
                        {HashTableRow{1, (puchar) "test", NULL}}};

  // Act
  bool equality_test = test_one == test_two;

  // Assert
  assert(equality_test);
}

/* -------------------------------------------------------------------------- */
/*                                    Main                                    */
/* -------------------------------------------------------------------------- */
int main() {
  testFindingFDupes();
  testFindingFDupesStrips();
  testFindingFDupesCallsCorrectCommand();
  testExceptionOnMissingFDupes();
  testExceptionOnFailedFDupesVersionCommand();
  testExceptionOnInvalidVersionCommand();
  testCheckingVersionCallsCorrectCommands();
  testExtractingVersionFromString();
  testExtractingVersionWithToLittleNumbers();
  testStripingAString();
  testSemVerGreaterThanOrEqualTo();
  testSemVerEquality();
  testExecuteFDupesCallsCorrectCommands();
  testCheckingTheFileForExistence();
  testExceptionOnNonExistingCache();
  testLoadDateFromSQLite();
  testLoadDateFromSQLiteCallsCorrectDirectoryQuery();
  testLoadDateFromSQLiteCallsCorrectHashQuery();
  testDirectoryTableRowEqualOperator();
  testHashTableRowEqualOperator();
  testFileHashRowsEqualOperator();
}