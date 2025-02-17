#include "transform.h"

#include <openssl/evp.h>

#include <queue>

std::string DirectoryNode::getName() const { return name; }

FileNode::Files DirectoryNode::getFiles() const { return files; }

DirectoryNode::Directories DirectoryNode::getDirectories() const {
  return directories;
}

bool DirectoryNode::operator==(const DirectoryNode &rhs) const {
  return rhs.directories == directories && rhs.files == files &&
         rhs.name == name;
}

std::string HashNode::getName() const { return name; }

Hash HashNode::getHash() const { return hash; }

HashNode::HashedNodes HashNode::getHashedNodes() const { return hashed_nodes; }

bool HashNode::operator==(const HashNode &rhs) const {
  return rhs.hashed_nodes == hashed_nodes && rhs.hash == hash &&
         rhs.name == name;
}

bool HashPathSegment::operator==(const HashPathSegment &rhs) const {
  return rhs.hash_string == hash_string && rhs.path_segment == path_segment;
}

Hash computeHash(Hashes hashes) {
  EVP_MD_CTX *md_context;
  unsigned char *md5_digest;
  unsigned int md5_digest_len = EVP_MD_size(EVP_md5());

  md_context = EVP_MD_CTX_new();
  EVP_DigestInit_ex(md_context, EVP_md5(), nullptr);

  for (const Hash file_hash : hashes) {
    EVP_DigestUpdate(md_context, file_hash.data(), file_hash.size());
  }

  md5_digest = (unsigned char *)OPENSSL_malloc(md5_digest_len);
  EVP_DigestFinal_ex(md_context, md5_digest, &md5_digest_len);
  EVP_MD_CTX_free(md_context);

  std::vector<uint8_t> hash_of_hashes(md5_digest, md5_digest + md5_digest_len);

  OPENSSL_free(md5_digest);
  return hash_of_hashes;
}

DirectoryRowIdMap buildDirectoryRowIdMap(
    const DirectoryTableRow::Rows &directory_table_rows) {
  DirectoryRowIdMap directory_id_map;

  for (const DirectoryTableRow &row : directory_table_rows) {
    directory_id_map[row.id] = &row;
  }

  return directory_id_map;
}

Hash blobToHash(const void *blob) {
  uint8_t *uint_blob = (uint8_t *)blob;
  Hash hash{};
  for (uint8_t *iter = uint_blob; iter != uint_blob + MD5_DIGEST_LENGTH;
       ++iter) {
    hash.push_back(*iter);
  }
  return hash;
}

DirectoryNode buildFileNodeBranch(const HashTableRow &hash_result,
                                  const DirectoryRowIdMap &directory_id_map) {
  const DirectoryTableRow *next_directory_pointer =
      directory_id_map.at(hash_result.directory_id);
  DirectoryNode root_node{
      next_directory_pointer->name,
      {FileNode{hash_result.name, blobToHash(hash_result.hash)}}};

  while (next_directory_pointer->parent_id != -1) {
    next_directory_pointer =
        directory_id_map.at(next_directory_pointer->parent_id);
    root_node = DirectoryNode{next_directory_pointer->name, {}, {root_node}};
  }

  return root_node;
}

FileNode::Files mergeTwoFileLists(const FileNode::Files &files_one,
                                  const FileNode::Files &files_two) {
  std::unordered_map<std::string, bool> files_already_merged{};
  FileNode::Files merged_files = files_one;

  for (const FileNode &file : merged_files) {
    files_already_merged[file.getName()] = true;
  }

  for (const FileNode &file : files_two) {
    // The file does not exist in the list.
    if (files_already_merged.find(file.getName()) ==
        files_already_merged.end()) {
      merged_files.push_back(file);
    }
  }

  return merged_files;
}

DirectoryNode::Directories mergeTwoDirectoryLists(
    const DirectoryNode::Directories &directories_one,
    const DirectoryNode::Directories &directories_two) {
  std::unordered_map<std::string, unsigned int> merged_directories_index;
  DirectoryNode::Directories merged_directories = directories_one;

  for (unsigned int i = 0; i < merged_directories.size(); i++) {
    merged_directories_index[merged_directories[i].getName()] = i;
  }

  for (const DirectoryNode &directory_node : directories_two) {
    // Directory already in the merged_directories.
    if (merged_directories_index.find(directory_node.getName()) !=
        merged_directories_index.end()) {
      unsigned int merged_index =
          merged_directories_index[directory_node.getName()];
      DirectoryNode &directory_already_merged =
          merged_directories[merged_index];
      merged_directories[merged_index] =
          mergeTwoDirectoryNodes(directory_node, directory_already_merged);
      continue;
    }

    merged_directories.push_back(directory_node);
  }

  return merged_directories;
}

