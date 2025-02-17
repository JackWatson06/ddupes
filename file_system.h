
#include <string>

template <class T>
void visitFiles(const std::string& directory_path,
                void (*fileNodeCallback)(const std::string, const T),
                const T& services) {
  for (const auto& entry : fs::recursive_directory_iterator(directory_path)) {
    fileNodeCallback(entry.path(), services)
  }
}

bool isFile(std::string path);
bool isDirectory(std::string path);
void extractHash(uint8_t* hash, std::string path);
