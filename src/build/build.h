
#include "../fs/file_system.h"
#include "../sqlite/sqlite.h"
#include <ostream>

void build(std::vector<std::string> paths, std::string cache_path,
           std::ostream &console);