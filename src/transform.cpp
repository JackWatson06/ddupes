#include "transform.h"

#include <iostream>
#include <queue>

/**
 * NOTE: Thoughts and Ideas:
 * - The hash table will only have to be as large as the amount of files
 * and directories we have. I think this may be idealistic though since the
 * hash tables key space is larger than the actual slots.
 * - I may never have to delete the in memory hash data unless we convert
 * to a string for the hash map. Only delete then! Even though it would
 * be best to delete all the hash data in one go. I could get a page
 * of memory for this.
 */

bool inode::operator==(inode const &rhs) const {
  return compareStrings(path_segment, rhs.path_segment) &&
         compareHashes(node_hash, rhs.node_hash) && inodes == rhs.inodes &&
         parent_node == rhs.parent_node;
}

std::size_t inode_hasher::operator()(hash_const k) const {
  std::size_t hash = 0;

  for (uint i = 0; i < 8; ++i) {
    hash |= (std::size_t)k[i] << (64 - (8 * (i + 1)));
  }

  return hash;
}

bool inode_key_equal::operator()(hash_const lhs, hash_const rhs) const {
  return compareHashes(lhs, rhs);
}

parent_directory_map
buildParentDirectoryMap(directory_table_row::rows const &directory_table_rows) {
  parent_directory_map map =
      new std::vector<directory_table_row_const *>[directory_table_rows.size() +
                                                   1] {};

  for (int i = 0; i < directory_table_rows.size(); ++i) {
    directory_table_row const &table_row = directory_table_rows[i];
    if (table_row.parent_id == -1) {
      continue;
    }

    if (table_row.parent_id > directory_table_rows.size()) {
      return map;
    }

    map[table_row.parent_id].push_back(&table_row);
  }

  return map;
}

parent_hash_map buildParentHashMap(hash_table_row::rows const &hash_table_rows,
                                   int num_of_directories) {
  parent_hash_map map =
      new std::vector<hash_table_row_const *>[num_of_directories + 1];

  for (int i = 0; i < hash_table_rows.size(); ++i) {
    hash_table_row const &table_row = hash_table_rows[i];

    if (table_row.directory_id > num_of_directories) {
      return map;
    }

    map[table_row.directory_id].push_back(&table_row);
  }

  return map;
}

inode buildINodeTree(parent_directory_map_const directory_map,
                     parent_hash_map_const hash_map,
                     directory_table_row_const *const current_directory_row,
                     int depth) {
  inode inode_tree{depth, stringDup(current_directory_row->name), nullptr};
  // Creates a new inode on the stack. Takes the size of the sizeof(char
  // const*), sizeof(hash), and sizeof(vector).

  ++depth;
  for (hash_table_row_const *hash_table_row :
       hash_map[current_directory_row->id]) {
    inode_tree.inodes.push_back({
        depth,
        stringDup(hash_table_row->name),
        hashDup(hash_table_row->hash),
    }); // Creates an inode on the heap. It does a malloc in push_back
  }

  for (directory_table_row_const *directory_table_row :
       directory_map[current_directory_row->id]) {
    inode_tree.inodes.push_back(
        buildINodeTree(directory_map, hash_map, directory_table_row, depth));
    // Creates an inode on the heap. It does a malloc in push_back;
  }

  return inode_tree; // Copy the return value into whatever variable we are
                     // assinging to from the caller.
}

void addInodePointerToHashMap(inode &directory_tree,
                              hash_inode_map &duplicate_inodes_map) {
  // TODO Does this make a copy of the hash? Should I just put in a poitner to
  // the hash on the original inode?
  if (duplicate_inodes_map.count(directory_tree.node_hash) == 0) {
    duplicate_inodes_map[directory_tree.node_hash] = {&directory_tree};
    return;
  }

  duplicate_inodes_map[directory_tree.node_hash].push_back(&directory_tree);
  return;
}

void removeEmptyINodes(inode &directory_tree) {
  if (directory_tree.inodes.size() == 0) { // At leaf node.
    return;
  }

  for (inode &leaf_node : directory_tree.inodes) {
    removeEmptyINodes(leaf_node);
  }

  for (auto iter = directory_tree.inodes.begin();
       iter != directory_tree.inodes.end();) {
    bool is_empty_folder =
        (*iter).node_hash == nullptr && (*iter).inodes.size() == 0;
    bool is_empty_file = compareHashes((*iter).node_hash, EMPTY_HASH);

    if (is_empty_file || is_empty_folder) {
      directory_tree.inodes.erase(iter);
      continue;
    }
    ++iter;
  }
}

