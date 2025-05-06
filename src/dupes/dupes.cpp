
#include "dupes.h"

#include "./load.h"
#include "./transform.h"

void dupes(std::string cache_path, std::ostream &console) {
  sqlite3 *db = initDB(cache_path.c_str());
  file_hash_rows rows = {fetchAllDirectories(db), fetchAllHashes(db)};
  console
      << "Done extracting the files from the SQLite Cache. Total Directories "
      << rows.directory_rows.size()
      << " Total Hashes: " << rows.hash_rows.size() << '\n'
      << std::endl;
  duplicate_path_seg_set transformation_results = transform(rows);

  load(console, transformation_results);
  freeDB(db);
}
