#include <memory>
#include <string>
#include <vector>

typedef std::vector<uint8_t> Hash;

struct DirectoryTableRow {
  typedef std::vector<DirectoryTableRow> Rows;

  int id;
  const unsigned char* name;
  int parent_id;

  // DirectoryTableRow(DirectoryTableRow& other)
  //     : id(other.id), name(other.name), parent_id(other.parent_id) {};
  bool operator==(const DirectoryTableRow& rhs);
};

struct HashTableRow {
  typedef std::vector<HashTableRow> Rows;
  int directory_id;
  const unsigned char* name;
  const void* hash;

  // HashTableRow(HashTableRow& other)
  //     : directory_id(other.directory_id), name(other.name), hash(other.hash)
  //     {};
  bool operator==(const HashTableRow& rhs);
};

struct FileHashRows {
  DirectoryTableRow::Rows directory_rows;
  HashTableRow::Rows hash_rows;

  bool operator==(const FileHashRows& rhs);
};

// struct DirectoryTableRow {
//   typedef std::vector<DirectoryTableRow> Rows;
//   unsigned int id;
//   std::string name;
//   unsigned int parent_id;
// };

// struct HashTableRow {
//   typedef std::vector<HashTableRow> Rows;
//   unsigned int directory_id;
//   std::string name;
//   Hash hash;
// };
