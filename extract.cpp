#include "extract.h"

#include <sys/stat.h>
#include <sys/wait.h>

#include <cerrno>
#include <regex>

constexpr SemVer FDUPES_VERSION{2, 0, 0};

/* -------------------------------------------------------------------------- */
/*                            File Executor Service                           */
/* -------------------------------------------------------------------------- */
bool SemVer::operator==(const SemVer& rhs) {
  return major == rhs.major && minor == rhs.minor && hotfix == rhs.hotfix;
}

bool SemVer::operator>=(const SemVer& rhs) {
  return major >= rhs.major && minor >= rhs.minor && hotfix >= rhs.hotfix;
}

std::string read_from_fd(int fd) {
  std::string buffer;
  char temp[4096];  // Temporary buffer for reading
  ssize_t bytesRead;

  // Once read comes across a EOF character it will stop reading.
  while ((bytesRead = read(fd, temp, sizeof(temp))) > 0) {
    buffer.append(temp, bytesRead);
  }

  if (bytesRead < 0) {
    throw std::runtime_error("Error reading from file descriptor");
  }

  return buffer;
}

/**
 * Return -1 if the executed process was deemed to have returned abnormally.
 * Aparently when you use exec you have to technically pass the command name
 * twice. Such as "which which ls". This is because the first specifies the
 * actual file that we want to execute. The second gets forwarded as argv[0].
 * This happens automatically on the command line.
 *
 * See: https://linux.die.net/man/2/waitpid
 */
ExecuteFileResult ForkExecuteFileCommand::execute(
    const std::string path, const std::vector<std::string> args) {
  std::vector<char*> arg_c_strings;
  for (const std::string& arg : args) {
    arg_c_strings.push_back(strdup(arg.c_str()));
  }
  arg_c_strings.push_back(nullptr);

  int command_fds[2];

  // Create two pipes. Each pipe has one read end and one write end.
  if (pipe(command_fds) == -1) {
    perror("pipe");

    throw std::system_error(errno, std::system_category(), "Pipe error!");
  }

  // Fork this process into a child process.
  pid_t pid = fork();

  if (pid == -1) {
    // Print any errors with the fork.
    perror("fork");
    throw std::system_error(errno, std::system_category(), "Fork error!");
  } else if (pid == 0) {
    // In the child process

    // Close the read ends of the pipes and copy the write file to the STDOUT
    // file descriptor.
    close(command_fds[0]);
    dup2(command_fds[1], STDOUT_FILENO);
    dup2(command_fds[1], STDERR_FILENO);
    close(command_fds[1]);  // Close the write end.

    // Execute our command after converting to proper commmand input.
    execv(path.c_str(), arg_c_strings.data());
    perror("exec");
    exit(1);
  }

  // Close the write pipe and read the command output from the read end of the
  // pipe.
  close(command_fds[1]);
  std::string output = read_from_fd(command_fds[0]);
  close(command_fds[0]);

  int code = 0;
  waitpid(pid, &code, 0);  // Wait until the process changes status.

  if (WIFEXITED(code)) {
    return {.return_code = WEXITSTATUS(code), .output = output};
  }

  return {.return_code = -1, .output = output};
}

SemVer extractVersionFromString(std::string& version_output) {
  std::regex version_regex("(\\d+)\\.(\\d+)\\.(\\d+)");
  std::smatch version_matches;

  if (std::regex_search(version_output, version_matches, version_regex)) {
    return {std::stoi(version_matches[1]), std::stoi(version_matches[2]),
            std::stoi(version_matches[3])};
  }

  throw MissingVersionException("Could not find the version number.");
}

/* -------------------------------------------------------------------------- */
/*                           File Existence Service                           */
/* -------------------------------------------------------------------------- */
bool FileExistenceCheck::check(const std::string& file_name) {
  struct stat buffer;
  return (stat(file_name.c_str(), &buffer) == 0);
}

/* -------------------------------------------------------------------------- */
/*                               SQLite Service                               */
/* -------------------------------------------------------------------------- */
SQLiteDatabase::SQLiteDatabase(std::string file) {
  int rc = sqlite3_open(file.c_str(), &db);

  if (rc != SQLITE_OK) {
    throw UnableToConnectError("Unable to conenct to the database.");
  }
}

template <class T>
void SQLiteTableView<T>::prepare(std::string query) {
  int code = sqlite3_prepare_v2(db.getDb(), query.c_str(), -1, &res, 0);

  if (code != SQLITE_OK) {
    throw UnableToBuildStatementError("Error trying to build the statement: " +
                                      query);
  }
}

template <class T>
bool SQLiteTableView<T>::step() {
  int rc = sqlite3_step(res);

  if (rc == SQLITE_ERROR || rc == SQLITE_MISUSE) {
    throw UnableToStepError(
        "You may have forgotten to prepare your statement before stepping!");
  }

  if (sqlite3_step(res) == SQLITE_DONE) {
    return false;
  }

  SQLiteTableView<T>::last_row_fetched = buildTableRow();

  return true;
}

