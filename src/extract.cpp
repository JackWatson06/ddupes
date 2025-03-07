#include "extract.h"

#include "./constants.h"
#include "./fs/file_system.h"

/**
 *
 *
 * TODO:
 * - Test to make sure we don't pass in the same path.
 *
 *
 */

constexpr char DELIMITER = '/';

struct FileVisitorServices {
  CreateCacheDatabaseServices db;
  CreateCacheFileSystemServices fs;
  std::string relative_argument_path;
  std::vector<int> directory_stack;
  int depth;
};

bool ArgumentPath::operator==(const ArgumentPath& rhs) const {
  return rhs.relative_path == relative_path &&
         rhs.canonicalized_path_tokens == canonicalized_path_tokens;
}

bool RootCalcResult::operator==(const RootCalcResult& rhs) const {
  return rhs.root_path == root_path && rhs.argument_paths == argument_paths;
}

void copyFromSecondVector(SVector& vector_one, SVector& vector_two) {
  for (const std::string string : vector_two) {
    vector_one.push_back(string);
  }
}

std::string removeBeginningSlash(std::string& path) {
  if (path.at(0) == '/') {
    std::string relative_path;
    for (int i = 1; i < path.size(); ++i) {
      relative_path += path[i];
    }
    return relative_path;
  }

  return path;
}

SVector tokenizeRootPath(std::string& path) {
  SVector tokens = {};
  if (path.size() == 0) {
    return tokens;
  }

  tokens.push_back("/");

  SVector tokenized_relative_path = tokenizeRelativePath(path);
  copyFromSecondVector(tokens, tokenized_relative_path);
  return tokens;
}

SVector tokenizeRelativePath(std::string& path) {
  if (path.size() == 0) {
    return {};
  }

  SVector tokens = {};
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

int countShortestTokenizePath(std::vector<SVector>& argument_paths) {
  if (argument_paths.size() == 0) {
    return 0;
  }

  int shortest_path = argument_paths[0].size();

  for (const SVector argument_path : argument_paths) {
    if (argument_path.size() < shortest_path) {
      shortest_path = argument_path.size();
    }
  }

  return shortest_path;
}

RootCalcResult calcRootPath(SVector& relative_argument_paths,
                            CreateCacheFileSystemServices fs_services) {
  SVector qualified_paths{};
  for (std::string path : relative_argument_paths) {
    qualified_paths.push_back(fs_services.qualifyRelativePath(path));
  }

  std::vector<SVector> tokenized_paths{};
  for (std::string qualified_path : qualified_paths) {
    tokenized_paths.push_back(tokenizeRootPath(qualified_path));
  }

  std::vector<ArgumentPath> argument_paths{};
  for (int i = 0; i < tokenized_paths.size(); ++i) {
    argument_paths.push_back({relative_argument_paths[i], {}});
  }

  int shortest_path = countShortestTokenizePath(tokenized_paths);
  // This will determine when the paths split off into seperate sub tress.
  std::string* root_path = 0;
  int unequal_index = 0;
  for (; unequal_index < shortest_path; ++unequal_index) {
    bool same_token = false;
    std::string* tmp_path_segment = &tokenized_paths[0][unequal_index];
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

std::string removeLeadingRelativePath(const std::string& leading_relative_path,
                                      const std::string& path) {
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

void fileVisitorCallback(const std::string path, const enum FileType type,
                         void* services) {
  FileVisitorServices* file_visitor_services =
      static_cast<FileVisitorServices*>(services);

  std::string path_without_relative = removeLeadingRelativePath(
      file_visitor_services->relative_argument_path, path);
  SVector tokenize_path = tokenizeRelativePath(path_without_relative);
  std::string file_node_name = tokenize_path[tokenize_path.size() - 1];

  if (file_visitor_services->depth != tokenize_path.size() - 1) {
    --file_visitor_services->depth;
    file_visitor_services->directory_stack.pop_back();
  }

  if (type == FILE_TYPE_FILE) {
    uint8_t file_hash[MD5_DIGEST_LENGTH];
    file_visitor_services->fs.extractHash(file_hash, path);
    file_visitor_services->db.createHash(
        file_visitor_services->db.db,
        {.directory_id = file_visitor_services->directory_stack.back(),
         .name = file_node_name,
         .hash = file_hash});
    return;
  }

  ++file_visitor_services->depth;
  int directory_id = file_visitor_services->db.createDirectory(
      file_visitor_services->db.db,
      {.parent_id = file_visitor_services->directory_stack.back(),
       .name = file_node_name});
  file_visitor_services->directory_stack.push_back(directory_id);
}

void buildCache(SVector& paths, CreateCacheDatabaseServices db_services,
                CreateCacheFileSystemServices fs_services) {
  db_services.resetDB(db_services.db);

  RootCalcResult root_calc_result = calcRootPath(paths, fs_services);

  int root_id = db_services.createDirectory(
      db_services.db, {.parent_id = -1, .name = root_calc_result.root_path});

  std::vector<int> directory_stack{root_id};
  for (const ArgumentPath path : root_calc_result.argument_paths) {
    for (const std::string token : path.canonicalized_path_tokens) {
      directory_stack.push_back(db_services.createDirectory(
          db_services.db,
          {.parent_id = directory_stack.back(), .name = token}));
    }

    FileVisitorServices file_visitor_services{
        db_services, fs_services, path.relative_path, directory_stack, 0};
    fs_services.visitFiles(path.relative_path, fileVisitorCallback,
                           &file_visitor_services);

    directory_stack = {root_id};
  }
}
FileHashRows extractUsingCache(ExtractDatabaseServices db_services) {
  return {db_services.fetchAllDirectories(db_services.db),
          db_services.fetchAllHashes(db_services.db)};
}