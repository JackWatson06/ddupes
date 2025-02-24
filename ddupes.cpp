#include <iostream>

#include "extract.h"
#include "load.h"
#include "transform.h"

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
** TODO **
- Fix this logic error:

When running the command with the directory inputs: "tests/testing_dirs/dir2",
"tests/testing_dirs/dir3", "tests/testing_dirs/dir1"

I get this output.
//home/jack/WorkingDirectory/ddupes/tests/testing_dirs/dir1/sub_dir
//home/jack/WorkingDirectory/ddupes/tests/testing_dirs/dir2/sub_dir

This is not true. dir1/sub_dir has two files and dir2/sub_dir only has one file.

- Remove the SQLLite cache at the end.
- Having a filter to return the folder I am specifically looking at would be
wonderful.
- How can we get the home directory of the current user when finding the hash.db
file.
- Integration tests with the SQLLite query classes.
- Improve error handling for sqlite
- Do a map of the .h file dependencies
- Double check the nullptr check for the hash check.
*/

int main() {
  try {
    FileHashRows extraction_results =
        extract({"tests/testing_dirs/dir2", "tests/testing_dirs/dir3",
                 "tests/testing_dirs/dir1"},
                "/home/jack/.cache/fdupes/hash.db");
    DuplicateINodesSet transformation_results = transform(extraction_results);
    load(std::cout, transformation_results);
  } catch (const std::runtime_error& e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
