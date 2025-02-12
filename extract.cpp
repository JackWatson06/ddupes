#include "extract.h"

#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <regex>
#include <stdexcept>
#include <string>
#include <system_error>

// Remove the below
#include <iostream>

constexpr SemVer FDUPES_VERSION{2, 0, 0};

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
 * See: https://linux.die.net/man/2/waitpid
 */
ExecuteFileResult ForkExecuteFileCommand::execute(
    const std::string path, const std::vector<std::string> args) {
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
    close(command_fds[1]);  // Close the write end.

    // Execute our command after converting to proper commmand input.
    std::vector<char*> arg_c_strings;
    for (const std::string& arg : args) {
      arg_c_strings.push_back(strdup(arg.c_str()));
    }

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

std::string findFDupesCommand(ForkExecuteFileCommand& executor) {
  ExecuteFileResult which_result = executor.execute("/bin/which", {"fdupes"});

  if (which_result.return_code != 0) {
    throw NoFDupesException(
        "Could not find the fdupes command on your system!");
  }

  return which_result.output;
}

void checkFDupesVersion(std::string fdupes_file,
                        ForkExecuteFileCommand& executor) {
  ExecuteFileResult fdupes_version_result =
      executor.execute(fdupes_file, {"--version"});

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
  std::vector<std::string> args{"--cache"};

  for (const std::string path_to_check : paths_to_check) {
    args.push_back(path_to_check);
  }

  ExecuteFileResult fdupes_cache_build_result =
      executor.execute(fdupes_file, args);

  if (fdupes_cache_build_result.return_code != 0) {
    throw FailedFDupesCacheBuildException("FDupes command failed!");
  }
}

void extract(RelativePaths& paths) {
  ForkExecuteFileCommand executor;

  std::string file_dupes_file = findFDupesCommand(executor);
  std::cout << file_dupes_file;
}

// Verify cache exists.

// Load cache database into sqllite.

// Confirm the correct table exists.

// Extract out of the tables and convert into extract_output.h data structures.
