
#include "dupes.h"

#include "./extract.h"
#include "./load.h"
#include "./transform.h"

void dupes(std::vector<std::string> paths, std::ostream &console,
           bool build = false) {
  sqlite3 *db =
      initDB("sort_msi_windows_desktop_DDrive_AND_Desktop_AND_User.hash");
  if (build) {
    buildCache(paths, db);
  }
  file_hash_rows extraction_results = extractUsingCache(db);
  console
      << "Done extracting the files from the SQLite Cache. Total Directories "
      << extraction_results.directory_rows.size()
      << " Total Hashes: " << extraction_results.hash_rows.size() << '\n'
      << std::endl;
  duplicate_path_seg_set transformation_results = transform(extraction_results);

  load(console, transformation_results);
  freeDB(db);
}
