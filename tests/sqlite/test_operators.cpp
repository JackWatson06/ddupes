#include <cassert>

#include "../../src/lib.cpp"
#include "../../src/sqlite/operators.cpp"
#include "../data.cpp"

/* --------------------------- directory_table_row -------------------------- */
void testDirectoryTableRowEqualOperator() {
  // Arrange
  directory_table_row test_one{1, "test", 1};
  directory_table_row test_two{1, "test", 1};

  // Act
  bool equality_test = test_one == test_two;

  // Assert
  assert(equality_test);
}

/* ----------------------------- hash_table_row ----------------------------- */
void testHashTableRowEqualOperator() {
  // Arrange
  hash_table_row test_one{1, 1, "test", uniqueTestHash()};
  hash_table_row test_two{1, 1, "test", uniqueTestHash()};

  // Act
  bool equality_test = test_one == test_two;

  // Assert
  assert(equality_test);
}

/* ------------------------ scan_meta_data_table_row ------------------------ */
void testScanMetaDataRowEqualOperator() {
  // Arrange
  scan_meta_data_table_row test_one{"test"};
  scan_meta_data_table_row test_two{"test"};

  // Act
  bool equality_test = test_one == test_two;

  // Assert
  assert(equality_test);
}

/* ------------------------------- hash_input ------------------------------- */
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

/* ----------------------------- directory_input ---------------------------- */
void testDirectoryInputEquals() {
  // Arrange
  directory_input test_directory_input_one = {1, "testing"};
  directory_input test_directory_input_two = {1, "testing"};

  // Act
  bool equality_test = test_directory_input_one == test_directory_input_two;

  // Assert
  assert(equality_test);
}

/* -------------------------- scan_meta_data_input -------------------------- */
void testScanMetaDataInputEquals() {
  // Arrange
  scan_meta_data_input test_scan_meta_data_input_one = {"testing"};
  scan_meta_data_input test_scan_meta_data_input_two = {"testing"};

  // Act
  bool equality_test =
      test_scan_meta_data_input_one == test_scan_meta_data_input_two;

  // Assert
  assert(equality_test);
}

int main() {
  testDirectoryTableRowEqualOperator();
  testHashTableRowEqualOperator();
  testScanMetaDataRowEqualOperator();
  testHashInputEquals();
  testHashInputNotEquals();
  testDirectoryInputEquals();
  testScanMetaDataInputEquals();
}