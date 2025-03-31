#include <iostream>

#include "src/extract.h"
#include "src/fs/file_system.h"
#include "src/load.h"
#include "src/sqlite/sqlite.h"
#include "src/transform.h"

/*
** Functionality and Design **
- Take in the directories that we want to use to find the ddupes for.
- Run the fdupes command over the set of the directories. I wonder how I can
exec in c++
- Move and remove the cache created by fdupes into a temporary directory.
- Get a list of all the directories from the SQLLite
- Simple layer structure. Domain layer has hashing logic, and structs.
- I think I actually need a tree structure for this. Basically I create the tree
structure then I
- Have an array of three directory trees in memory.
- Loop through the leaf nodes. If parent has a hash somewhere in the system then
continue up the tree until I find a unique directroy.
- Depth First Search and Hashmap for Hashes to nodes.
- When I find a unique directory then don't navigate up to parent and clip this
branch off the tree and store the full path of the duplicate directory or file.
- Store the duplicate directories in a grouped result set.
- Since a function could call delete on an object the desctuctors are part of
the objects interface A function does not construct an object of the passed in
type. The object will already be constructed. When it's passed in.
*/

/**
** TODO: **
- Improve file reading speed.
- Fix this logic error:
- Remove the SQLLite cache at the end.
- Having a filter to return the folder I am specifically looking at would be
wonderful.
- How can we get the home directory of the current user when finding the hash.db
file.
- Improve error handling for sqlite
- Do a map of the .h file dependencies.
*/

int main() {
  try {
    string_vector paths = {"tests/testing_dirs/dir1",
                           "tests/testing_dirs/dir2"};
    sqlite3 *db = initDB("testing.hash");

    buildCache(paths,
               create_cache_database_service{.db = db,
                                             .resetDB = resetDB,
                                             .createDirectory = createDirectory,
                                             .createHash = createHash},
               create_cache_file_system_service{.qualifyRelativePath =
                                                    qualifyRelativeURL,
                                                .visitFiles = visitFiles,
                                                .extractHash = extractHash});

    file_hash_rows extraction_results = extractUsingCache(
        extract_database_service{.db = db,
                                 .fetchAllDirectories = fetchAllDirectories,
                                 .fetchAllHashes = fetchAllHashes});
    std::cout
        << "Done extracting the files from the SQLite Cache. Total Directories "
        << extraction_results.directory_rows.size()
        << " Total Hashes: " << extraction_results.hash_rows.size() << '\n'
        << std::endl;
    duplicate_path_seg_set transformation_results =
        transform(extraction_results);

    load(std::cout, transformation_results);

    freeDB(db);
  } catch (const std::runtime_error &e) {
    std::cerr << e.what() << std::endl;
  } catch (...) {
    std::cerr << "Unexpected error." << std::endl;
  }

  return 0;
}
