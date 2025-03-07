#pragma once

#include <stdexcept>
#include <string>

enum FileType { FILE_TYPE_FILE, FILE_TYPE_DIRECTORY };

typedef void (*FileVisitorCallback)(const std::string, const enum FileType,
                                    void*);

/* -------------------------------------------------------------------------- */
/*                                  Functions                                 */
/* -------------------------------------------------------------------------- */
int testing();
std::string qualifyRelativeURL(std::string& relative_path);
void visitFiles(const std::string& directory_path,
                FileVisitorCallback visitor_callback, void* context);
void extractHash(uint8_t* hash, std::string path);
bool fileExists(std::string path);

/* -------------------------------------------------------------------------- */
/*                                 Exceptions                                 */
/* -------------------------------------------------------------------------- */
class FileOpenError : public std::runtime_error {
 public:
  FileOpenError(const std::string& message) : std::runtime_error(message) {}
};