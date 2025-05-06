#include "./cli.h"
#include <iostream>

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
- Add update command which will update the cache and remove files that were
deleted from the operating system.
- Implement a cli inteface which takes in the operation you want to do (dupes,
build, or update)
  - dupes should take in the cache you are building the duplicate list from.
This will be passed in as a key ex. `--cache inbox`
  - build should take in the list of paths you are building the cache from. It
will also take in a `--cache inbox` option which will build the cache using that
key in the .cache directory.
  - update should take in the cache key and update the cache accordingly
`--cache inbox`

- How can we get the home directory of the current user when finding the hash.db
file.
- Improve error handling for sqlite
- Do a map of the .h file dependencies.
*/

int main(int argc, char *argv[]) {
  try {
    process(argc, argv);
  } catch (const std::runtime_error &e) {
    std::cerr << e.what() << std::endl;
  } catch (...) {
    std::cerr << "Unexpected error." << std::endl;
  }

  return 0;
}

/**
 *
 * extractArgs()
 *
 * validateArgs()
 *
 * switch(arg_type) {
 *
 *  case "dupes":
 *    getCacheName()
 *    validateCacheExists()
 *    sqlite(cache_name)
 *    if (build):
 *      buildUsingCache()
 *    extractFromCache()
 *
 *  case "update":
 *    getCacheName()
 *    validateCacheExists()
 *    sqlite(cache_name)
 *    updateCache()
 *
 *  default:
 *    doPythonScriptStuff()
 * }
 *
 * Cache service?
 *
 */
