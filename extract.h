#include <sqlite3.h>

#include <stdexcept>
#include <system_error>

#include "extract_output.h"

typedef std::vector<std::string> RelativePaths;

/* -------------------------------------------------------------------------- */
/*                            File Executor Service                           */
/* -------------------------------------------------------------------------- */
class NoFDupesException : std::runtime_error {
  using std::runtime_error::runtime_error;
};
class FDupesVersionException : std::runtime_error {
  using std::runtime_error::runtime_error;
};
class InvalidVersionException : std::runtime_error {
  using std::runtime_error::runtime_error;
};
class MissingVersionException : public std::runtime_error {
  using std::runtime_error::runtime_error;
};
class FailedFDupesCacheBuildException : public std::runtime_error {
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

/* -------------------------------------------------------------------------- */
/*                           File Existence Service                           */
/* -------------------------------------------------------------------------- */
class FileExistenceCheck {
 public:
  virtual bool check(const std::string& file_name);
};

/* -------------------------------------------------------------------------- */
/*                               SQLite Service                               */
/* -------------------------------------------------------------------------- */
template <class T>
class TableView {
 public:
  virtual void prepare(std::string query) = 0;
  virtual bool step() = 0;
  T getLastRowFetched() const { return last_row_fetched; };

 protected:
  T last_row_fetched;
};

class UnableToConnectError : std::runtime_error {
  using std::runtime_error::runtime_error;
};

class UnableToBuildStatementError : public std::runtime_error {
 public:
  UnableToBuildStatementError(const std::string& message)
      : std::runtime_error(message) {}
};

class UnableToStepError : std::runtime_error {
  using std::runtime_error::runtime_error;
};

class SQLiteDatabase {
 public:
  SQLiteDatabase(std::string file);

  ~SQLiteDatabase() { sqlite3_close(db); };

  sqlite3* getDb() const { return db; }

 private:
  sqlite3* db;
};

template <class T>
class SQLiteTableView : public TableView<T> {
 public:
  SQLiteTableView(const SQLiteDatabase& db) : db(db) {}
  ~SQLiteTableView() {
    if (res != nullptr) {
      sqlite3_finalize(res);
    }
  }

  virtual T buildTableRow() = 0;

  void prepare(std::string query);
  bool step();

 protected:
  const SQLiteDatabase& db;
  sqlite3_stmt* res = nullptr;
};

class DirectoryTableView : public SQLiteTableView<DirectoryTableRow> {
 public:
  using SQLiteTableView<DirectoryTableRow>::SQLiteTableView;

  DirectoryTableRow buildTableRow();
};

class HashTableView : public SQLiteTableView<HashTableRow> {
 public:
  using SQLiteTableView<HashTableRow>::SQLiteTableView;

  HashTableRow buildTableRow();
};

/* -------------------------------------------------------------------------- */
/*                              Process Functions                             */
/* -------------------------------------------------------------------------- */
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
FileHashRows loadDataFromCache(
    TableView<DirectoryTableRow>* const directory_table_view,
    TableView<HashTableRow>* const hash_table_view);

FileHashRows extract(const RelativePaths& paths,
                     const std::string cache_directory);