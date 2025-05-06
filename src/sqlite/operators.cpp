#include "sqlite.h"

bool directory_table_row::operator==(const directory_table_row &rhs) const {
  return rhs.id == id && compareStrings(rhs.name, name) &&
         rhs.parent_id == parent_id;
};

bool hash_table_row::operator==(const hash_table_row &rhs) const {
  return rhs.id == id && rhs.directory_id == directory_id &&
         compareStrings(rhs.name, name) && compareHashes(hash, rhs.hash);
};

bool scan_meta_data_table_row::operator==(
    const scan_meta_data_table_row &rhs) const {
  return compareStrings(rhs.root_dir, root_dir);
};

bool directory_input::operator==(const directory_input &rhs) const {
  return rhs.parent_id == parent_id && compareStrings(rhs.name, name);
}

bool hash_input::operator==(const hash_input &rhs) const {
  return rhs.directory_id == directory_id && compareStrings(rhs.name, name) &&
         compareHashes(hash, rhs.hash);
}

bool scan_meta_data_input::operator==(const scan_meta_data_input &rhs) const {
  return compareStrings(rhs.root_dir, root_dir);
};