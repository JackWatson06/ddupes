#pragma once

#include <stdexcept>
#include <string>

enum file_type { FILE_TYPE_FILE, FILE_TYPE_DIRECTORY };

typedef void (*file_visitor_callback)(const std::string, const enum file_type,
                                      void *);

/* -------------------------------------------------------------------------- */
/*                                  Functions                                 */
/* -------------------------------------------------------------------------- */
int testing();
std::string qualifyRelativeURL(std::string &relative_path);
void visitFiles(const std::string &directory_path,
                file_visitor_callback visitor_callback, void *context);
void extractHash(uint8_t *hash, std::string path);
bool fileExists(std::string path);

/* -------------------------------------------------------------------------- */
/*                                 Exceptions                                 */
/* -------------------------------------------------------------------------- */
class file_open_error : public std::runtime_error {
public:
  file_open_error(const std::string &message) : std::runtime_error(message) {}
};