DirectoryNode mergeTwoDirectoryNodes(const DirectoryNode &directory_node_one,
                                     const DirectoryNode &directory_node_two) {
  if (directory_node_one.getName() != directory_node_two.getName()) {
    return DirectoryNode{""};
  }

  FileNode::Files node_one_files = directory_node_one.getFiles();
  DirectoryNode::Directories node_one_directories =
      directory_node_one.getDirectories();
  FileNode::Files node_two_files = directory_node_two.getFiles();
  DirectoryNode::Directories node_two_directories =
      directory_node_two.getDirectories();

  return DirectoryNode{
      directory_node_one.getName(),
      mergeTwoFileLists(node_one_files, node_two_files),
      mergeTwoDirectoryLists(node_one_directories, node_two_directories)};
}

DirectoryNode buildDirectoryTree(
    const HashTableRow::Rows &hash_table_rows,
    const DirectoryRowIdMap &directory_table_rows) {
  if (directory_table_rows.size() == 0 || hash_table_rows.size() == 0) {
    return DirectoryNode("");
  }

  DirectoryNode tree =
      buildFileNodeBranch(hash_table_rows[0], directory_table_rows);

  for (int i = 1; i < hash_table_rows.size(); i++) {
    DirectoryNode file_node_branch =
        buildFileNodeBranch(hash_table_rows[i], directory_table_rows);
    tree = mergeTwoDirectoryNodes(file_node_branch, tree);
  }

  return tree;
}

Hash computeHashNodesHash(const HashNode::HashedNodes nodes) {
  Hashes hashes = {};

  for (const HashNode &hash_node : nodes) {
    hashes.push_back(hash_node.getHash());
  }

  return computeHash(hashes);
}

HashNode::HashedNodes buildHashNodes(const FileNode::Files &file_nodes) {
  HashNode::HashedNodes hash_nodes(file_nodes.size());

  for (int i = 0; i < hash_nodes.size(); i++) {
    hash_nodes[i] = file_nodes[i];
  }

  return hash_nodes;
}

HashNode buildHashNode(const DirectoryNode &directory_node) {
  HashNode::HashedNodes hash_nodes = buildHashNodes(directory_node.getFiles());

  if (directory_node.getDirectories().size() == 0) {
    return HashNode{directory_node.getName(), computeHashNodesHash(hash_nodes),
                    hash_nodes};
  }

  for (const DirectoryNode &directory_node : directory_node.getDirectories()) {
    hash_nodes.push_back(buildHashNode(directory_node));
  }

  return HashNode{directory_node.getName(), computeHashNodesHash(hash_nodes),
                  hash_nodes};
}

std::string buildStringFromHash(const Hash &hash) {
  std::string hash_string = "";

  for (const uint8_t byte : hash) {
    hash_string += (char)byte;
  }

  return hash_string;
}

void addHashStringToHashDuplicateNodesMap(
    const std::string &hash_string, const HashPath &relative_path,
    HashToDuplicateNodes &hash_to_duplicate_nodes) {
  if (hash_to_duplicate_nodes.map.find(hash_string) ==
      hash_to_duplicate_nodes.map.end()) {
    hash_to_duplicate_nodes.map[hash_string] = {relative_path};
    hash_to_duplicate_nodes.order.push_back(hash_string);
    return;
  }

  hash_to_duplicate_nodes.map[hash_string].push_back(relative_path);
}

HashPath popOffParentPathQueue(std::queue<HashPath> &parent_path) {
  if (parent_path.empty()) {
    return {};
  }

  HashPath parent_hash_path_segment = parent_path.front();
  parent_path.pop();
  return parent_hash_path_segment;
}

HashToDuplicateNodes buildHashToDuplicateNodesMap(const HashNode &hash_node) {
  HashToDuplicateNodes hash_to_duplicate_nodes;
  std::queue<HashNode> hash_queue;
  std::queue<HashPath> parent_path;

  HashPath root_hash_path{{}};
  hash_queue.emplace(hash_node);

  while (!hash_queue.empty()) {
    HashNode next_hash_node = hash_queue.front();
    hash_queue.pop();

    std::string current_hash_string =
        buildStringFromHash(next_hash_node.getHash());
    HashPath current_path = popOffParentPathQueue(parent_path);
    current_path.push_back({next_hash_node.getName(), current_hash_string});

    addHashStringToHashDuplicateNodesMap(current_hash_string, current_path,
                                         hash_to_duplicate_nodes);

    for (const HashNode &child_hash_node : next_hash_node.getHashedNodes()) {
      hash_queue.emplace(child_hash_node);
      parent_path.emplace(current_path);
    }
  }

  return hash_to_duplicate_nodes;
}

