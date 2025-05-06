#pragma once

#include <stdexcept>
#include <string>
#include <vector>

enum file_type { FILE_TYPE_FILE, FILE_TYPE_DIRECTORY };

typedef void (*file_visitor_callback)(const std::string, const enum file_type,
                                      void *);

/* -------------------------------------------------------------------------- */
/*                                  Functions                                 */
/* -------------------------------------------------------------------------- */
std::string qualifyRelativeURL(std::string &relative_path);
std::string joinPath(std::vector<std::string> const &path_segments);
void visitFiles(const std::string &directory_path,
                file_visitor_callback visitor_callback, void *context);
void extractHash(uint8_t *hash, std::string path);
bool fileExists(std::string const &file_path);
void createDirectory(std::string const &path);

/* --------------------------------------------------------------------------
 */
/*                                 Exceptions */
/* --------------------------------------------------------------------------
 */
class file_open_error : public std::runtime_error {
public:
  file_open_error(const std::string &message) : std::runtime_error(message) {}
};

class create_directory_error : public std::runtime_error {
public:
  create_directory_error(const std::string &message)
      : std::runtime_error(message) {}
};