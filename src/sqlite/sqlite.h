#pragma once

#include <sqlite3.h>

#include <stdexcept>
#include <string>
#include <vector>

/* -------------------------------------------------------------------------- */
/*                                  Database                                  */
/* -------------------------------------------------------------------------- */
sqlite3* initDB(std::string file_name);
void resetDB(sqlite3* db);
void freeDB(sqlite3* db_handle);

/* -------------------------------------------------------------------------- */
/*                               Table Gateways                               */
/* -------------------------------------------------------------------------- */

struct DirectoryTableRow {
  typedef std::vector<DirectoryTableRow> Rows;

  int id;
  std::string name;
  int parent_id;

  bool operator==(const DirectoryTableRow& rhs) const;
};

struct HashTableRow {
  typedef std::vector<HashTableRow> Rows;
  int directory_id;
  std::string name;
  uint8_t* hash;

  ~HashTableRow() {
    if (hash != nullptr) {
      delete[] hash;
    }
  }
  HashTableRow(int directory_id, std::string name, uint8_t* hash)
      : directory_id(directory_id), name(name), hash(hash) {}
  HashTableRow() : directory_id(-1), name(""), hash(nullptr) {}
  HashTableRow(const HashTableRow& other);
  HashTableRow& operator=(const HashTableRow& other);

  bool operator==(const HashTableRow& rhs) const;
};

struct HashInput {
  int directory_id;
  std::string name;
  uint8_t* hash;

  bool operator==(const HashInput& rhs) const;
};

struct DirectoryInput {
  int parent_id;
  std::string name;

  bool operator==(const DirectoryInput& rhs) const;
};

bool compareBlobs(const uint8_t* blob_one, const uint8_t* blob_two);

int fetchLastDirectoryId(sqlite3* db);
DirectoryTableRow::Rows fetchAllDirectories(sqlite3* db);
int createDirectory(sqlite3* db, DirectoryInput& directory_table_input);

HashTableRow::Rows fetchAllHashes(sqlite3* db);
void createHash(sqlite3* db, HashInput& hash_table_input);

/* -------------------------------------------------------------------------- */
/*                                   Errors                                   */
/* -------------------------------------------------------------------------- */
class UnableToConnectError : public std::runtime_error {
 public:
  UnableToConnectError(const std::string& message)
      : std::runtime_error(message) {}
};

class UnableToCreateTableError : public std::runtime_error {
 public:
  UnableToCreateTableError(const std::string& message)
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

class UnableToInsertError : public std::runtime_error {
 public:
  UnableToInsertError(const std::string& message)
      : std::runtime_error(message) {}
};

class NotFoundError : public std::runtime_error {
 public:
  NotFoundError(const std::string& message) : std::runtime_error(message) {}
};
