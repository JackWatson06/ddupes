#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

class NoFDupesException : std::runtime_error {
  using std::runtime_error::runtime_error;
};
class FDupesVersionException : std::runtime_error {
  using std::runtime_error::runtime_error;
};
class InvalidVersionException : std::runtime_error {
  using std::runtime_error::runtime_error;
};
class MissingVersionException : std::runtime_error {
  using std::runtime_error::runtime_error;
};
class FailedFDupesCacheBuildException : std::runtime_error {
  using std::runtime_error::runtime_error;
};
class CacheNotFoundException : std::runtime_error {
  using std::runtime_error::runtime_error;
};
class CommandExecutionException : std::system_error {
 public:
  using std::system_error::system_error;
};

class SemVer {
 public:
  int major;
  int minor;
  int hotfix;

  bool operator==(const SemVer& rhs);
  bool operator>=(const SemVer& rhs);
};

class ExecuteFileResult {
 public:
  int return_code;
  std::string output;
};

// We could probably just pass in a callback. Instead of wrapping a function
// with a class.
class ForkExecuteFileCommand {
 public:
  virtual ExecuteFileResult execute(const std::string path,
                                    const std::vector<std::string> args);
};

class FileExistenceCheck {
 public:
  virtual bool check(const std::string& file_name);
};

typedef std::vector<std::string> RelativePaths;

std::string findFDupesCommand(ForkExecuteFileCommand& executor);
void checkFDupesVersion(std::string fdupes_file,
                        ForkExecuteFileCommand& executor);
std::string strip(const std::string& input);
SemVer extractVersionFromString(std::string& version_string);
void executeFDupesCacheBuild(std::string fdupes_file,
                             RelativePaths paths_to_check,
                             ForkExecuteFileCommand& executor);
void verifyCacheExists(const std::string& file_name,
                       FileExistenceCheck& checker);
void extract(const RelativePaths& paths);