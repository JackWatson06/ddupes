#include "sqlite.h"

#include <cstring>

/* -------------------------------------------------------------------------- */
/*                                  Database                                  */
/* -------------------------------------------------------------------------- */
sqlite3 *initDB(str_const file) {
  sqlite3 *db;
  int rc = sqlite3_open(file, &db);

  if (rc != SQLITE_OK) {
    throw unable_to_connect_error("Unable to conenct to the database.");
  }

  return db;
}

/**
 * TODO: Error logging.
 */
void resetDB(sqlite3 *db) {
  // Reset or create Directories table.
  const char *delete_all_directories_stmt = "DELETE FROM Directories;";
  const char *reset_directory_increments_stmt =
      "DELETE FROM sqlite_sequence WHERE name='Directories';";

  int truncate_directories_result =
      sqlite3_exec(db, delete_all_directories_stmt, 0, 0, 0);
  sqlite3_exec(db, reset_directory_increments_stmt, 0, 0, 0);

  if (truncate_directories_result != SQLITE_OK) {
    const char *create_directories_table_ddl =
        "CREATE TABLE Directories (id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT NOT NULL, parent_id INTEGER NOT NULL );";

    int create_directories_result =
        sqlite3_exec(db, create_directories_table_ddl, 0, 0, 0);

    if (create_directories_result != SQLITE_OK) {
      throw unable_to_create_table_error(
          "Could not create the Directories table.");
    }
  }

  // Reset or create Hashes table.
  const char *delete_all_hashes_stmt = "DELETE FROM Hashes;";
  const char *reset_hash_increments_stmt =
      "DELETE FROM sqlite_sequence WHERE name='Hashes';";

  int truncate_hashes_result =
      sqlite3_exec(db, delete_all_hashes_stmt, 0, 0, 0);
  sqlite3_exec(db, reset_hash_increments_stmt, 0, 0, 0);
  if (truncate_hashes_result != SQLITE_OK) {
    const char *create_hash_table_ddl =
        "CREATE TABLE Hashes (id INTEGER PRIMARY KEY "
        "AUTOINCREMENT, directory_id INTEGER NOT NULL, "
        "name TEXT NOT NULL, hash BLOB NOT NULL );";

    int create_hashes_result = sqlite3_exec(db, create_hash_table_ddl, 0, 0, 0);

    if (create_hashes_result) {
      throw unable_to_create_table_error("Could not create the Hashes table.");
    }
  }

  // Reset or create ScanMetaData table.
  const char *delete_all_scan_meta_data_stmt = "DELETE FROM ScanMetaData;";
  int truncate_scan_meta_data_result =
      sqlite3_exec(db, delete_all_scan_meta_data_stmt, 0, 0, 0);

  if (truncate_scan_meta_data_result != SQLITE_OK) {
    const char *create_scan_meta_data_ddl =
        "CREATE TABLE ScanMetaData (root_dir TEXT NOT NULL);";

    int create_scan_meta_data_result =
        sqlite3_exec(db, create_scan_meta_data_ddl, 0, 0, 0);

    if (create_scan_meta_data_result) {
      throw unable_to_create_table_error(
          "Could not create the ScanMetaData table.");
    }
  }
}

void freeDB(sqlite3 *db) { sqlite3_close(db); }

