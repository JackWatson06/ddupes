#include "./update.h"

/**
 * - Extract from the cache.
 * - Build hash map from directories id's to the directory_row (same as dupes
 * command)
 * - Loop over all the files. Build the path and tack on the meta data path root
 * thing.
 * - Check if the file exists. If it does not remove it from the cache using the
 * id.
 * - Loop over all the directories. Build the path.
 * - Check if the directory exists. If it does not remove it from the cache
 * using the id. This does not need to be in order. If any child directory is
 * checked after a parent directory it would automatically be removed.
 * - Record the amount of directories and hashes were removed. Print these to
 * the console.
 */

typedef directory_table_row_const **parent_directory_map;
typedef directory_table_row_const const *const
    *const parent_directory_map_const;

parent_directory_map
buildDirectoryRowMap(directory_table_row::rows const &directory_table_rows) {
  parent_directory_map map =
      new directory_table_row_const *[directory_table_rows.size() + 1];

  for (directory_table_row const &row : directory_table_rows) {
    map[row.id] = &row;
  }

  return map;
}

std::string buildRelativePath(hash_table_row const &hash_row,
                              parent_directory_map_const directory_map) {

  std::vector<std::string> path_segments{hash_row.name};

  int parent_directory_id = hash_row.directory_id;
  while (parent_directory_id != -1) {
    directory_table_row_const &parent_row = *directory_map[parent_directory_id];
    path_segments.insert(path_segments.begin(), parent_row.name);
    parent_directory_id = parent_row.parent_id;
  }

  return joinPath(path_segments);
}

std::vector<int>
determineHashesToDelete(hash_table_row::rows const &hashes,
                        parent_directory_map_const &directory_map,
                        str_const root_dir) {
  std::vector<int> hash_ids_to_remove{};
  std::string absolute_path{root_dir};

  for (hash_table_row hash_row : hashes) {
    std::string file_path =
        joinPath({absolute_path, buildRelativePath(hash_row, directory_map)});
    if (!fileExists(file_path)) {
      hash_ids_to_remove.push_back(hash_row.id);
    }
  }

  return hash_ids_to_remove;
}

void update(std::string cache_path, std::ostream &console) {
  sqlite3 *db = initDB(cache_path.c_str());
  scan_meta_data_table_row meta_data_row = fetchScanMetaData(db);
  directory_table_row::rows directory_table_rows = fetchAllDirectories(db);
  hash_table_row::rows hash_table_rows = fetchAllHashes(db);

  parent_directory_map_const directory_map =
      buildDirectoryRowMap(directory_table_rows);

  std::vector<int> hash_ids_to_delete = determineHashesToDelete(
      hash_table_rows, directory_map, meta_data_row.root_dir);

  for (int hash_id_to_delete : hash_ids_to_delete) {
    deleteHash(db, hash_id_to_delete);
  }

  console << "Deleted a total of " << hash_ids_to_delete.size()
          << " hashes from the cache. These represent files hashed on the file "
             "system. Empty directories are automatically filtered when "
             "running the dupes "
             "command.\n";
  freeDB(db);
}