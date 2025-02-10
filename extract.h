#include <string>
#include <vector>

class ExecuteFileCommand {
 public:
  virtual int execute(std::string command);
};

typedef std::vector<std::string> RelativePaths;

void validateFDupesDependency(ExecuteFileCommand execute_file_command);
void buildFileCache(const RelativePaths& paths);
