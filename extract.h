#include <stdexcept>
#include <system_error>

#include "sqlite.h"

typedef std::vector<std::string> RelativePaths;

/* -------------------------------------------------------------------------- */
/*                           File Existence Service                           */
/* -------------------------------------------------------------------------- */
class FileExistenceCheck {
 public:
  virtual bool check(const std::string& file_name);
};

/* -------------------------------------------------------------------------- */
/*                              Process Functions                             */
/* -------------------------------------------------------------------------- */
void verifyCacheExists(const std::string& file_name,
                       FileExistenceCheck& checker);
FileHashRows loadDataFromCache(
    TableView<DirectoryTableRow>* const directory_table_view,
    TableView<HashTableRow>* const hash_table_view);

FileHashRows extract(const RelativePaths& paths,
                     const std::string cache_directory);