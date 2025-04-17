#include "extract.h"

#include "./fs/file_system.h"

/**
 * TODO:
 * - Test to make sure we don't pass in the same path.
 */

constexpr char DELIMITER = '/';

struct file_visitor_services {
  create_cache_database_service db;
  create_cache_file_system_service fs;
  std::string relative_argument_path;
  std::vector<int> directory_stack;
  int depth;
};

bool argument_path::operator==(const argument_path &rhs) const {
  return rhs.relative_path == relative_path &&
         rhs.canonicalized_path_tokens == canonicalized_path_tokens;
}

bool root_calc_result::operator==(const root_calc_result &rhs) const {
  return rhs.root_path == root_path && rhs.argument_paths == argument_paths;
}

void copyFromSecondVector(string_vector &vector_one,
                          string_vector &vector_two) {
  for (const std::string string : vector_two) {
    vector_one.push_back(string);
  }
}

std::string removeBeginningSlash(std::string &path) {
  if (path.at(0) == '/') {
    std::string relative_path;
    for (int i = 1; i < path.size(); ++i) {
      relative_path += path[i];
    }
    return relative_path;
  }

  return path;
}

string_vector tokenizeRootPath(std::string &path) {
  string_vector tokens = {};
  if (path.size() == 0) {
    return tokens;
  }

  tokens.push_back("/");

  string_vector tokenized_relative_path = tokenizeRelativePath(path);
  copyFromSecondVector(tokens, tokenized_relative_path);
  return tokens;
}

string_vector tokenizeRelativePath(std::string &path) {
  if (path.size() == 0) {
    return {};
  }

  string_vector tokens = {};
  std::string path_without_slash = removeBeginningSlash(path);

  std::string temp_string{};
  for (const char character : path_without_slash) {
    if (character == DELIMITER) {
      tokens.push_back(temp_string);
      temp_string = "";
      continue;
    }

    temp_string.push_back(character);
  }

  if (path_without_slash.size() != 0 &&
      path_without_slash.at(path_without_slash.size() - 1) != DELIMITER) {
    tokens.push_back(temp_string);
  }

  return tokens;
}

int countShortestTokenizePath(std::vector<string_vector> &argument_paths) {
  if (argument_paths.size() == 0) {
    return 0;
  }

  int shortest_path = argument_paths[0].size();

  for (const string_vector argument_path : argument_paths) {
    if (argument_path.size() < shortest_path) {
      shortest_path = argument_path.size();
    }
  }

  return shortest_path;
}

root_calc_result calcRootPath(string_vector &relative_argument_paths,
                              create_cache_file_system_service fs_services) {
  string_vector qualified_paths{};
  for (std::string path : relative_argument_paths) {
    qualified_paths.push_back(fs_services.qualifyRelativePath(path));
  }

  std::vector<string_vector> tokenized_paths{};
  for (std::string qualified_path : qualified_paths) {
    tokenized_paths.push_back(tokenizeRootPath(qualified_path));
  }

  std::vector<argument_path> argument_paths{};
  for (int i = 0; i < tokenized_paths.size(); ++i) {
    argument_paths.push_back({relative_argument_paths[i], {}});
  }

  int shortest_path = countShortestTokenizePath(tokenized_paths);
  // This will determine when the paths split off into seperate sub tress.
  std::string *root_path = 0;
  int unequal_index = 0;
  for (; unequal_index < shortest_path; ++unequal_index) {
    bool same_token = false;
    std::string *tmp_path_segment = &tokenized_paths[0][unequal_index];
    for (int j = 1; j < tokenized_paths.size(); ++j) {
      same_token = *tmp_path_segment == tokenized_paths[j][unequal_index];
      if (!same_token) {
        break;
      }
    }

    if (same_token) {
      root_path = tmp_path_segment;
      continue;
    }

    break;
  }

  // Copy the rest of the paths that are not duplicates.
  for (int i = 0; i < tokenized_paths.size(); ++i) {
    for (int j = unequal_index; j < tokenized_paths[i].size(); ++j) {
      argument_paths[i].canonicalized_path_tokens.push_back(
          tokenized_paths[i][j]);
    }
  }

  if (!root_path) {
    return {.root_path = "", .argument_paths = argument_paths};
  }

  return {.root_path = *root_path, .argument_paths = argument_paths};
}

