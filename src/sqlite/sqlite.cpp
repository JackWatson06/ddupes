#include "sqlite.h"

#include <cstring>

#include "../constants.h"

/* -------------------------------------------------------------------------- */
/*                                  Database                                  */
/* -------------------------------------------------------------------------- */
sqlite3* initDB(std::string file) {
  sqlite3* db;
  int rc = sqlite3_open(file.c_str(), &db);

  if (rc != SQLITE_OK) {
    throw UnableToConnectError("Unable to conenct to the database.");
  }

  return db;
}

void resetDB(sqlite3* db) {
  // TODO Error loggin
  const char* delete_all_directories_stmt = "DELETE FROM Directories;";
  const char* delete_all_hashes_stmt = "DELETE FROM Hashes;";
  const char* reset_increments_stmt =
      "DELETE FROM sqlite_sequence WHERE name='Directories';";

  int truncate_directories_result =
      sqlite3_exec(db, delete_all_directories_stmt, 0, 0, 0);
  int reset_autocomplete_result =
      sqlite3_exec(db, reset_increments_stmt, 0, 0, 0);

  if (truncate_directories_result != SQLITE_OK) {
    const char* create_directories_table_ddl =
        "CREATE TABLE Directories (id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT NOT NULL, parent_id INTEGER NOT NULL );";

    int create_directories_result =
        sqlite3_exec(db, create_directories_table_ddl, 0, 0, 0);

    if (create_directories_result != SQLITE_OK) {
      throw UnableToCreateTableError("Could not create the Directories table.");
    }
  }

  int truncate_hashes_result =
      sqlite3_exec(db, delete_all_hashes_stmt, 0, 0, 0);

  if (truncate_hashes_result != SQLITE_OK) {
    const char* create_hash_table_ddl =
        "CREATE TABLE Hashes (directory_id INTEGER NOT NULL, "
        "name TEXT NOT NULL, hash BLOB NOT NULL );";

    int create_hashes_result = sqlite3_exec(db, create_hash_table_ddl, 0, 0, 0);

    if (create_hashes_result) {
      throw UnableToCreateTableError("Could not create the Hashes table.");
    }
  }
}

void freeDB(sqlite3* db) { sqlite3_close(db); }

/* -------------------------------------------------------------------------- */
/*                               Table Gateways                               */
/* -------------------------------------------------------------------------- */
bool compareBlobs(const uint8_t* blob_one, const uint8_t* blob_two) {
  if (blob_one == nullptr || blob_two == nullptr) {
    return blob_one == blob_two;
  }

  for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
    if (blob_one[i] != blob_two[i]) {
      return false;
    }
  }

  return true;
}

bool HashTableRow::operator==(const HashTableRow& rhs) const {
  return rhs.directory_id == directory_id && rhs.name == name &&
         compareBlobs(hash, rhs.hash);
};

HashTableRow::HashTableRow(const HashTableRow& other) {
  directory_id = other.directory_id;
  name = other.name;

  if (other.hash != nullptr) {
    uint8_t* blob_buffer = new uint8_t[MD5_DIGEST_LENGTH];
    std::memcpy(blob_buffer, other.hash, MD5_DIGEST_LENGTH);

    hash = blob_buffer;
  } else {
    hash = nullptr;
  }
}

HashTableRow& HashTableRow::operator=(const HashTableRow& other) {
  if (this == &other) return *this;

  delete[] hash;

  directory_id = other.directory_id;
  name = other.name;

  if (other.hash != nullptr) {
    hash = new uint8_t[MD5_DIGEST_LENGTH];
    std::memcpy(hash, other.hash, MD5_DIGEST_LENGTH);
  } else {
    hash = nullptr;
  }
  return *this;
}

bool DirectoryTableRow::operator==(const DirectoryTableRow& rhs) const {
  return rhs.id == id && rhs.name == name && rhs.parent_id == parent_id;
};

bool HashInput::operator==(const HashInput& rhs) const {
  return rhs.directory_id == directory_id && rhs.name == name &&
         compareBlobs(hash, rhs.hash);
}

bool DirectoryInput::operator==(const DirectoryInput& rhs) const {
  return rhs.parent_id == parent_id && rhs.name == name;
}

