

#include "./file_system.h"

#include <openssl/evp.h>
#include <stdlib.h>

#include <filesystem>
#include <fstream>

#include "../lib.h"

std::string qualifyRelativeURL(std::string &relative_path) {
  std::string canonicalized_path = realpath(relative_path.c_str(), 0);
  return std::string(std::filesystem::absolute(canonicalized_path).c_str());
}

int testing() { return 5; }

std::ifstream openFile(std::string filePath) {
  std::ifstream file(filePath, std::ios::binary);
  if (!file) {
    throw file_open_error("Could not open the file: " + filePath);
  }
  return file;
}

void visitFiles(const std::string &directory_path,
                file_visitor_callback callback, void *context) {
  for (const auto &entry :
       std::filesystem::recursive_directory_iterator(directory_path)) {
    if (std::filesystem::is_regular_file(entry.path())) {
      callback(entry.path(), FILE_TYPE_FILE, context);
    }
    if (std::filesystem::is_directory(entry.path())) {
      callback(entry.path(), FILE_TYPE_DIRECTORY, context);
    }
  }
}

void extractHash(hash hash, std::string path) {
  std::ifstream file = openFile(path);
  EVP_MD_CTX *md_context;
  unsigned char *md5_digest;
  unsigned int md5_digest_length = MD5_DIGEST_LENGTH;
  const int BUFFER_SIZE =
      4194304; // This should give us an stack overflow error.
  char buffer[BUFFER_SIZE];
  int blocks_read = 0;

  md_context = EVP_MD_CTX_new();
  EVP_DigestInit_ex(md_context, EVP_md5(), nullptr);

  while (file.read(buffer, BUFFER_SIZE) || file.gcount()) {
    ++blocks_read;
    EVP_DigestUpdate(md_context, buffer, file.gcount());
  }

  if (!blocks_read) {
    for (int i = 0; i < md5_digest_length; ++i) {
      hash[i] = 0;
    }
    return;
  }

  md5_digest = (unsigned char *)OPENSSL_malloc(md5_digest_length);
  EVP_DigestFinal_ex(md_context, md5_digest, &md5_digest_length);
  EVP_MD_CTX_free(md_context);

  for (int i = 0; i < md5_digest_length; ++i) {
    hash[i] = md5_digest[i];
  }

  OPENSSL_free(md5_digest);
}
