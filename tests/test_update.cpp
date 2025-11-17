
// #include "../"
#include <cassert>
#include <sstream>
#include <unordered_map>

#include "../src/lib.cpp"
#include "../src/sqlite/operators.cpp"
#include "../src/update/update.cpp"
#include "./data.cpp"

/* ------------------------------ Output Mocks ------------------------------ */
std::ostringstream OUTPUT_MOCK{};
std::ostringstream OUTPUT_ERROR_MOCK{};

/* ---------------------------------- Mocks --------------------------------- */
const std::unordered_map<std::string, bool> file_exists_return{
    {"/user/test/home/dir1/testing.txt", true},
    {"/user/test/home/dir1/oranges/testing_two.txt", false},
    {"/user/test/home/dir2/oranges/testing_three.txt", false},
    {"/user/test/home/dir1/apples/testing_four.txt", true}};
const directory_table_row::rows fetch_all_directories_return{
    {1, "dir1", -1},   {2, "dir2", -1},  {3, "oranges", 1},
    {4, "oranges", 2}, {5, "apples", 1},
};
const hash_table_row::rows fetch_all_hashes_return{
    {1, 1, "testing.txt", uniqueTestHash()},
    {2, 3, "testing_two.txt", uniqueTestHash()},
    {3, 4, "testing_three.txt", uniqueTestHash()},
    {4, 5, "testing_four.txt", uniqueTestHash()}};

const scan_meta_data_table_row fetch_scan_meta_data_return{
    .root_dir = "/user/test/home"};

std::vector<int> last_delete_hash_id{};

bool fileExists(std::string const &file_path) {
  return file_exists_return.at(file_path);
}

std::string joinPath(std::vector<std::string> const &path_segments) {
  std::string joined_path{};
  for (int i = 0; i < path_segments.size() - 1; ++i) {
    joined_path += path_segments[i];
    joined_path += '/';
  }
  joined_path += path_segments[path_segments.size() - 1];
  return joined_path;
}

sqlite3 *initDB(char const *const file_name) { return nullptr; };
void freeDB(sqlite3 *db) { return; }

directory_table_row::rows fetchAllDirectories(sqlite3 *db) {
  return fetch_all_directories_return;
}

hash_table_row::rows fetchAllHashes(sqlite3 *db) {
  return fetch_all_hashes_return;
}

scan_meta_data_table_row fetchScanMetaData(sqlite3 *db) {
  return fetch_scan_meta_data_return;
}

void deleteHash(sqlite3 *db, int id) { last_delete_hash_id.push_back(id); }

void resetMocks() { last_delete_hash_id = {}; }

/* ---------------------------------- Tests --------------------------------- */
void testUpdateDeletesMissingFiles() {
  // Arrange
  resetMocks();

  // Act
  update("testing", OUTPUT_MOCK);

  // Assert
  std::vector<int> expected_deleted_hash_ids = {2, 3};
  assert(last_delete_hash_id == expected_deleted_hash_ids);
}

void testPrintingTheNumberOfFilesWeDelete() {
  // Arrange
  resetMocks();

  // Act
  update("testing", OUTPUT_MOCK);

  // Assert
  // Regex and search for the delete file numbers.
}

int main() { testUpdateDeletesMissingFiles(); }
