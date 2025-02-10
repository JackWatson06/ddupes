#include <string>
#include <vector>

typedef std::vector<uint8_t> Hash;

struct DirectoryTableRow {
  typedef std::vector<DirectoryTableRow> Rows;
  unsigned int id;
  std::string name;
  unsigned int *parent_id;
};

struct HashTableRow {
  typedef std::vector<HashTableRow> Rows;
  unsigned int directory_id;
  std::string name;
  Hash hash;
};
