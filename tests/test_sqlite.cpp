
#include <cassert>

#include "../sqlite.h"

/* ----------------------------- SQLiteDatabase ----------------------------- */
void testConnectingToDb() {
  // Arrange & Act & Assert
  try {
    SQLiteDatabase db{"tests/hash.db"};
    assert(true);
  } catch (UnableToConnectError& e) {
    assert(false);
  }
}

/* ----------------------------- SQLiteTableView ---------------------------- */
void testUnableToBuildStatement() {
  // Arrange
  SQLiteDatabase db{"tests/hash.db"};

  DirectoryTableView directory_table_view{db};

  // Act & Assert
  try {
    directory_table_view.prepare("TEST INVALID");
    assert(false);
  } catch (UnableToBuildStatementError& e) {
    assert(true);
  }
}

void testFetchingUntilDone() {
  // Arrange
  SQLiteDatabase db{"tests/hash.db"};
  DirectoryTableView directory_table_view{db};
  directory_table_view.prepare("SELECT * FROM Directories;");

  const int expected_directories_in_table = 11;
  bool continue_to_step = true;

  // Act
  for (unsigned int i = 0; i < expected_directories_in_table; ++i) {
    continue_to_step = directory_table_view.step();
  }

  // Assert
  assert(continue_to_step == false);
}

/* --------------------------- DirectoryTableView --------------------------- */
const DirectoryTableRow::Rows expected_directory_table_rows = {
    {1, "/", -1},      {2, "home", 1},
    {3, "jack", 2},    {4, "WorkingDirectory", 3},
    {5, "ddupes", 4},  {6, "testing_dirs", 5},
    {7, "dir1", 6},    {8, "dir3", 6},
    {9, "sub_dir", 7}, {10, "sub_dir_two", 7},
};

void testLoadingDirectoriesFromTestDB() {
  // Arrange
  SQLiteDatabase db{"tests/hash.db"};
  DirectoryTableView directory_table_view{db};
  directory_table_view.prepare("SELECT * FROM Directories;");

  // Act
  DirectoryTableRow::Rows actual_directory_table_rows{};
  while (directory_table_view.step()) {
    actual_directory_table_rows.push_back(
        directory_table_view.getLastRowFetched());
  }

  // Assert
  assert(actual_directory_table_rows == expected_directory_table_rows);
}

/* ------------------------------ HashTableView ----------------------------- */
const HashTableRow::Rows expected_hash_table_rows = {
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
  SQLiteDatabase db{"tests/hash.db"};
  HashTableView hash_table_view{db};
  hash_table_view.prepare("SELECT * FROM Hashes;");

  // Act
  HashTableRow::Rows actual_hash_table_rows{};
  while (hash_table_view.step()) {
    actual_hash_table_rows.push_back(hash_table_view.getLastRowFetched());
  }

  // Assert
  assert(actual_hash_table_rows == expected_hash_table_rows);
}

int main() {
  testConnectingToDb();
  testUnableToBuildStatement();
  testFetchingUntilDone();
  testLoadingDirectoriesFromTestDB();
  testLoadingHashesFromTestDB();
}
