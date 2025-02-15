#include <cassert>

#include "../extract_output.h"

uint8_t* uniqueTestBlob(uint8_t first_byte = 255) {
  return new uint8_t[MD5_DIGEST_LENGTH]{first_byte, 255, 255, 255, 255, 255,
                                        255,        255, 255, 255, 255, 255,
                                        255,        255, 255, 255};
}

/* ---------------------------- DirectoryTableRow --------------------------- */
void testDirectoryTableRowEqualOperator() {
  // Arrange
  DirectoryTableRow test_one{1, "test", 1};
  DirectoryTableRow test_two{1, "test", 1};

  // Act
  bool equality_test = test_one == test_two;

  // Assert
  assert(equality_test);
}

/* ---------------------------- HashTableRow --------------------------- */
void testHashTableRowEqualOperator() {
  // Arrange
  HashTableRow test_one{1, "test", uniqueTestBlob()};
  HashTableRow test_two{1, "test", uniqueTestBlob()};

  // Act
  bool equality_test = test_one == test_two;

  // Assert
  assert(equality_test);
}

void testHashTableRowCopyConstructor() {
  // Arrange
  HashTableRow test{1, "test", uniqueTestBlob()};

  // Act
  HashTableRow actual_hash_table_row{test};

  // Assert
  bool equality_test = test == actual_hash_table_row;
  assert(equality_test);
}

void testHashTableRowCopyConstructorWithNull() {
  // Arrange
  HashTableRow test{1, "test", nullptr};

  // Act
  HashTableRow actual_hash_table_row{test};

  // Assert
  bool equality_test = test == actual_hash_table_row;
  assert(equality_test);
}

void testHashTableAssignmentOperator() {
  // Arrange
  HashTableRow test_one{1, "test", uniqueTestBlob(80)};
  HashTableRow test_two{1, "test", uniqueTestBlob(100)};

  // Act
  test_one = test_two;

  // Assert
  assert(test_one.hash[0] == 100);
}

void testHashTableAssignmentOperatorWithSame() {
  // Arrange
  HashTableRow test_one{1, "test", uniqueTestBlob(80)};

  // Act
  test_one = test_one;

  // Assert
  assert(test_one.hash[0] == 80);
}

void testHashTableEqualityOperatorWithNull() {
  // Arrange
  HashTableRow test_one{1, "test", uniqueTestBlob(80)};
  HashTableRow test_two{1, "test", nullptr};

  // Act
  test_one = test_two;

  // Assert
  assert(test_one.hash == nullptr);
}

/* ------------------------------ compareBlobs ------------------------------ */
void testCompareBlobsEqual() {
  // Arrange
  uint8_t* test_one = uniqueTestBlob();
  uint8_t* test_two = uniqueTestBlob();

  // Act
  bool equality_test = compareBlobs(test_one, test_two);

  // Assert
  assert(equality_test == true);
}

void testCompareBlobsNotEqual() {
  // Arrange
  uint8_t* test_one = uniqueTestBlob();
  uint8_t* test_two = uniqueTestBlob();

  test_one[8] = 200;
  test_two[9] = 200;

  // Act
  bool equality_test = compareBlobs(test_one, test_two);

  // Assert
  assert(equality_test == false);
}

void testCompareBlobsWithNullptr() {
  // Arrange
  uint8_t* test_one = nullptr;
  uint8_t* test_two = nullptr;

  // Act
  bool equality_test = compareBlobs(test_one, test_two);

  // Assert
  assert(equality_test == true);
}

void testCompareBlobsWithNullptrNotEqual() {
  // Arrange
  uint8_t* test_one = nullptr;
  uint8_t* test_two = uniqueTestBlob();

  // Act
  bool equality_test = compareBlobs(test_one, test_two);

  // Assert
  assert(equality_test == false);
}

/* ------------------------------ FileHashRows ------------------------------ */
void testFileHashRowsEqualOperator() {
  // Arrange
  FileHashRows test_one{{DirectoryTableRow{1, "test", 1}},
                        {HashTableRow{1, "test", nullptr}}};
  FileHashRows test_two{{DirectoryTableRow{1, "test", 1}},
                        {HashTableRow{1, "test", nullptr}}};

  // Act
  bool equality_test = test_one == test_two;

  // Assert
  assert(equality_test);
}

int main() {
  testDirectoryTableRowEqualOperator();
  testHashTableRowEqualOperator();
  testHashTableRowCopyConstructor();
  testHashTableRowCopyConstructorWithNull();
  testHashTableAssignmentOperator();
  testHashTableAssignmentOperatorWithSame();
  testHashTableEqualityOperatorWithNull();
  testCompareBlobsEqual();
  testCompareBlobsNotEqual();
  testCompareBlobsWithNullptr();
  testCompareBlobsWithNullptrNotEqual();
  testFileHashRowsEqualOperator();
}