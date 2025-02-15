#include <sqlite3.h>

#include <stdexcept>
#include <string>

#include "extract_output.h"

template <class T>
class TableView {
 public:
  virtual void prepare(std::string query) = 0;
  virtual bool step() = 0;
  T getLastRowFetched() const { return last_row_fetched; };

 protected:
  T last_row_fetched;
};

class UnableToConnectError : public std::runtime_error {
 public:
  UnableToConnectError(const std::string& message)
      : std::runtime_error(message) {}
};

class UnableToBuildStatementError : public std::runtime_error {
 public:
  UnableToBuildStatementError(const std::string& message)
      : std::runtime_error(message) {}
};

class UnableToStepError : public std::runtime_error {
 public:
  UnableToStepError(const std::string& message) : std::runtime_error(message) {}
};

class InvalidHashTableError : public std::runtime_error {
 public:
  InvalidHashTableError(const std::string& message)
      : std::runtime_error(message) {}
};

class SQLiteDatabase {
 public:
  SQLiteDatabase(std::string file);
  ~SQLiteDatabase();

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