/* -------------------------------------------------------------------------- */
/*                               Table Gateways                               */
/* -------------------------------------------------------------------------- */
int fetchLastDirectoryId(sqlite3 *db) {
  sqlite3_stmt *statement;
  // Select last order by id.
  int rc = sqlite3_prepare_v2(
      db, "SELECT id FROM Directories ORDER BY id DESC LIMIT 1", -1, &statement,
      0);

  if (rc != SQLITE_OK) {
    throw unable_to_build_statement_error(
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
    throw unable_to_step_error(
        "Could not step while building the select statement in "
        "'fetchDirectoryIdByName'.");
  }
  return -1;
}

directory_table_row::rows fetchAllDirectories(sqlite3 *db) {
  directory_table_row::rows results{};

  sqlite3_stmt *statement;
  int rc =
      sqlite3_prepare_v2(db, "SELECT * FROM Directories;", -1, &statement, 0);

  if (rc != SQLITE_OK) {
    throw unable_to_build_statement_error(
        "Could not build the select statement in 'fetchAllDirectories'");
  }

  while (sqlite3_step(statement) != SQLITE_DONE) {
    results.push_back(directory_table_row{
        .id = sqlite3_column_int(statement, 0),
        .name = stringDup((const char *)sqlite3_column_text(statement, 1)),
        .parent_id = sqlite3_column_int(statement, 2)});
  }

  sqlite3_finalize(statement);
  return results;
}

int createDirectory(sqlite3 *db, directory_input const &directory_table_input) {
  sqlite3_stmt *statement;
  int rc = sqlite3_prepare_v2(
      db, "INSERT INTO Directories (name, parent_id) VALUES(?, ?);", -1,
      &statement, 0);

  if (rc == SQLITE_OK) {
    sqlite3_bind_text(statement, 1, directory_table_input.name, -1, 0);
    sqlite3_bind_int(statement, 2, directory_table_input.parent_id);
  } else {
    throw unable_to_build_statement_error(
        "Could not build the insert statement in 'createDirectory'.");
  }

  int step = sqlite3_step(statement);
  sqlite3_finalize(statement);

  if (step == SQLITE_DONE) {
    return fetchLastDirectoryId(db);
  }

  throw unable_to_insert_error("Could not insert in 'createDirectory'");
}

int fetchLastHashId(sqlite3 *db) {
  sqlite3_stmt *statement;
  // Select last order by id.
  int rc = sqlite3_prepare_v2(
      db, "SELECT id FROM Hashes ORDER BY id DESC LIMIT 1", -1, &statement, 0);

  if (rc != SQLITE_OK) {
    throw unable_to_build_statement_error(
        "Could not build the select statement in 'fetchLastHashId'.");
  }

  int step = sqlite3_step(statement);

  if (step == SQLITE_ROW) {
    int id = sqlite3_column_int(statement, 0);
    sqlite3_finalize(statement);
    return id;
  }

  sqlite3_finalize(statement);
  if (step == SQLITE_ERROR) {
    throw unable_to_step_error(
        "Could not step while building the select statement in "
        "'fetchLastHashId'.");
  }
  return -1;
}

hash_table_row::rows fetchAllHashes(sqlite3 *db) {
  hash_table_row::rows results{};

  sqlite3_stmt *statement;
  int rc = sqlite3_prepare_v2(db, "SELECT * FROM Hashes;", -1, &statement, 0);

  if (rc != SQLITE_OK) {
    throw unable_to_build_statement_error(
        "Could not build the select statement in 'fetchAllHashes'");
  }

  while (sqlite3_step(statement) != SQLITE_DONE) {
    uint8_t *hash_blob = (uint8_t *)sqlite3_column_blob(statement, 3);
    uint8_t *hash_buffer = new uint8_t[MD5_DIGEST_LENGTH];
    std::memcpy(hash_buffer, hash_blob, MD5_DIGEST_LENGTH);

    results.push_back(hash_table_row{
        sqlite3_column_int(statement, 0), sqlite3_column_int(statement, 1),
        stringDup((const char *)sqlite3_column_text(statement, 2)),
        hash_buffer});
  }

  sqlite3_finalize(statement);
  return results;
}

int createHash(sqlite3 *db, hash_input const &hash_table_input) {
  sqlite3_stmt *statement;
  int rc = sqlite3_prepare_v2(
      db, "INSERT INTO Hashes (directory_id, name, hash) VALUES(?, ?, ?);", -1,
      &statement, 0);

  if (rc == SQLITE_OK) {
    sqlite3_bind_int(statement, 1, hash_table_input.directory_id);
    sqlite3_bind_text(statement, 2, hash_table_input.name, -1, 0);
    sqlite3_bind_blob(statement, 3, hash_table_input.hash, MD5_DIGEST_LENGTH,
                      0);
  } else {
    throw unable_to_build_statement_error(
        "Could not build the insert statement in 'createHashes'.");
  }

  int step = sqlite3_step(statement);
  sqlite3_finalize(statement);

  if (step == SQLITE_DONE) {
    return fetchLastHashId(db);
  }

  throw unable_to_insert_error("Could not insert in 'createHashes'");
}

void deleteHash(sqlite3 *db, int id) {
  sqlite3_stmt *statement;
  int rc = sqlite3_prepare_v2(db, "DELETE FROM Hashes WHERE id = ?;", -1,
                              &statement, 0);

  if (rc == SQLITE_OK) {
    sqlite3_bind_int(statement, 1, id);
  } else {
    throw unable_to_build_statement_error(
        "Could not build the delete statement in 'deleteHash'.");
  }

  int step = sqlite3_step(statement);
  sqlite3_finalize(statement);

  if (step == SQLITE_DONE) {
    return;
  }

  throw unable_to_delete_error("Could not delete in 'deleteHash'");
}

scan_meta_data_table_row fetchScanMetaData(sqlite3 *db) {
  sqlite3_stmt *statement;

  int rc = sqlite3_prepare_v2(db, "SELECT * FROM ScanMetaData LIMIT 1", -1,
                              &statement, 0);

  if (rc != SQLITE_OK) {
    throw unable_to_build_statement_error(
        "Could not build the select statement in 'fetchScanMetaData'.");
  }

  int step = sqlite3_step(statement);

  if (step == SQLITE_ROW) {
    scan_meta_data_table_row row{
        .root_dir = stringDup((const char *)sqlite3_column_text(statement, 0))};

    sqlite3_finalize(statement);
    return row;
  }

  sqlite3_finalize(statement);
  if (step == SQLITE_ERROR) {
    throw unable_to_step_error(
        "Could not step while building the select statement in "
        "'fetchScanMetaData'.");
  }
  throw not_found_error("Could not find a row in 'fetchScanMetaData'.");
  return {};
}

void createScanMetaData(sqlite3 *db,
                        scan_meta_data_input const &scan_meta_data_input) {
  int rc = sqlite3_exec(db, "DELETE FROM ScanMetaData;", 0, 0, 0);

  if (rc != SQLITE_OK) {
    throw unable_to_delete_error(
        "Could not delete all in 'createScanMetaData'");
  }

  sqlite3_stmt *insert_statement;
  rc = sqlite3_prepare_v2(db, "INSERT INTO ScanMetaData (root_dir) VALUES(?);",
                          -1, &insert_statement, 0);

  if (rc == SQLITE_OK) {
    sqlite3_bind_text(insert_statement, 1, scan_meta_data_input.root_dir, -1,
                      0);
  } else {
    throw unable_to_build_statement_error(
        "Could not build the insert statement in 'createScanMetaData'.");
  }

  int step = sqlite3_step(insert_statement);
  sqlite3_finalize(insert_statement);

  if (step == SQLITE_DONE) {
    return;
  }

  throw unable_to_insert_error("Could not insert in 'createScanMetaData'");
}