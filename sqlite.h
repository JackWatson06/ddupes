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

struct HashInput {
  unsigned int directory_id;
  std::string name;
  uint8_t* hash;
};

struct DirectoryInput {
  unsigned int parent_id;
  std::string name;
};

class DirectoryTableGateway {
 public:
  DirectoryTableGateway(const SQLiteDatabase& db) : db(db) {}

  unsigned int fetchIdByName(std::string name);
  DirectoryTableRow::Rows fetchAll(void);
  void create(DirectoryInput directory_table_input);

 private:
  const SQLiteDatabase& db;
};

class HashTableGateway {
 public:
  HashTableGateway(const SQLiteDatabase& db) : db(db) {}

  HashTableRow::Rows fetchAll(void);
  void create(HashInput hash_table_input);

 private:
  const SQLiteDatabase& db;
};