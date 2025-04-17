#include <list>
#include <ostream>
#include <unordered_map>

#include "./extract_output.h"
#include "./lib.h"
#include "./transform_output.h"

typedef std::vector<directory_table_row_const *> *parent_directory_map;
typedef std::vector<directory_table_row_const *> const
    *const parent_directory_map_const;
// TODO How do I type these as const?
typedef std::vector<hash_table_row_const *> *parent_hash_map;
typedef std::vector<hash_table_row_const *> const *const parent_hash_map_const;

struct inode {
  int depth;
  char const *path_segment;
  hash node_hash;
  std::vector<inode> inodes;
  inode const *parent_node;

  bool operator==(inode const &rhs) const;
};

struct inode_hasher {
  std::size_t operator()(hash_const k) const;
};

struct inode_key_equal {
  bool operator()(hash_const lhs, hash_const rhs) const;
};

typedef std::unordered_map<hash, std::vector<inode const *>, inode_hasher,
                           inode_key_equal>
    hash_inode_map;

parent_directory_map buildParentDirectoryMap(directory_table_row::rows const &);
parent_hash_map buildParentHashMap(hash_table_row::rows const &, int);
inode buildINodeTree(parent_directory_map_const, parent_hash_map_const,
                     directory_table_row_const *const, int = 1);
void removeEmptyINodes(inode &directory_tree);
hash_inode_map *calculateHashes(inode &directory_tree);
int countShortestDepth(std::vector<inode const *> const &);
std::vector<inode const *>
fastForwardINodeReferences(std::vector<inode const *> const);
duplicate_path_segments buildDuplicatePathSegments(
    std::vector<inode const *> const &duplicate_inode_references);
duplicate_path_seg_set filterNonDupsAndNestedHashes(inode const &,
                                                    hash_inode_map *);
duplicate_path_seg_set transform(file_hash_rows const &);
