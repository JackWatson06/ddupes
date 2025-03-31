#include <stdexcept>
#include <system_error>

#include "./fs/file_system.h"
#include "extract_output.h"

typedef std::vector<std::string> string_vector;

struct create_cache_file_system_service {
  std::string (*qualifyRelativePath)(std::string &relative_path);
  void (*visitFiles)(const std::string &directory_path,
                     file_visitor_callback visitor_callback, void *context);
  void (*extractHash)(uint8_t *hash, std::string path);
};

struct create_cache_database_service {
  sqlite3 *db;
  void (*resetDB)(sqlite3 *db);
  int (*createDirectory)(sqlite3 *db,
                         const directory_input &directory_table_input);
  void (*createHash)(sqlite3 *db, const hash_input &hash_table_input);
};

struct extract_database_service {
  sqlite3 *db;
  directory_table_row::rows (*fetchAllDirectories)(sqlite3 *db);
  hash_table_row::rows (*fetchAllHashes)(sqlite3 *db);
};

struct argument_path {
  std::string relative_path;
  string_vector canonicalized_path_tokens;

  bool operator==(const argument_path &rhs) const;
};

struct root_calc_result {
  std::string root_path;
  std::vector<argument_path> argument_paths;

  bool operator==(const root_calc_result &rhs) const;
};

/* -------------------------------------------------------------------------- */
/*                              Process Functions                             */
/* -------------------------------------------------------------------------- */

string_vector tokenizeRootPath(std::string &path);
string_vector tokenizeRelativePath(std::string &path);
int countShortestTokenizePath(std::vector<string_vector> &argument_paths);
std::string removeLeadingRelativePath(const std::string &leading_relative_path,
                                      const std::string &path);
root_calc_result calcRootPath(string_vector &argument_paths,
                              create_cache_file_system_service fs_services);

void buildCache(string_vector &paths, create_cache_database_service db_services,
                create_cache_file_system_service fs_services);
file_hash_rows extractUsingCache(extract_database_service db_services);