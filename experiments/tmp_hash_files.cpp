
#include <openssl/evp.h>

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

namespace fs = std::filesystem;

constexpr unsigned int MD5_HASH_LENGTH = 16;
// CHATGPT code
// ---------------------
std::ifstream openFile(std::string filePath) {
  std::ifstream file(filePath, std::ios::binary);
  if (!file) {
    throw std::runtime_error("Could not open the file: " + filePath);
  }
  return file;
}

void computeFileHash(uint8_t *hash_buffer, std::ifstream &file) {
  EVP_MD_CTX *md_context;
  unsigned char *md5_digest;
  unsigned int md5_digest_length = MD5_HASH_LENGTH;
  const int BUFFER_SIZE = 4096;
  char buffer[BUFFER_SIZE];

  md_context = EVP_MD_CTX_new();
  EVP_DigestInit_ex(md_context, EVP_md5(), nullptr);

  while (file.read(buffer, BUFFER_SIZE) || file.gcount()) {
    EVP_DigestUpdate(md_context, buffer, file.gcount());
  }

  md5_digest = (unsigned char *)OPENSSL_malloc(md5_digest_length);
  EVP_DigestFinal_ex(md_context, md5_digest, &md5_digest_length);
  EVP_MD_CTX_free(md_context);

  for (int i = 0; i < md5_digest_length; ++i) {
    hash_buffer[i] = md5_digest[i];
  }

  OPENSSL_free(md5_digest);
}

void hashFilesInDirectory(const std::string &directoryPath) {
  for (const auto &entry : fs::recursive_directory_iterator(directoryPath)) {
    entry.~directory_entry if (fs::is_regular_file(entry.path())) {
      std::string file_path = entry.path().string();
      std::ifstream file = openFile(file_path);
      uint8_t hash_buffer[MD5_HASH_LENGTH];
      computeFileHash(hash_buffer, file);
      std::cout << "Computed Hash for file, " << file_path << ": ";
      for (int i = 0; i < MD5_HASH_LENGTH; ++i) {
        std::cout << hash_buffer[i];
      }
      std::cout << '\n';
    }
    else {
      std::cout << entry.path() << '\n';
    }
  }
}
// ---------------------

/**
 * The Cache ETL process contains the following steps:
 * - Load the files from the the file system and the type! (Extract)
 * - If Statement for type (Extract)
 *    - If file then extract directory id, and file hash (Transform)
 *      - Create file hash struct (Transform)
 *      - Put into the has database (Load)
 *    - If directory then extract directory id (Transform)
 *      - Create the directory hash struct. (Transform)
 *      - Put into the database (Load)
 *
 * - Here we have a classic case of E(Loop), T, L where you do the transormation
 * and loading in each entry then fetch the next one.
 * - I have two processes here. First we create the cache, then we load from the
 * cache and compute directory hashes.
 * - Another option could be doing a post-order DFS as part of the EXTRACT
 * phase, then my transformation and load phases also compute the directory
 * hash. This removes the need to recreate this tree in memory.
 * - I could improve this by keeping a reference to the directory insert on a
 * stack.
 */

void visitFiles(const std::string &directory_path,
                void (*fileNodeCallback)(const std::string, const T),
                const(*void) services) {
  for (const auto &entry : fs::recursive_directory_iterator(directory_path)) {
    file_type file_type = // IF directory then load directory.
        fileNodeCallback(entry.path(), file_type, services)
  }
}

struct CacheServices {
  bool (*fileCheck)(std::string);
  bool (*directoryCheck)(std::string);
  void (*extractHash)(uint8_t *hash, std::string);
  DirectoryTableGateway
      &directory_table_gateway; // These can be pointers (although it becomes a
                                // pain in the butt to carry around the
                                // reference to the db if these are just
                                // callbacks.)
  FileTableGateway &file_table_gateway; // These can be pointers.
}

enum file_type {
  DIRECTORY = 1,
  FILE = 2,
}

// This is my code. It interacts with no services except those passed in.
void fileNodeCallback(
    std::string path, file_type file_type, (void*) services) {
  const CacheServices cache_services = (CacheServices *)services;
  const unsigned int directory_id =
      services.directory_table_gateway.getByName(path);
  const std::string name = "testing";
  // Fetch the name.
  // Fetch the parent just split and pull out the first and next to first. This
  // does not need to be injected because we won't use libraries.

  if (directory_id == 0) {
    throw std::runtime_error("Can't find the directory.");
  }

  if (services.fileCheck(path)) {
    uint8_t hash[16];
    services.extractHash(hash, path);
    const FileHashInput file_hash_input{
        .directory_id = directory_id, .name = name, .hash = hash};
    services.file_table_gateway.create(file_hash_input);
  }

  if (services.directoryCheck(path)) {
    const DirectoryHashInput directory_hash_input{.parent_id = directory_id,
                                                  .name = name};
    services.directory_table_gateway.create();
  }
}

int main() {
  std::string directory = "tests/testing_dirs/";
  visitFiles(directory);

  return 0;
}
