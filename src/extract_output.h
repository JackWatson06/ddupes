#pragma once

#include "./sqlite/sqlite.h"

struct FileHashRows {
  DirectoryTableRow::Rows directory_rows;
  HashTableRow::Rows hash_rows;

  bool operator==(const FileHashRows& rhs) const;
};