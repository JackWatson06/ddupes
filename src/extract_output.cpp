#include "extract_output.h"

bool file_hash_rows::operator==(file_hash_rows const &rhs) const {
  return rhs.directory_rows == directory_rows && rhs.hash_rows == hash_rows;
}