#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>

// These resources were helpful in figuring this out! I had no idea what I was
// doing.
// https://www.linuxquestions.org/questions/programming-9/how-to-get-the-execl-output-452026/
// https://www.youtube.com/watch?v=rW_NV6rf0rM

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

int main() {
  int pfd[2];
  // Create a pipe. One read end and one write end.
  if (pipe(pfd) == -1) {
    perror("pipe");
    return 1;
  }

  // Fork this process into a child process.
  pid_t pid = fork();

  if (pid == -1) {
    // Print any errors with the fork.
    perror("fork");
    return 1;
  } else if (pid == 0) {
    // In the child process
    close(pfd[0]);                // Close the read end.
    dup2(pfd[1], STDOUT_FILENO);  // Bind STDOUT to the write file.
    close(pfd[1]);                // Close the write end.
    execl("/bin/ls", "-al", "/var", (char *)0);  // Run LS.
    perror("exec");                              // Print any exec errors.
    exit(-1);                                    // Exit with exit all ones.
  }

  close(pfd[1]);  // Close the write pipe.
  std::string output =
      read_from_fd(pfd[0]);  // Read from the read end until EOF.
  close(pfd[0]);             // Close the read end.
  waitpid(pid, nullptr, 0);  // Wait until the process changes status.

  std::cout << output;

  return 0;
}

/**
 * -- After pipe and fork ---
 *
 * Parent
 * 0 (STDIN) -> (File 1)
 * 1 (STDOUT) -> (File 2)
 * 2 (STDERR) -> (File 3)
 * 3 -> (File 4) // Pipe read
 * 4 -> (File 5) // Pipe write
 *
 * Fork
 * 0 (STDIN) -> (File 1)
 * 1 (STDOUT) -> (File 2)
 * 2 (STDERR) -> (File 3)
 * 3 -> (File 4) // Pipe read
 * 4 -> (File 5) // Pipe write
 *
 * -- After close(pfd[0]) ---
 *
 * Parent
 * 0 (STDIN) -> (File 1)
 * 1 (STDOUT) -> (File 2)
 * 2 (STDERR) -> (File 3)
 * 3 -> (File 4) // Pipe read
 * 4 -> (File 5) // Pipe write
 *
 * Fork
 * 0 (STDIN) -> (File 1)
 * 1 (STDOUT) -> (File 2)
 * 2 (STDERR) -> (File 3)
 * 3 x (File 4) // Pipe read
 * 4 -> (File 5) // Pipe write
 *
 *
 * -- After dup2(pfd[1], STDOUT_FILENO) ---
 *
 * Parent
 * 0 (STDIN) -> (File 1)
 * 1 (STDOUT) -> (File 2)
 * 2 (STDERR) -> (File 3)
 * 3 -> (File 4) // Pipe read
 * 4 -> (File 5) // Pipe write
 *
 * Fork
 * 0 (STDIN) -> (File 1)
 * 1 (STDOUT) -> (File 5) <-- Set STDOUT to go to file 5.
 * 2 (STDERR) -> (File 3)
 * 3 x (File 4) // Pipe read
 * 4 -> (File 5) // Pipe write
 *
 *
 * -- After close(pfd[1]) ---
 *
 * Parent
 * 0 (STDIN) -> (File 1)
 * 1 (STDOUT) -> (File 2)
 * 2 (STDERR) -> (File 3)
 * 3 -> (File 4) // Pipe read
 * 4 -> (File 5) // Pipe write
 *
 * Fork
 * 0 (STDIN) -> (File 1)
 * 1 (STDOUT) -> (File 5) <-- Set STDOUT to go to file 5.
 * 2 (STDERR) -> (File 3)
 * 3 x (File 4) // Pipe read
 * 4 x (File 5) // Pipe write
 *
 * -- Execl writes executes ls which will write to STDOUT. (does execl write to
 * File 2 or File 5???)
 *
 *
 * -- After close(pfd[1]) in parent ---
 *
 * Parent
 * 0 (STDIN) -> (File 1)
 * 1 (STDOUT) -> (File 2)
 * 2 (STDERR) -> (File 3)
 * 3 -> (File 4) // Pipe read
 * 4 x (File 5) // Pipe write (File 5 killed because it has no refs)
 *
 * Fork (Killed)
 * 0 (STDIN) -> (File 1)
 * 1 (STDOUT) -> (File 5) <-- Set STDOUT to go to file 5.
 * 2 (STDERR) -> (File 3)
 * 3 x (File 4) // Pipe read
 * 4 x (File 5) // Pipe write
 *
 *
 * (If I read from File 5 I should get the input.)
 *
 */
