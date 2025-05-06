
#include <cassert>
#include <filesystem>

#include "../../src/lib.cpp"
#include "../../src/sqlite/operators.cpp"
#include "../../src/sqlite/sqlite.cpp"
#include "../data.cpp"

// TODO: Test with spaces.
/* ----------------------------- SQLiteDatabase ----------------------------- */
void testConnectingToDb() {
  try {
    // Act
    sqlite3 *db = initDB("tests/test_hash.db");

    // Assert
    assert(true);

    // Cleanup
    freeDB(db);
  } catch (unable_to_connect_error &e) {
    assert(false);
  }
}

void testResetingDatabase() {
  // Arrange
  str_const test_db = "tests/test_reset_hash.db";
  sqlite3 *db = initDB(test_db);

  // Act
  resetDB(db);

  // Assert
  int directories_code =
      sqlite3_exec(db, "SELECT * FROM Directories;", 0, 0, 0);
  int hashes_code = sqlite3_exec(db, "SELECT * FROM Hashes;", 0, 0, 0);
  int scan_meta_data_code =
      sqlite3_exec(db, "SELECT * FROM ScanMetaData;", 0, 0, 0);

  assert(directories_code == SQLITE_OK && hashes_code == SQLITE_OK &&
         scan_meta_data_code == SQLITE_OK);

  // Cleanup
  freeDB(db);
  std::filesystem::remove(test_db);
}

void testResetingTables() {
  // Arrange
  str_const test_db = "tests/test_reset_hash.db";
  sqlite3 *db = initDB(test_db);
  resetDB(db);
  sqlite3_exec(db, "INSERT INTO Directories VALUES(-1, 'testing');", 0, 0, 0);
  sqlite3_exec(db, "INSERT INTO Hashes VALUES(1, 'testing', 'testing');", 0, 0,
               0);
  sqlite3_exec(db, "INSERT INTO ScanMetaData VALUES('testing');", 0, 0, 0);

  // Act
  resetDB(db);

  // Assert
  sqlite3_stmt *dir_stmt;
  sqlite3_stmt *hash_stmt;
  sqlite3_stmt *scan_meta_data_stmt;
  sqlite3_prepare_v2(db, "SELECT * FROM Directories;", -1, &dir_stmt, 0);
  sqlite3_prepare_v2(db, "SELECT * FROM Hashes;", -1, &hash_stmt, 0);
  sqlite3_prepare_v2(db, "SELECT * FROM ScanMetaData;", -1,
                     &scan_meta_data_stmt, 0);

  assert(sqlite3_step(dir_stmt) == SQLITE_DONE &&
         sqlite3_step(hash_stmt) == SQLITE_DONE &&
         sqlite3_step(scan_meta_data_stmt) == SQLITE_DONE);

  // Cleanup
  sqlite3_finalize(dir_stmt);
  sqlite3_finalize(hash_stmt);
  sqlite3_finalize(scan_meta_data_stmt);
  freeDB(db);
  std::filesystem::remove(test_db);
}

void testResetingDirectoriesAutoIncrement() {
  // Arrange
  str_const test_db = "tests/test_reset_hash.db";
  sqlite3 *db = initDB(test_db);
  resetDB(db);
  sqlite3_exec(
      db, "INSERT INTO Directories (name, parent_id) VALUES('testing', -1);", 0,
      0, 0);

  // Act
  resetDB(db);

  // Assert
  sqlite3_exec(
      db, "INSERT INTO Directories  (name, parent_id)  VALUES('testing', -1);",
      0, 0, 0);
  sqlite3_stmt *res;
  sqlite3_prepare_v2(db, "SELECT * FROM Directories;", -1, &res, 0);
  while (sqlite3_step(res) != SQLITE_DONE) {
    assert(sqlite3_column_int(res, 0) == 1);
  }

  // Cleanup
  sqlite3_finalize(res);
  freeDB(db);
  std::filesystem::remove(test_db);
}

void testResetingHashesAutoIncrement() {
  // Arrange
  str_const test_db = "tests/test_reset_hash.db";
  sqlite3 *db = initDB(test_db);
  resetDB(db);
  sqlite3_exec(
      db,
      "INSERT INTO Hashes (directory_id, name, hash) VALUES('testing', -1);", 0,
      0, 0);

  // Act
  resetDB(db);

  // Assert
  sqlite3_exec(
      db, "INSERT INTO Directories  (name, parent_id)  VALUES('testing', -1);",
      0, 0, 0);
  sqlite3_stmt *res;
  sqlite3_prepare_v2(db, "SELECT * FROM Directories;", -1, &res, 0);
  while (sqlite3_step(res) != SQLITE_DONE) {
    assert(sqlite3_column_int(res, 0) == 1);
  }

  // Cleanup
  sqlite3_finalize(res);
  freeDB(db);
  std::filesystem::remove(test_db);
}

