#include <cassert>

#include "../src/extract_output.h"

/* ------------------------------ FileHashRows ------------------------------ */
void testFileHashRowsEqualOperator() {
  // Arrange
  file_hash_rows test_one{{directory_table_row{1, "test", 1}},
                          {hash_table_row{1, "test", nullptr}}};
  file_hash_rows test_two{{directory_table_row{1, "test", 1}},
                          {hash_table_row{1, "test", nullptr}}};

  // Act
  bool equality_test = test_one == test_two;

  // Assert
  assert(equality_test);
}

int main() { testFileHashRowsEqualOperator(); }