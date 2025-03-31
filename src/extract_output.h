#pragma once

#include "./sqlite/sqlite.h"

struct file_hash_rows {
  directory_table_row::rows directory_rows;
  hash_table_row::rows hash_rows;

  bool operator==(const file_hash_rows &rhs) const;
};