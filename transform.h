#include <list>
#include <ostream>
#include <unordered_map>

#include "./extract_output.h"
#include "./transform_output.h"

struct HashPathSegment {
  std::string path_segment;
  std::string hash_string;

  bool operator==(const HashPathSegment &rhs) const;
};

typedef std::vector<Hash> Hashes;
typedef std::vector<HashPathSegment> HashPath;
typedef std::vector<HashPath> DuplicatePaths;
typedef std::unordered_map<unsigned int, const DirectoryTableRow *>
    DirectoryRowIdMap;
typedef std::unordered_map<std::string, DuplicatePaths> DuplicatePathsMap;

struct HashToDuplicateNodes {
  DuplicatePathsMap map;
  std::list<std::string> order;
};

class HashNode {
 public:
  typedef std::vector<HashNode> HashedNodes;

  HashNode(std::string name, Hash hash, HashedNodes nodes = {})
      : name(name), hash(hash), hashed_nodes(nodes) {}
  HashNode() : HashNode("", {}) {}

  std::string getName() const;
  Hash getHash() const;
  HashedNodes getHashedNodes() const;
  bool operator==(const HashNode &rhs) const;

 private:
  std::string name;
  Hash hash;
  HashedNodes hashed_nodes;
};

class FileNode : public HashNode {
 public:
  typedef std::vector<FileNode> Files;

  FileNode(std::string name, Hash hash) : HashNode(name, hash, {}) {}
};

class DirectoryNode {
 public:
  typedef std::vector<DirectoryNode> Directories;

  DirectoryNode(std::string name, FileNode::Files files = {},
                Directories directories = {})
      : name(name), files(files), directories(directories) {}
  DirectoryNode() : DirectoryNode("") {}

  std::string getName() const;
  FileNode::Files getFiles() const;
  Directories getDirectories() const;
  bool operator==(const DirectoryNode &rhs) const;

 private:
  std::string name;
  FileNode::Files files;
  Directories directories;
};

// Utility
template <class T>
int countShortestVector(const std::vector<T> &vectors);

// Building Directory Map
DirectoryRowIdMap buildDirectoryRowIdMap(
    const DirectoryTableRow::Rows &directory_table_rows);
DirectoryNode buildFileNodeBranch(const HashTableRow &hash_result,
                                  const DirectoryRowIdMap &directory_results);
DirectoryNode buildDirectoryTree(const HashTableRow::Rows &hash_results,
                                 const DirectoryRowIdMap &directory_results);
DirectoryNode mergeTwoDirectoryNodes(const DirectoryNode &tree_one,
                                     const DirectoryNode &tree_two);
FileNode::Files mergeTwoFileLists(const FileNode::Files &files_one,
                                  const FileNode::Files &files_two);
DirectoryNode::Directories mergeTwoDirectoryLists(
    const DirectoryNode::Directories &directories_one,
    const DirectoryNode::Directories &directories_two);

// Computing Hash Tree
Hash computeHashNodesHash(const HashNode::HashedNodes nodes);
Hash computeHash(Hashes file_hashes);
HashNode::HashedNodes buildHashNodes(const FileNode::Files &file_nodes);
HashNode buildHashNode(const DirectoryNode &directory_node);

// Finding hashes
HashToDuplicateNodes buildHashToDuplicateNodesMap(const HashNode &hash_node);
std::string buildStringFromHash(const Hash &hash);
void filterNonDuplicatesFromDupNodesMap(
    HashToDuplicateNodes &hash_to_duplicate_nodes);
bool hasSharedParent(const DuplicatePaths &paths,
                     const DuplicatePathsMap &hash_to_duplicate_nodes);
void filterSharedHashNodes(HashToDuplicateNodes &hash_to_duplicate_nodes);

// Output
SVector buildPathsFromHashPath(const HashPath &path);
DuplicateINodesSet buildDuplicateINodeSet(
    const DuplicatePathsMap &duplicate_nodes);
bool comparePath(const SVector &path_one, const SVector &path_two);
bool comparePaths(const Paths &paths_one, const Paths &paths_two);
void sortPaths(Paths &paths);
void sortDuplicateINodesSet(DuplicateINodesSet &duplicate_i_nodes_set);
DuplicateINodesSet transform(const DirectoryTableRow::Rows &directory_results,
                             const HashTableRow::Rows &hash_results);