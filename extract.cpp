#include "extract.h"

#include <sys/stat.h>
#include <sys/wait.h>

#include <cerrno>
#include <regex>

constexpr SemVer FDUPES_VERSION{2, 0, 0};

/**
 * Notes:
 *
 * - Step 1: Validate the file paths passedin are real.
 * - Step 2: Setup the Database Tables required to create the cache.
 * - Step 3:Recusively loop through each directory and the files. Taking a hash
 * of each file (I could probably also take a hash of each folder during this
 * process as well?)
 * - Step 4: Save the data to the database when stepping through.
 * -
 *
 *
 */

/* -------------------------------------------------------------------------- */
/*                            File Hasing Service */
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/*                           File Existence Service                           */
/* -------------------------------------------------------------------------- */
bool FileExistenceCheck::check(const std::string& file_name) {
  struct stat buffer;
  return (stat(file_name.c_str(), &buffer) == 0);
}

/* -------------------------------------------------------------------------- */
/*                              Process Functions                             */
/* -------------------------------------------------------------------------- */

void verifyCacheExists(const std::string& file_name,
                       FileExistenceCheck& checker) {
  bool file_exists = checker.check(file_name);

  if (!file_exists) {
    throw CacheNotFoundException("The file cache passed in was not found!");
  }
}

FileHashRows loadDataFromCache(
    TableView<DirectoryTableRow>* const directory_table_view,
    TableView<HashTableRow>* const hash_table_view) {
  DirectoryTableRow::Rows directory_table_rows{};
  HashTableRow::Rows hash_table_rows{};

  directory_table_view->prepare("SELECT * FROM Directories;");
  hash_table_view->prepare("SELECT * FROM Hashes;");

  while (directory_table_view->step()) {
    directory_table_rows.push_back(directory_table_view->getLastRowFetched());
  }

  while (hash_table_view->step()) {
    hash_table_rows.push_back(hash_table_view->getLastRowFetched());
  }

  return {directory_table_rows, hash_table_rows};
};

FileHashRows extract(const RelativePaths& paths, const int flags) {
  // 1-bit => reset cache.
  // 2-bit => load cache.

  // Check if the cache file exists.
  FileExistenceCheck checker;
  verifyCacheExists(cache_directory, checker);

  // Load the data from the SQL database.
  SQLiteDatabase sqlite{cache_directory};
  DirectoryTableView directory_table_view{sqlite};
  HashTableView hash_table_view{sqlite};

  return loadDataFromCache(
      (TableView<DirectoryTableRow>* const)&directory_table_view,
      (TableView<HashTableRow>* const)&hash_table_view);
}
