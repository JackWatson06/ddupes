#include "../fs/file_system.h"
#include "extract_output.h"

/* -------------------------------------------------------------------------- */
/*                              Process Functions                             */
/* -------------------------------------------------------------------------- */
void buildCache(std::vector<std::string> &paths, sqlite3 *db);
file_hash_rows extractUsingCache(sqlite3 *db);