/* -------------------------- fetchLastDirectoryId -------------------------- */
const directory_table_row::rows expected_directory_table_rows = {
    {1, "/", -1},      {2, "home", 1},
    {3, "testing", 2}, {4, "WorkingDirectory", 3},
    {5, "ddupes", 4},  {6, "testing_dirs", 5},
    {7, "dir1", 6},    {8, "dir3", 6},
    {9, "sub_dir", 7}, {10, "sub_dir_two", 7},
};

void testFetchingLastDirectoryId() {
  // Arrange
  sqlite3 *db = initDB("tests/test_hash.db");

  // Act
  int actual_directory_id = fetchLastDirectoryId(db);

  // Assert
  assert(actual_directory_id == 10);

  // Cleanup
  freeDB(db);
}

void testFetchingLastDirectoryIdReturnsNegative() {
  // Arrange
  str_const test_db = "tests/test_empty_id_hash.db";
  sqlite3 *db = initDB(test_db);
  resetDB(db);

  // Act
  int actual_directory_id = fetchLastDirectoryId(db);

  // Assert
  assert(actual_directory_id == -1);

  // Cleanup
  std::filesystem::remove(test_db);
  freeDB(db);
}

/* --------------------------- fetchAllDirectories -------------------------- */
// TODO test error result with prepare statement when the table does not exit.
void testLoadingDirectoriesFromTestDB() {
  // Arrange
  sqlite3 *db = initDB("tests/test_hash.db");

  // Act
  directory_table_row::rows actual_directory_table_rows =
      fetchAllDirectories(db);

  // Assert
  assert(actual_directory_table_rows == expected_directory_table_rows);

  // Cleanup
  freeDB(db);
}

/* ----------------------------- createDirectory ---------------------------- */
void testCreatingANewDirectory() {
  // Arrange
  str_const test_db = "tests/test_create_hash.db";
  sqlite3 *db = initDB(test_db);
  resetDB(db);
  directory_input test_directory{.parent_id = 8, .name = "testing_create"};

  // Act
  int id = createDirectory(db, test_directory);

  // Assert
  assert(id == 1);

  // Cleanup
  std::filesystem::remove(test_db);
  freeDB(db);
}

/* ----------------------------- fetchAllHashes ----------------------------- */
const hash_table_row::rows expected_hash_table_rows = {
    {1, 8, "example1.txt",
     new uint8_t[16]{141, 221, 139, 228, 177, 121, 165, 41, 175, 165, 242, 255,
                     174, 75, 152, 88}},
    {2, 7, "example1.txt",
     new uint8_t[16]{141, 221, 139, 228, 177, 121, 165, 41, 175, 165, 242, 255,
                     174, 75, 152, 88}},
    {3, 8, "example3.txt",
     new uint8_t[16]{149, 94, 253, 46, 194, 84, 142, 89, 168, 221, 65, 254, 159,
                     139, 200, 60}},
    {4, 7, "example3.txt",
     new uint8_t[16]{149, 94, 253, 46, 194, 84, 142, 89, 168, 221, 65, 254, 159,
                     139, 200, 60}},
    {5, 9, "example4.txt",
     new uint8_t[16]{150, 69, 150, 207, 35, 44, 71, 46, 198, 59, 175, 2, 153,
                     240, 212, 80}},
    {6, 7, "example5.txt",
     new uint8_t[16]{150, 69, 150, 207, 35, 44, 71, 46, 198, 59, 175, 2, 153,
                     240, 212, 80}},
    {7, 10, "example8.txt",
     new uint8_t[16]{122, 71, 103, 43, 252, 178, 37, 83, 168, 171, 14, 203, 175,
                     36, 116, 66}}};

/* ----------------------------- fetchLastHashId ---------------------------- */
void testFetchingLastHashId() {
  // Arrange
  sqlite3 *db = initDB("tests/test_hash.db");

  // Act
  int actual_hash_id = fetchLastHashId(db);

  // Assert
  assert(actual_hash_id == 7);

  // Cleanup
  freeDB(db);
}

void testFetchingLastHashIdReturnsNegative() {
  // Arrange
  str_const test_db = "tests/test_empty_id_hash.db";
  sqlite3 *db = initDB(test_db);
  resetDB(db);

  // Act
  int actual_hash_id = fetchLastHashId(db);

  // Assert
  assert(actual_hash_id == -1);

  // Cleanup
  std::filesystem::remove(test_db);
  freeDB(db);
}