void filterNonDuplicatesFromDupNodesMap(
    HashToDuplicateNodes &hash_to_duplicate_nodes) {
  for (auto iter = hash_to_duplicate_nodes.order.begin();
       iter != hash_to_duplicate_nodes.order.end();) {
    if (hash_to_duplicate_nodes.map.at(*iter).size() != 1) {
      ++iter;
      continue;
    }
    hash_to_duplicate_nodes.map.erase(*iter);
    iter = hash_to_duplicate_nodes.order.erase(iter);
  }
}

int countShortestDuplicatePath(const DuplicatePaths &duplicate_paths) {
  if (duplicate_paths.size() == 0) {
    return 0;
  }

  int shortest_vector = duplicate_paths[0].size();
  for (const HashPath &hash_path : duplicate_paths) {
    if (hash_path.size() < shortest_vector) {
      shortest_vector = hash_path.size();
    }
  }

  return shortest_vector;
}

bool hasSharedParent(const DuplicatePaths &paths,
                     const DuplicatePathsMap &hash_to_duplicate_nodes) {
  int shortest_path = countShortestDuplicatePath(paths);

  if (shortest_path == 0) {
    return false;
  }

  // Skip the last element because this will always be shared across all the
  // duplicate paths.
  for (unsigned int i = 0; i < shortest_path - 1; i++) {
    bool all_have_same_hash = true;
    std::string first_hash_string = paths[0][i].hash_string;

    for (auto iter = paths.cbegin(); iter != paths.end(); ++iter) {
      all_have_same_hash = (*iter)[i].hash_string == first_hash_string;
    }

    if (all_have_same_hash && hash_to_duplicate_nodes.find(first_hash_string) !=
                                  hash_to_duplicate_nodes.end()) {
      return true;
    }
  }

  return false;
}

void filterSharedHashNodes(HashToDuplicateNodes &hash_to_duplicate_nodes) {
  DuplicatePathsMap no_shared_hash_nodes{};

  for (auto iter = hash_to_duplicate_nodes.order.begin();
       iter != hash_to_duplicate_nodes.order.end();) {
    if (!hasSharedParent(hash_to_duplicate_nodes.map[*iter],
                         no_shared_hash_nodes)) {
      no_shared_hash_nodes[*iter] = hash_to_duplicate_nodes.map[*iter];
      ++iter;
      continue;
    }
    hash_to_duplicate_nodes.map.erase(*iter);
    iter = hash_to_duplicate_nodes.order.erase(iter);
  }
}

SVector buildPathsFromHashPath(const HashPath &path) {
  SVector paths = {};

  for (auto iter = path.cbegin(); iter != path.end(); ++iter) {
    paths.push_back(iter->path_segment);
  }

  return paths;
}
DuplicateINodesSet buildDuplicateINodeSet(
    const DuplicatePathsMap &duplicate_nodes) {
  DuplicateINodesSet duplicate_i_node_set;

  for (auto iter = duplicate_nodes.begin(); iter != duplicate_nodes.end();
       ++iter) {
    Paths paths;
    for (auto path_iter = iter->second.cbegin();
         path_iter != iter->second.end(); ++path_iter) {
      paths.push_back(buildPathsFromHashPath(*path_iter));
    }

    duplicate_i_node_set.push_back(paths);
  }

  return duplicate_i_node_set;
}

DuplicateINodesSet transform(const FileHashRows &file_hash_rows) {
  // Building Directory Map
  DirectoryRowIdMap directory_row_id_map =
      buildDirectoryRowIdMap(file_hash_rows.directory_rows);

  DirectoryNode directory_tree =
      buildDirectoryTree(file_hash_rows.hash_rows, directory_row_id_map);

  // Compute all hashes
  HashNode directory_hash_tree = buildHashNode(directory_tree);

  // Find Hashes
  HashToDuplicateNodes hash_to_nodes =
      buildHashToDuplicateNodesMap(directory_hash_tree);
  filterNonDuplicatesFromDupNodesMap(hash_to_nodes);
  filterSharedHashNodes(hash_to_nodes);

  // Output
  DuplicateINodesSet duplicate_i_node_set =
      buildDuplicateINodeSet(hash_to_nodes.map);
  return duplicate_i_node_set;
}
