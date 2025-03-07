#include <list>
#include <ostream>
#include <unordered_map>

#include "./extract_output.h"
#include "./transform_output.h"

/* -------------------------------------------------------------------------- */
/*                          Data Structures and Types                         */
/* -------------------------------------------------------------------------- */
typedef std::vector<uint8_t> Hash;
typedef std::vector<Hash> Hashes;

struct HashPathSegment {
  std::string path_segment;
  std::string hash_string;

  bool operator==(const HashPathSegment &rhs) const;
};

typedef std::vector<HashPathSegment> HashPath;
typedef std::vector<HashPath> DuplicatePaths;
typedef std::unordered_map<std::string, DuplicatePaths> DuplicatePathsMap;

struct HashToDuplicateNodes {
  DuplicatePathsMap map;
  std::list<std::string> order;
};

typedef std::unordered_map<unsigned int, const DirectoryTableRow *>
    DirectoryRowIdMap;

struct HashNode {
  typedef std::vector<HashNode> HashedNodes;

  std::string name;
  Hash hash;
  HashedNodes hashed_nodes;

  HashNode(std::string name, Hash hash, HashedNodes nodes = {})
      : name(name), hash(hash), hashed_nodes(nodes) {}
  HashNode() : HashNode("", {}) {}
  bool operator==(const HashNode &rhs) const;
};

struct FileNode : public HashNode {
 public:
  typedef std::vector<FileNode> Files;

  FileNode(std::string name, Hash hash) : HashNode(name, hash, {}) {}
};

struct DirectoryNode {
  typedef std::vector<DirectoryNode> Directories;
  std::string name;
  FileNode::Files files;
  Directories directories;

  DirectoryNode(std::string name, FileNode::Files files = {},
                Directories directories = {})
      : name(name), files(files), directories(directories) {}
  DirectoryNode() : DirectoryNode("") {}
  bool operator==(const DirectoryNode &rhs) const;
};

/* -------------------------------------------------------------------------- */
/*                           Building Directory Map                           */
/* -------------------------------------------------------------------------- */
DirectoryRowIdMap buildDirectoryRowIdMap(
    const DirectoryTableRow::Rows &directory_table_rows);
Hash blobToHash(const void *blob);
DirectoryNode buildFileNodeBranch(const HashTableRow &hash_result,
                                  const DirectoryRowIdMap &directory_results);
FileNode::Files mergeTwoFileLists(const FileNode::Files &files_one,
                                  const FileNode::Files &files_two);
DirectoryNode::Directories mergeTwoDirectoryLists(
    const DirectoryNode::Directories &directories_one,
    const DirectoryNode::Directories &directories_two);
DirectoryNode mergeTwoDirectoryNodes(const DirectoryNode &tree_one,
                                     const DirectoryNode &tree_two);
DirectoryNode buildDirectoryTree(const HashTableRow::Rows &hash_results,
                                 const DirectoryRowIdMap &directory_results);

/* -------------------------------------------------------------------------- */
/*                             Computing Hash Tree                            */
/* -------------------------------------------------------------------------- */
Hash computeHashNodesHash(const HashNode::HashedNodes nodes);
Hash computeHash(Hashes file_hashes);
HashNode::HashedNodes buildHashNodes(const FileNode::Files &file_nodes);
HashNode buildHashNode(const DirectoryNode &directory_node);

/* -------------------------------------------------------------------------- */
/*                               Finding Hashes                               */
/* -------------------------------------------------------------------------- */
HashToDuplicateNodes buildHashToDuplicateNodesMap(const HashNode &hash_node);
std::string buildStringFromHash(const Hash &hash);
void filterNonDuplicatesFromDupNodesMap(
    HashToDuplicateNodes &hash_to_duplicate_nodes);
int countShortestDuplicatePath(const DuplicatePaths &duplicate_paths);
bool hasSharedParent(const DuplicatePaths &paths,
                     const DuplicatePathsMap &hash_to_duplicate_nodes);
void filterSharedHashNodes(HashToDuplicateNodes &hash_to_duplicate_nodes);

/* -------------------------------------------------------------------------- */
/*                                   Output                                   */
/* -------------------------------------------------------------------------- */
SVector buildPathsFromHashPath(const HashPath &path);
DuplicateINodesSet buildDuplicateINodeSet(
    const DuplicatePathsMap &duplicate_nodes);
DuplicateINodesSet transform(const FileHashRows &file_hash_rows);