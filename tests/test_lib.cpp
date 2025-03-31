#include <assert.h>

#include "../src/lib.h"
#include "./data.cpp"

/* ------------------------------ stringLength ------------------------------ */
void testStringLength() {
  // Arrange
  str_const test_string_one = "hello!";

  // Act
  int count = stringLength(test_string_one);

  // Assert
  assert(count == 7);
}

/* -------------------------------- stringDup ------------------------------- */

void testStringDup() {
  // Arrange
  str_const test_string_one = "hello!";

  // Act
  char *string_two = stringDup(test_string_one);

  // Assert
  assert(compareStrings(test_string_one, string_two));
}

/* ----------------------------- compareStrings ----------------------------- */
void testStringsEqual() {
  // Arrange
  str_const test_string_one = "hello!";
  str_const test_string_two = "hello!";

  // Act
  bool equality_test = compareStrings(test_string_one, test_string_two);

  // Assert
  assert(equality_test);
}

void testStringsNotEqual() {
  // Arrange
  str_const test_string_one = "hello!";
  str_const test_string_two = "hello";

  // Act
  bool equality_test = compareStrings(test_string_one, test_string_two);

  // Assert
  assert(equality_test == false);
}
/* ------------------------------- computeHash ----------------------------- */
void testHashingAListOfHashes() {
  // Arrange
  hashes test_hashes =
      new hash[3]{uniqueTestHash(), uniqueTestHash(), uniqueTestHash()};

  // Act
  hash actual_hash = computeHash(test_hashes, 3);

  // Assert
  hash expected_hash = new uint8_t[MD5_DIGEST_LENGTH]{
      91, 83, 0, 15, 77, 131, 26, 67, 48, 112, 248, 0, 4, 38, 41, 16};
  assert(compareHashes(expected_hash, actual_hash));
}

void testTwoHashesAreDifferent() {
  // Arrange
  hashes test_one_hashes =
      new hash[3]{uniqueTestHash(), uniqueTestHash(), uniqueTestHash()};
  hashes test_two_hashes = new hash[2]{uniqueTestHash(), uniqueTestHash()};

  // Act
  hash actual_hash_one = computeHash(test_one_hashes, 3);
  hash actual_hash_two = computeHash(test_two_hashes, 2);

  // Assert
  assert(!compareHashes(actual_hash_one, actual_hash_two));
}

/* ------------------------------ compareHashes ----------------------------- */
void testCompareHashesEqual() {
  // Arrange
  uint8_t *test_one = uniqueTestHash();
  uint8_t *test_two = uniqueTestHash();

  // Act
  bool equality_test = compareHashes(test_one, test_two);

  // Assert
  assert(equality_test == true);
}

void testCompareHashesNotEqual() {
  // Arrange
  uint8_t *test_one = uniqueTestHash();
  uint8_t *test_two = uniqueTestHash();

  test_one[8] = 200;
  test_two[9] = 200;

  // Act
  bool equality_test = compareHashes(test_one, test_two);

  // Assert
  assert(equality_test == false);
}

void testCompareHashesWithNullptr() {
  // Arrange
  uint8_t *test_one = nullptr;
  uint8_t *test_two = nullptr;

  // Act
  bool equality_test = compareHashes(test_one, test_two);

  // Assert
  assert(equality_test == true);
}

void testCompareHashesWithNullptrNotEqual() {
  // Arrange
  uint8_t *test_one = nullptr;
  uint8_t *test_two = uniqueTestHash();

  // Act
  bool equality_test = compareHashes(test_one, test_two);

  // Assert
  assert(equality_test == false);
}

/* --------------------------------- hashDup -------------------------------- */
void testHashDup() {
  // Arrange
  hash test_hash = new uint8_t[MD5_DIGEST_LENGTH]{
      91, 83, 0, 15, 77, 131, 26, 67, 48, 112, 248, 0, 4, 38, 41, 16};

  // Act
  hash actual_hash = hashDup(test_hash);

  // Assert
  assert(compareHashes(test_hash, actual_hash));
}

int main() {
  testStringLength();
  testStringDup();
  testStringsEqual();
  testStringsNotEqual();
  testHashingAListOfHashes();
  testTwoHashesAreDifferent();
  testCompareHashesEqual();
  testCompareHashesNotEqual();
  testCompareHashesWithNullptr();
  testCompareHashesWithNullptrNotEqual();
}