hash calculateINodeHashesRecursive(inode &directory_tree,
                                   hash_inode_map &duplicate_inodes_map,
                                   inode const *parent_pointer = nullptr) {
  directory_tree.parent_node = parent_pointer;

  if (directory_tree.inodes.size() == 0 &&
      directory_tree.node_hash == nullptr) {
    directory_tree.node_hash = new uint8_t[MD5_DIGEST_LENGTH]{};
  }

  if (directory_tree.inodes.size() == 0) {
    addInodePointerToHashMap(directory_tree, duplicate_inodes_map);
    return directory_tree.node_hash;
  }

  int num_of_hashes = directory_tree.inodes.size();
  hashes sub_node_hashes = new hash[num_of_hashes];

  for (int i = 0; i < directory_tree.inodes.size(); ++i) {
    sub_node_hashes[i] = calculateINodeHashesRecursive(
        directory_tree.inodes[i], duplicate_inodes_map, &directory_tree);
  }

  directory_tree.node_hash = computeHash(sub_node_hashes, num_of_hashes);
  addInodePointerToHashMap(directory_tree, duplicate_inodes_map);
  delete[] sub_node_hashes;

  return directory_tree.node_hash;
}

hash_inode_map *calculateHashes(inode &directory_tree) {
  hash_inode_map *duplicate_inodes_map = new hash_inode_map;
  calculateINodeHashesRecursive(directory_tree, *duplicate_inodes_map);
  return duplicate_inodes_map;
}

int countShortestDepth(std::vector<inode const *> const &inode_references) {
  if (inode_references.size() == 0) {
    return 0;
  }

  int min_size = INT32_MAX;

  for (auto inode_reference : inode_references) {
    if (inode_reference->depth < min_size) {
      min_size = inode_reference->depth;
    }
  }

  return min_size;
}

/**
 * In order to determine if any of the inodes have a shared parent they all must
 * be at the same depth. When checking if inodes have a shared parent you only
 * have to check the depth of shallowist node.
 */
std::vector<inode const *>
fastForwardINodeReferences(std::vector<inode const *> const inode_references) {
  int shortest_path = countShortestDepth(inode_references);

  if (shortest_path == 0) {
    return inode_references;
  }

  // Skip the last element because this will always be shared across all the
  // duplicate paths.
  std::vector<inode const *> current_inode_references{};
  for (auto iter = inode_references.cbegin(); iter != inode_references.end();
       ++iter) {

    inode const *current_inode_reference = (*iter);

    while (current_inode_reference->depth != shortest_path) {
      current_inode_reference = current_inode_reference->parent_node;
    }

    current_inode_references.push_back(current_inode_reference->parent_node);
  }
  return current_inode_references;
}

/**
 *
 * This code will essentially take a list of list of node pointers each
 * representing a group of node pointers with the same hash. If any of the node
 * pointers share a common ancestor that is also a duplicate (meaning it's in a
 * duplicate folder) then you remove that from the list. Otherwise, if one
 * ancestor does not share a ancestor with a duplicate hash then you keep those
 * in the list because even if other entries are in a duplicate folder the fact
 * that no ancestors had the same folder hash means that one is outside the
 * duplicate folder which we want to list.
 */
bool hasSharedParent(std::vector<inode const *> const inode_references,
                     hash_inode_map const *hash_to_duplicate_nodes) {
  if (inode_references.size() == 0) {
    return false;
  }

  std::vector<inode const *> fast_forwarded_references =
      fastForwardINodeReferences(inode_references);

  // Skip the leaf node of the shortest

  while (fast_forwarded_references[0] != nullptr) {
    bool all_have_same_hash = true;
    hash const &first_hash = fast_forwarded_references[0]->node_hash;
    fast_forwarded_references[0] = fast_forwarded_references[0]->parent_node;

    for (auto iter = fast_forwarded_references.begin() + 1;
         iter != fast_forwarded_references.end(); ++iter) {
      all_have_same_hash = compareHashes((*iter)->node_hash, first_hash);
      (*iter) = (*iter)->parent_node;
    }

    // Do all the nodes have the same hash and is the node actually duplicated
    // with another?
    if (all_have_same_hash && hash_to_duplicate_nodes->count(first_hash) != 0) {
      return true;
    }
  }

  return false;
}

