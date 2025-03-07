#include <stdexcept>
#include <system_error>

#include "./fs/file_system.h"
#include "extract_output.h"

typedef std::vector<std::string> SVector;

struct CreateCacheFileSystemServices {
  std::string (*qualifyRelativePath)(std::string& relative_path);
  void (*visitFiles)(const std::string& directory_path,
                     FileVisitorCallback visitor_callback, void* context);
  void (*extractHash)(uint8_t* hash, std::string path);
};

struct CreateCacheDatabaseServices {
  sqlite3* db;
  void (*resetDB)(sqlite3* db);
  int (*createDirectory)(sqlite3* db,
                         const DirectoryInput& directory_table_input);
  void (*createHash)(sqlite3* db, const HashInput& hash_table_input);
};

struct ExtractDatabaseServices {
  sqlite3* db;
  DirectoryTableRow::Rows (*fetchAllDirectories)(sqlite3* db);
  HashTableRow::Rows (*fetchAllHashes)(sqlite3* db);
};

struct ArgumentPath {
  std::string relative_path;
  SVector canonicalized_path_tokens;

  bool operator==(const ArgumentPath& rhs) const;
};

struct RootCalcResult {
  std::string root_path;
  std::vector<ArgumentPath> argument_paths;

  bool operator==(const RootCalcResult& rhs) const;
};

/* -------------------------------------------------------------------------- */
/*                              Process Functions                             */
/* -------------------------------------------------------------------------- */

SVector tokenizeRootPath(std::string& path);
SVector tokenizeRelativePath(std::string& path);
int countShortestTokenizePath(std::vector<SVector>& argument_paths);
std::string removeLeadingRelativePath(const std::string& leading_relative_path,
                                      const std::string& path);
RootCalcResult calcRootPath(SVector& argument_paths,
                            CreateCacheFileSystemServices fs_services);

void buildCache(SVector& paths, CreateCacheDatabaseServices db_services,
                CreateCacheFileSystemServices fs_services);
FileHashRows extractUsingCache(ExtractDatabaseServices db_services);