int fetchLastDirectoryId(sqlite3* db) {
  sqlite3_stmt* statement;
  // Select last order by id.
  int rc = sqlite3_prepare_v2(
      db, "SELECT id FROM Directories ORDER BY id DESC LIMIT 1", -1, &statement,
      0);

  if (rc != SQLITE_OK) {
    throw UnableToBuildStatementError(
        "Could not build the select statement in 'fetchLastDirectoryId'.");
  }

  int step = sqlite3_step(statement);

  if (step == SQLITE_ROW) {
    int id = sqlite3_column_int(statement, 0);
    sqlite3_finalize(statement);
    return id;
  }

  sqlite3_finalize(statement);
  if (step == SQLITE_ERROR) {
    throw UnableToStepError(
        "Could not step while building the select statement in "
        "'fetchDirectoryIdByName'.");
  }
  return -1;
}

DirectoryTableRow::Rows fetchAllDirectories(sqlite3* db) {
  DirectoryTableRow::Rows results{};

  sqlite3_stmt* statement;
  int rc =
      sqlite3_prepare_v2(db, "SELECT * FROM Directories;", -1, &statement, 0);

  if (rc != SQLITE_OK) {
    throw UnableToBuildStatementError(
        "Could not build the select statement in 'fetchAllDirectories'");
  }

  while (sqlite3_step(statement) != SQLITE_DONE) {
    results.push_back(DirectoryTableRow{
        .id = sqlite3_column_int(statement, 0),
        .name = std::string((const char*)sqlite3_column_text(statement, 1)),
        .parent_id = sqlite3_column_int(statement, 2)});
  }

  sqlite3_finalize(statement);
  return results;
}

int createDirectory(sqlite3* db, DirectoryInput& directory_table_input) {
  sqlite3_stmt* statement;
  int rc = sqlite3_prepare_v2(
      db, "INSERT INTO Directories (name, parent_id) VALUES(?, ?);", -1,
      &statement, 0);

  if (rc == SQLITE_OK) {
    sqlite3_bind_text(statement, 1, directory_table_input.name.c_str(), -1, 0);
    sqlite3_bind_int(statement, 2, directory_table_input.parent_id);
  } else {
    throw UnableToBuildStatementError(
        "Could not build the insert statement in 'createDirectory'.");
  }

  int step = sqlite3_step(statement);
  sqlite3_finalize(statement);

  if (step == SQLITE_DONE) {
    return fetchLastDirectoryId(db);
  }

  throw UnableToInsertError("Could not insert in 'createDirectory'");
}

HashTableRow::Rows fetchAllHashes(sqlite3* db) {
  HashTableRow::Rows results{};

  sqlite3_stmt* statement;
  int rc = sqlite3_prepare_v2(db, "SELECT * FROM Hashes;", -1, &statement, 0);

  if (rc != SQLITE_OK) {
    throw UnableToBuildStatementError(
        "Could not build the select statement in 'fetchAllHashes'");
  }

  while (sqlite3_step(statement) != SQLITE_DONE) {
    uint8_t* hash_blob = (uint8_t*)sqlite3_column_blob(statement, 2);
    uint8_t* hash_buffer = new uint8_t[MD5_DIGEST_LENGTH];
    std::memcpy(hash_buffer, hash_blob, MD5_DIGEST_LENGTH);

    results.push_back(HashTableRow{
        sqlite3_column_int(statement, 0),
        std::string((const char*)sqlite3_column_text(statement, 1)),
        hash_buffer});
  }

  sqlite3_finalize(statement);
  return results;
}

void createHash(sqlite3* db, HashInput& hash_table_input) {
  sqlite3_stmt* statement;
  int rc = sqlite3_prepare_v2(db, "INSERT INTO Hashes VALUES(?, ?, ?);", -1,
                              &statement, 0);

  if (rc == SQLITE_OK) {
    sqlite3_bind_int(statement, 1, hash_table_input.directory_id);
    sqlite3_bind_text(statement, 2, hash_table_input.name.c_str(), -1, 0);
    sqlite3_bind_blob(statement, 3, hash_table_input.hash, MD5_DIGEST_LENGTH,
                      0);
  } else {
    throw UnableToBuildStatementError(
        "Could not build the insert statement in 'createHashes'.");
  }

  int step = sqlite3_step(statement);
  sqlite3_finalize(statement);

  if (step == SQLITE_DONE) {
    return;
  }

  throw UnableToInsertError("Could not insert in 'createHashes'");
}
