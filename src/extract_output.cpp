#include "extract_output.h"

bool FileHashRows::operator==(const FileHashRows& rhs) const {
  return rhs.directory_rows == directory_rows && rhs.hash_rows == hash_rows;
}