/* ----------------------------- fetchAllHashes ----------------------------- */
void testLoadingHashesFromTestDB() {
  // Arrange
  sqlite3 *db = initDB("tests/test_hash.db");

  // Act
  hash_table_row::rows actual_hash_table_rows = fetchAllHashes(db);

  // Assert
  assert(actual_hash_table_rows == expected_hash_table_rows);

  // Cleanup
  freeDB(db);
}

/* ------------------------------- createHash ------------------------------- */
void testCreatingANewHash() {
  // Arrange
  str_const test_db = "tests/test_create_hash.db";
  sqlite3 *db = initDB(test_db);
  resetDB(db);
  hash_input test_hash{
      .directory_id = 10, .name = "testing.txt", .hash = uniqueTestHash()};

  // Act
  int id = createHash(db, test_hash);

  // Assert
  assert(id == 1);

  // Cleanup
  std::filesystem::remove(test_db);
  freeDB(db);
}

/* ------------------------------- deleteHash ------------------------------- */
void testDeletingAHash() {
  // Arrange
  str_const test_db = "tests/test_delete_hash.db";
  sqlite3 *db = initDB(test_db);
  resetDB(db);
  hash_input test_hash{
      .directory_id = 10, .name = "testing.txt", .hash = uniqueTestHash()};
  int id = createHash(db, test_hash);

  // Act
  deleteHash(db, id);

  // Assert
  assert(fetchLastHashId(db) == -1);

  // Cleanup
  std::filesystem::remove(test_db);
  freeDB(db);
}

/* ---------------------------- fetchScanMetaData --------------------------- */
void testFetchScanMetaData() {
  // Arrange
  sqlite3 *db = initDB("tests/test_hash.db");

  // Act
  scan_meta_data_table_row row = fetchScanMetaData(db);

  // Assert
  scan_meta_data_table_row expected_scan_meta_data_row{.root_dir = "testing"};
  assert(row == expected_scan_meta_data_row);

  // Cleanup
  freeDB(db);
}

void testFetchScanMetaDataReturnsErrorWhenMissing() {
  // Arrange
  str_const test_db = "tests/test_empty_hash.db";
  sqlite3 *db = initDB(test_db);
  resetDB(db);

  try {
    // Act
    scan_meta_data_table_row row = fetchScanMetaData(db);

    assert(false);
  } catch (not_found_error &e) {
    // Assert
    assert(true);
  }
  // Cleanup
  freeDB(db);
  std::filesystem::remove(test_db);
}

/* --------------------------- createScanMetaData --------------------------- */
void testCreatingScanMetaData() {
  // Arrange
  str_const test_db = "tests/test_create_hash.db";
  sqlite3 *db = initDB(test_db);
  resetDB(db);
  scan_meta_data_input test_scan_meta_data{.root_dir = "testing"};

  // Act
  createScanMetaData(db, test_scan_meta_data);

  // Assert
  scan_meta_data_table_row row = fetchScanMetaData(db);
  scan_meta_data_table_row expected_row{.root_dir = "testing"};
  assert(row == expected_row);

  // Cleanup
  std::filesystem::remove(test_db);
  freeDB(db);
}

void testCreatingScanMetaDataOverwritesPrevious() {
  // Arrange
  str_const test_db = "tests/test_create_hash.db";
  sqlite3 *db = initDB(test_db);
  resetDB(db);

  scan_meta_data_input test_scan_meta_data_one{.root_dir = "testing"};
  scan_meta_data_input test_scan_meta_data_two{.root_dir = "testing_two"};

  createScanMetaData(db, test_scan_meta_data_one);

  // Act
  createScanMetaData(db, test_scan_meta_data_two);

  // Assert
  scan_meta_data_table_row row = fetchScanMetaData(db);
  scan_meta_data_table_row expected_row{.root_dir = "testing_two"};
  assert(row == expected_row);

  // Cleanup
  std::filesystem::remove(test_db);
  freeDB(db);
}

int main() {
  testConnectingToDb();
  testResetingDatabase();
  testResetingTables();
  testResetingDirectoriesAutoIncrement();
  testResetingHashesAutoIncrement();
  testFetchingLastDirectoryId();
  testFetchingLastDirectoryIdReturnsNegative();
  testLoadingDirectoriesFromTestDB();
  testCreatingANewDirectory();
  testFetchingLastHashId();
  testFetchingLastHashIdReturnsNegative();
  testLoadingHashesFromTestDB();
  testCreatingANewHash();
  testDeletingAHash();
  testFetchScanMetaData();
  testFetchScanMetaDataReturnsErrorWhenMissing();
  testCreatingScanMetaData();
  testCreatingScanMetaDataOverwritesPrevious();
}
