#include "extract_output.h"

bool compareBlobs(const uint8_t* blob_one, const uint8_t* blob_two) {
  if (blob_one == nullptr || blob_two == nullptr) {
    return blob_one == blob_two;
  }

  for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
    if (blob_one[i] != blob_two[i]) {
      return false;
    }
  }

  return true;
}

bool HashTableRow::operator==(const HashTableRow& rhs) const {
  return rhs.directory_id == directory_id && rhs.name == name &&
         compareBlobs(hash, rhs.hash);
};

HashTableRow::HashTableRow(const HashTableRow& other) {
  directory_id = other.directory_id;
  name = other.name;

  if (other.hash != nullptr) {
    uint8_t* blob_buffer = new uint8_t[MD5_DIGEST_LENGTH];
    std::memcpy(blob_buffer, other.hash, MD5_DIGEST_LENGTH);

    hash = blob_buffer;
  } else {
    hash = nullptr;
  }
}

HashTableRow& HashTableRow::operator=(const HashTableRow& other) {
  if (this == &other) return *this;

  delete[] hash;

  directory_id = other.directory_id;
  name = other.name;

  if (other.hash != nullptr) {
    hash = new uint8_t[MD5_DIGEST_LENGTH];
    std::memcpy(hash, other.hash, MD5_DIGEST_LENGTH);
  } else {
    hash = nullptr;
  }
  return *this;
}

bool DirectoryTableRow::operator==(const DirectoryTableRow& rhs) const {
  return rhs.id == id && rhs.name == name && rhs.parent_id == parent_id;
};

bool FileHashRows::operator==(const FileHashRows& rhs) const {
  return rhs.directory_rows == directory_rows && rhs.hash_rows == hash_rows;
}