#include <iostream>
DirectoryTableRow DirectoryTableView::buildTableRow() {
  // Parent Id
  int parent_id = sqlite3_column_int(res, 3);
  parent_id = parent_id == 0 ? -1 : parent_id;

  return DirectoryTableRow{
      .id = sqlite3_column_int(res, 0),
      .name = std::string((const char*)sqlite3_column_text(res, 1)),
      .parent_id = parent_id};
}

HashTableRow HashTableView::buildTableRow() {
  // Hash
  const uint8_t* blob = (const uint8_t*)sqlite3_column_blob(res, 10);
  uint8_t* blob_buffer = new uint8_t[MD5_DIGEST_LENGTH];
  std::memcpy(blob_buffer, blob, MD5_DIGEST_LENGTH);

  return {sqlite3_column_int(res, 0),
          std::string((const char*)sqlite3_column_text(res, 1)), blob_buffer};
}

/* -------------------------------------------------------------------------- */
/*                              Process Functions                             */
/* -------------------------------------------------------------------------- */
std::string strip(const std::string& input) {
  auto start_it = input.begin();
  auto end_it = input.rbegin();
  while (std::isspace(*start_it)) {
    ++start_it;
  }

  while (std::isspace(*end_it)) {
    ++end_it;
  }

  return std::string(start_it, end_it.base());
}

std::string findFDupesCommand(ForkExecuteFileCommand& executor) {
  ExecuteFileResult which_result =
      executor.execute("/bin/which", {"which", "fdupes"});

  if (which_result.return_code != 0) {
    throw NoFDupesException(
        "Could not find the fdupes command on your system!");
  }

  return strip(which_result.output);
}

void checkFDupesVersion(std::string fdupes_file,
                        ForkExecuteFileCommand& executor) {
  ExecuteFileResult fdupes_version_result =
      executor.execute(fdupes_file, {"fdupes", "--version"});

  if (fdupes_version_result.return_code != 0) {
    throw FDupesVersionException("Could not run the fdupes version command.");
  }

  if ((extractVersionFromString(fdupes_version_result.output) >=
       FDUPES_VERSION) == false) {
    throw InvalidVersionException("FDupes does not have the correct version!");
  }
}

void executeFDupesCacheBuild(std::string fdupes_file,
                             RelativePaths paths_to_check,
                             ForkExecuteFileCommand& executor) {
  std::vector<std::string> args{"fdupes", "-r", "--cache", "-x", "cache.clear"};

  for (const std::string path_to_check : paths_to_check) {
    args.push_back(path_to_check);
  }

  ExecuteFileResult fdupes_cache_build_result =
      executor.execute(fdupes_file, args);

  if (fdupes_cache_build_result.return_code != 0) {
    throw FailedFDupesCacheBuildException("FDupes command failed! Output: " +
                                          fdupes_cache_build_result.output);
  }
}

void verifyCacheExists(const std::string& file_name,
                       FileExistenceCheck& checker) {
  bool file_exists = checker.check(file_name);

  if (!file_exists) {
    throw CacheNotFoundException("The file cache passed in was not found!");
  }
}

FileHashRows loadDataFromCache(
    TableView<DirectoryTableRow>* const directory_table_view,
    TableView<HashTableRow>* const hash_table_view) {
  DirectoryTableRow::Rows directory_table_rows{};
  HashTableRow::Rows hash_table_rows{};

  directory_table_view->prepare("SELECT * FROM Directories;");
  hash_table_view->prepare("SELECT * FROM Hashes;");

  while (directory_table_view->step()) {
    directory_table_rows.push_back(directory_table_view->getLastRowFetched());
  }

  while (hash_table_view->step()) {
    hash_table_rows.push_back(hash_table_view->getLastRowFetched());
  }

  return {directory_table_rows, hash_table_rows};
};

FileHashRows extract(const RelativePaths& paths,
                     const std::string cache_directory) {
  // Test and run the FDupes command.
  ForkExecuteFileCommand executor;
  std::string file_dupes_file = findFDupesCommand(executor);
  checkFDupesVersion(file_dupes_file, executor);
  executeFDupesCacheBuild(file_dupes_file, paths, executor);

  // Check if the cache file exists.
  FileExistenceCheck checker;
  verifyCacheExists(cache_directory, checker);

  // Load the data from the SQL database.
  SQLiteDatabase sqlite{cache_directory};
  DirectoryTableView directory_table_view{sqlite};
  HashTableView hash_table_view{sqlite};

  return loadDataFromCache(
      (TableView<DirectoryTableRow>* const)&directory_table_view,
      (TableView<HashTableRow>* const)&hash_table_view);
}
