#include <memory>
#include <string>
#include <vector>

typedef const unsigned char* puchar;
typedef std::vector<uint8_t> Hash;

struct DirectoryTableRow {
  typedef std::vector<DirectoryTableRow> Rows;

  int id;
  const unsigned char* name;
  int parent_id;

  bool operator==(const DirectoryTableRow& rhs) const;
};

struct HashTableRow {
  typedef std::vector<HashTableRow> Rows;
  int directory_id;
  const unsigned char* name;
  const void* hash;

  bool operator==(const HashTableRow& rhs) const;
};

struct FileHashRows {
  DirectoryTableRow::Rows directory_rows;
  HashTableRow::Rows hash_rows;

  bool operator==(const FileHashRows& rhs) const;
};
