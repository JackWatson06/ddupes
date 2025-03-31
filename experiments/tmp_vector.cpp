
#include <iostream>
#include <vector>

typedef unsigned char* hash;

// When using in a vector. The vector will not remove the value_string and the
// hash_data. I think it will simply run a delete[] on the vector's internal
// array. Leaving the memory pointed to originally untouched.
struct TestRow {
  int const value_one;
  char const* const value_string;
  hash const hash_data;
};

int testing_function(TestRow const& testing_row) {
  std::vector<TestRow*> vecotr_of_rows{
      testing_row, testing_row, testing_row, testing_row,
      testing_row, testing_row, testing_row,
  };

  for (TestRow const vector_of_row : vecotr_of_rows) {
    std::cout << vector_of_row.value_string << std::endl;
  }

  return 0;
}

int main() {
  hash testing_hash = new unsigned char[2]{'a', 'b'};
  TestRow testing_row{
      .value_one = 100, .value_string = "Hello!", .hash_data = testing_hash};

  testing_function(testing_row);

  std::cout << "Out of function!" << std::endl;
  std::cout << testing_row.value_string << std::endl;

  for (int i = 0; i < 2; ++i) {
    std::cout << testing_hash[i] << std::endl;
  }
}
