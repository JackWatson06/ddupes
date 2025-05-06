#include "./cli.h"
#include "./build/build.h"
#include "./dupes/dupes.h"
#include "./env/env.h"
#include "./fs/file_system.h"
#include "./lib.h"
#include "./update/update.h"
#include <iostream>

char const CACHE_OPTION_NAME[] = "--cache";
char const DUPES_COMMAND_NAME[] = "dupes";
char const BUILD_COMMAND_NAME[] = "build";
char const UPDATE_COMMAND_NAME[] = "update";

char const *parseCacheArgument(int argc, char *argv[]) {
  for (int i = 0; i < argc; ++i) {

    bool cache_found = compareStrings(CACHE_OPTION_NAME, argv[i]);

    if (cache_found && i == argc - 1) {
      throw command_error(
          "'--cache' argument must have the name of the cache.");
    }

    if (cache_found) {
      return argv[i + 1];
    }
  }

  throw command_error("'--cache' argument must be included.");
}

std::vector<std::string> parsePathsArguments(int argc, char *argv[]) {
  std::vector<std::string> path_args;
  for (int i = 2; i < argc;) {
    if (compareStrings(CACHE_OPTION_NAME, argv[i])) {
      i += 2;
      continue;
    }

    path_args.push_back(argv[i]);
    ++i;
  }

  if (path_args.size() == 0) {
    throw command_error("Found no paths to process! Please pass in a list of "
                        "directories to find duplicates in.");
  }

  return path_args;
}

char const *buildChachePath() {
  char const *home_dir = fetchHomeDirectory();

  if (home_dir == nullptr) {
    throw command_error("Could not find the user's home directory!");
  }

  return stringConcat(home_dir, CACHE_DIR);
}

std::string postfixDb(char const *cache_path, char const *cache_arg) {
  char const *db_file = stringConcat(cache_arg, ".db");
  return joinPath({cache_path, db_file});
}

void process(int argc, char *argv[]) {

  if (argc < 2) {
    throw command_error("You must pass in the action! The actions include "
                        "'build', 'dupes', and 'update'.");
  }

  char *action = argv[1];

  // Setup DB file.
  char const *cache_path = buildChachePath();
  createDirectory(cache_path);
  char const *cache_arg = parseCacheArgument(argc, argv);
  std::string db_file = postfixDb(cache_path, cache_arg);

  if (compareStrings(DUPES_COMMAND_NAME, action)) {
    dupes(db_file, std::cout);
    return;
  }

  if (compareStrings(BUILD_COMMAND_NAME, action)) {
    build(parsePathsArguments(argc, argv), db_file, std::cout);
    return;
  }

  if (compareStrings(UPDATE_COMMAND_NAME, action)) {
    update(db_file, std::cout);
    return;
  }

  throw command_error("Invalid action. The allowed actions include "
                      "'build', 'dupes', and 'update'.");
}
