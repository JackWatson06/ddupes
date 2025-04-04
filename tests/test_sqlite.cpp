
#include <cassert>
#include <filesystem>

#include "../src/sqlite/sqlite.h"
#include "./data.cpp"

// TODO: Test with spaces.

/* ----------------------------- SQLiteDatabase ----------------------------- */
void testConnectingToDb() {
  // Arrange & Act & Assert
  try {
    sqlite3 *db = initDB("tests/test_hash.db");
    assert(true);
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

  assert(directories_code == SQLITE_OK && hashes_code == SQLITE_OK);

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

  // Act
  resetDB(db);

  // Assert
  sqlite3_stmt *dir_stmt;
  sqlite3_stmt *hash_stmt;
  sqlite3_prepare_v2(db, "SELECT * FROM Directories;", -1, &dir_stmt, 0);
  sqlite3_prepare_v2(db, "SELECT * FROM Hashes;", -1, &hash_stmt, 0);

  assert(sqlite3_step(dir_stmt) == SQLITE_DONE &&
         sqlite3_step(hash_stmt) == SQLITE_DONE);

  // Cleanup
  sqlite3_finalize(dir_stmt);
  sqlite3_finalize(hash_stmt);
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

/* ---------------------------- DirectoryTableRow --------------------------- */
void testDirectoryTableRowEqualOperator() {
  // Arrange
  directory_table_row test_one{1, "test", 1};
  directory_table_row test_two{1, "test", 1};

  // Act
  bool equality_test = test_one == test_two;

  // Assert
  assert(equality_test);
}

/* ---------------------------- HashTableRow --------------------------- */
void testHashTableRowEqualOperator() {
  // Arrange
  hash_table_row test_one{1, "test", uniqueTestHash()};
  hash_table_row test_two{1, "test", uniqueTestHash()};

  // Act
  bool equality_test = test_one == test_two;

  // Assert
  assert(equality_test);
}

void testHashTableRowCopyConstructor() {
  // Arrange
  hash_table_row test{1, "test", uniqueTestHash()};

  // Act
  hash_table_row actual_hash_table_row{test};

  // Assert
  bool equality_test = test == actual_hash_table_row;
  assert(equality_test);
}

void testHashTableRowCopyConstructorWithNull() {
  // Arrange
  hash_table_row test{1, "test", nullptr};

  // Act
  hash_table_row actual_hash_table_row{test};

  // Assert
  bool equality_test = test == actual_hash_table_row;
  assert(equality_test);
}

/* -------------------------------- HashInput ------------------------------- */

void testHashInputEquals() {
  // Arrange
  hash_input test_hash_input_one = {1, "testing", uniqueTestHash()};
  hash_input test_hash_input_two = {1, "testing", uniqueTestHash()};

  // Act
  bool equality_test = test_hash_input_one == test_hash_input_two;

  // Assert
  assert(equality_test);
}

void testHashInputNotEquals() {
  // Arrange
  hash_input test_hash_input_one = {1, "testing", uniqueTestHash()};
  hash_input test_hash_input_two = {1, "testing", uniqueTestHash(127)};

  // Act
  bool equality_test = test_hash_input_one == test_hash_input_two;

  // Assert
  assert(!equality_test);
}

/* ----------------------------- DirectoryInput ----------------------------- */
void testDirectoryInputEquals() {
  // Arrange
  directory_input test_directory_input_one = {1, "testing"};
  directory_input test_directory_input_two = {1, "testing"};

  // Act
  bool equality_test = test_directory_input_one == test_directory_input_two;

  // Assert
  assert(equality_test);
}

/* ------------------------- fetchDirectoryIdByName ------------------------- */
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
    {8, "example1.txt",
     new uint8_t[16]{141, 221, 139, 228, 177, 121, 165, 41, 175, 165, 242, 255,
                     174, 75, 152, 88}},
    {7, "example1.txt",
     new uint8_t[16]{141, 221, 139, 228, 177, 121, 165, 41, 175, 165, 242, 255,
                     174, 75, 152, 88}},
    {8, "example3.txt",
     new uint8_t[16]{149, 94, 253, 46, 194, 84, 142, 89, 168, 221, 65, 254, 159,
                     139, 200, 60}},
    {7, "example3.txt",
     new uint8_t[16]{149, 94, 253, 46, 194, 84, 142, 89, 168, 221, 65, 254, 159,
                     139, 200, 60}},
    {9, "example4.txt",
     new uint8_t[16]{150, 69, 150, 207, 35, 44, 71, 46, 198, 59, 175, 2, 153,
                     240, 212, 80}},
    {7, "example5.txt",
     new uint8_t[16]{150, 69, 150, 207, 35, 44, 71, 46, 198, 59, 175, 2, 153,
                     240, 212, 80}},
    {10, "example8.txt",
     new uint8_t[16]{122, 71, 103, 43, 252, 178, 37, 83, 168, 171, 14, 203, 175,
                     36, 116, 66}}};

void testLoadingHashesFromTestDB() {
  // Arrange
  sqlite3 *db = initDB("tests/test_hash.db");

  // Act
  hash_table_row::rows actual_hash_table_rows = fetchAllHashes(db);

  // Assert
  assert(actual_hash_table_rows == expected_hash_table_rows);

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
  createHash(db, test_hash);

  // Assert
  hash_table_row::rows expected_hashes = fetchAllHashes(db);
  assert(expected_hashes.size() == 1);

  // Cleanup
  std::filesystem::remove(test_db);
  freeDB(db);
}

int main() {
  testConnectingToDb();
  testResetingDatabase();
  testResetingTables();
  testResetingDirectoriesAutoIncrement();
  testDirectoryTableRowEqualOperator();
  testHashTableRowEqualOperator();
  testHashTableRowCopyConstructor();
  testHashTableRowCopyConstructorWithNull();
  testHashInputEquals();
  testHashInputNotEquals();
  testDirectoryInputEquals();
  testFetchingLastDirectoryId();
  testFetchingLastDirectoryIdReturnsNegative();
  testLoadingDirectoriesFromTestDB();
  testCreatingANewDirectory();
  testLoadingHashesFromTestDB();
  testCreatingANewHash();
}