std::string removeLeadingRelativePath(const std::string &leading_relative_path,
                                      const std::string &path) {
  std::string path_without_leading = "";

  int i = 0;
  for (; i < path.size(); ++i) {
    if (i == leading_relative_path.size() ||
        leading_relative_path[i] != path[i]) {
      break;
    }
  }

  for (; i < path.size(); ++i) {
    path_without_leading.push_back(path[i]);
  }

  return path_without_leading;
}

void fileVisitorCallback(const std::string path, const enum file_type type,
                         void *services) {
  file_visitor_services *file_services =
      static_cast<file_visitor_services *>(services);

  std::string path_without_relative =
      removeLeadingRelativePath(file_services->relative_argument_path, path);
  string_vector tokenize_path = tokenizeRelativePath(path_without_relative);
  std::string file_node_name = tokenize_path[tokenize_path.size() - 1];

  if (file_services->depth != tokenize_path.size() - 1) {
    int ascend_depth = (file_services->depth + 1) - tokenize_path.size();
    for (int i = 0; i < ascend_depth; ++i) {
      --file_services->depth;
      file_services->directory_stack.pop_back();
    }
  }

  if (type == FILE_TYPE_FILE) {
    uint8_t file_hash[MD5_DIGEST_LENGTH];
    file_services->fs.extractHash(file_hash, path);
    file_services->db.createHash(
        file_services->db.db,
        {.directory_id = file_services->directory_stack.back(),
         .name = file_node_name.c_str(),
         .hash = file_hash});
    return;
  }

  ++file_services->depth;
  int directory_id = file_services->db.createDirectory(
      file_services->db.db, {.parent_id = file_services->directory_stack.back(),
                             .name = file_node_name.c_str()});
  file_services->directory_stack.push_back(directory_id);
}

void buildCache(string_vector &paths, create_cache_database_service db_services,
                create_cache_file_system_service fs_services) {
  db_services.resetDB(db_services.db);

  root_calc_result root_calc_result = calcRootPath(paths, fs_services);

  int root_id = db_services.createDirectory(
      db_services.db,
      {.parent_id = -1, .name = root_calc_result.root_path.c_str()});

  std::vector<int> directory_stack{root_id};
  for (const argument_path path : root_calc_result.argument_paths) {
    for (const std::string token : path.canonicalized_path_tokens) {
      directory_stack.push_back(db_services.createDirectory(
          db_services.db,
          {.parent_id = directory_stack.back(), .name = token.c_str()}));
    }

    file_visitor_services file_visitor_services{
        db_services, fs_services, path.relative_path, directory_stack, 0};
    fs_services.visitFiles(path.relative_path, fileVisitorCallback,
                           &file_visitor_services);

    directory_stack = {root_id};
  }
}

file_hash_rows extractUsingCache(extract_database_service db_services) {
  return {db_services.fetchAllDirectories(db_services.db),
          db_services.fetchAllHashes(db_services.db)};
}

/**
 *
 * $49 = "/home/jack/Desktop/#inbox/sort_msi_windows_desktop_DDrive/Desktop"
  (gdb) p ((file_visitor_services *)services)->directory_stack
  $50 = std::vector of length 3, capacity 4 = {1, 3, 4}
  (gdb) p ((file_visitor_services *)services)->depth
  $51 = 1

  $37 = "/home/jack/Desktop/#inbox/sort_msi_windows_desktop_DDrive/UserFaces"
  (gdb) p ((file_visitor_services *)services)->directory_stack
  $38 = std::vector of length 2, capacity 2 = {1, 3}
  (gdb) p ((file_visitor_services *)services)->depth


 *
 *
 *
 */
