#include <list>
#include <ostream>
#include <unordered_map>

#include "../lib.h"
#include "../sqlite/sqlite.h"
#include "./transform_output.h"

struct file_hash_rows {
  directory_table_row::rows directory_rows;
  hash_table_row::rows hash_rows;

  bool operator==(const file_hash_rows &rhs) const;
};

duplicate_path_seg_set transform(file_hash_rows const &);
