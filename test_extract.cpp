#include <unistd.h>

#include <cassert>

#include "extract.h"

class MockExecuteFileCommand : public ExecuteFileCommand {
 public:
  MockExecuteFileCommand(int return_code) : return_code(return_code) {};
  int execute(std::string command) override;

 private:
  int return_code;
  std::vector<std::string> commands_executed;
};

int MockExecuteFileCommand::execute(std::string command) {
  commands_executed.push_back(command);
  return return_code;
}

void testVerifyingFDupesCallsTheCorrectCommand() {
  // Arrange
  bool called_exception = false;
  MockExecuteFileCommand mock_command_shim{0};

  // Act
  validateFDupesDependency(mock_command_shim);

  // Assert
  assert(mock_command_shim.commands_executed ==
         {"which fdupes > /dev/null 2>&1", "fdupes --version"});
}

void testVeryifyingFDupes() {
  // Arrange
  bool called_exception = false;
  MockExecuteFileCommand mock_command{1};

  // Act
  try {
    vlidateFDupesDependency(mock_command{);
      called_exception = true;
  } catch (DependencyException e) {
      called_exception = false;
  }

  // Assert
  assert(called_exception == true);
  }

  void testBuildingTheFileCache() {
    // Arrange
    RelativePaths paths{"./testing_dirs/dir1", "./testing_dirs/dir2"};

    buildFileCache("./testing_dirs/dir1", "./testing_dirs/dir2");

    // Test if the file cache exists where it get's stored in fdupes.
  }
