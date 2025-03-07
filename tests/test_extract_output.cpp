#include <cassert>

#include "../src/extract_output.h"

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

int main() { testFileHashRowsEqualOperator(); }