void filterNonDupesRecursive(inode const &node_to_process,
                             hash_inode_map *hash_inode_map) {
  // Post-order DFS.
  if (node_to_process.inodes.size() > 0) {
    for (auto node : node_to_process.inodes) {
      filterNonDupesRecursive(node, hash_inode_map);
    }
  }

  // Hash was already removed or Hash has duplicates.
  if (hash_inode_map->count(node_to_process.node_hash) == 0 ||
      hash_inode_map->at(node_to_process.node_hash).size() != 1) {
    return;
  }

  // Hash does not have duplicates so remove.
  hash_inode_map->erase(node_to_process.node_hash);
}

// TODO: Should this be moved to the view? I would argue that we have already
// found the duplicates once we have a vector of all the duplicated node
// references.
duplicate_path_segments buildDuplicatePathSegments(
    std::vector<inode const *> const &duplicate_inode_references) {
  duplicate_path_segments
      duplicate_inodes_result{}; // Create a new vector on stack.

  for (auto inode_reference : duplicate_inode_references) {
    // TODO: This could just be a staic array with the length (if we calc depth
    // on calculateHashes)
    std::vector<const char *>
        path_segments{}; // Create vector on stack and copy the
                         // string from the inode tree.

    inode const *current_inode_pointer = inode_reference;

    while (current_inode_pointer != nullptr) {
      path_segments.insert(
          path_segments.begin(),
          stringDup(current_inode_pointer
                        ->path_segment)); // Create copy of string and push the
      // pointer to the vector.
      current_inode_pointer = current_inode_pointer->parent_node;
    }

    duplicate_inodes_result.push_back(
        path_segments); // Push copy of the path_segments vector.
    // path_segments vector goes out of scope. Call destructor (does not delete
    // the string only deletes it's pointer)
  }

  return duplicate_inodes_result;
}

void filterNestedHashesRecursive(inode const &node_to_process,
                                 hash_inode_map *hash_inode_map,
                                 duplicate_path_seg_set &duplicate_nodes) {
  // Post-order DFS.
  if (node_to_process.inodes.size() > 0) {
    for (auto node : node_to_process.inodes) {
      filterNestedHashesRecursive(node, hash_inode_map, duplicate_nodes);
    }
  }

  if (hash_inode_map->count(node_to_process.node_hash) == 0) {
    // Already process the node previously since it was removed from the hash
    // map.
    return;
  }

  std::vector<inode const *> duplicate_inodes =
      (*hash_inode_map)[node_to_process.node_hash];

  if (duplicate_inodes.size() < 2) {
    // Nothing to do. The node was not a duplicate or it was just created.
    return;
  }

  if (hasSharedParent(duplicate_inodes, hash_inode_map)) {
    hash_inode_map->erase(node_to_process.node_hash);
    return;
  }

  // Add the current duplicate_inodes to duplicate_nodes. These are actual
  // duplicates.
  hash_inode_map->erase(node_to_process.node_hash);
  duplicate_nodes.push_back(buildDuplicatePathSegments(duplicate_inodes));
}

duplicate_path_seg_set
filterNonDupsAndNestedHashes(inode const &root_node,
                             hash_inode_map *hash_inode_map) {

  filterNonDupesRecursive(root_node, hash_inode_map);

  duplicate_path_seg_set duplicate_nodes_set{};
  filterNestedHashesRecursive(root_node, hash_inode_map, duplicate_nodes_set);
  return duplicate_nodes_set;
}

duplicate_path_seg_set transform(file_hash_rows const &file_hashes) {
  parent_directory_map directory_map =
      buildParentDirectoryMap(file_hashes.directory_rows);
  parent_hash_map hash_map = buildParentHashMap(
      file_hashes.hash_rows, file_hashes.directory_rows.size());
  inode root_inode =
      buildINodeTree(directory_map, hash_map, &file_hashes.directory_rows[0]);

  removeEmptyINodes(root_inode);
  hash_inode_map *hash_to_inode_map = calculateHashes(root_inode);

  return filterNonDupsAndNestedHashes(root_inode, hash_to_inode_map);
}
