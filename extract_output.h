#pragma once
#include <cstring>
#include <memory>
#include <string>
#include <vector>

constexpr uint MD5_DIGEST_LENGTH = 16;

typedef std::vector<uint8_t> Hash;

struct DirectoryTableRow {
  typedef std::vector<DirectoryTableRow> Rows;

  int id;
  std::string name;
  int parent_id;

  bool operator==(const DirectoryTableRow& rhs) const;
};

struct HashTableRow {
  typedef std::vector<HashTableRow> Rows;
  int directory_id;
  std::string name;
  uint8_t* hash;

  ~HashTableRow() {
    if (hash != nullptr) {
      delete[] hash;
    }
  }
  HashTableRow(int directory_id, std::string name, uint8_t* hash)
      : directory_id(directory_id), name(name), hash(hash) {}
  HashTableRow() : directory_id(-1), name(""), hash(nullptr) {}
  HashTableRow(const HashTableRow& other);
  HashTableRow& operator=(const HashTableRow& other);

  bool operator==(const HashTableRow& rhs) const;
};

struct FileHashRows {
  DirectoryTableRow::Rows directory_rows;
  HashTableRow::Rows hash_rows;

  bool operator==(const FileHashRows& rhs) const;
};

bool compareBlobs(const uint8_t* blob_one, const uint8_t* blob_two);