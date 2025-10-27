/**
 * This program and the accompanying materials are made available under the terms of the
 * Eclipse Public License v2.0 which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v20.html
 *
 * SPDX-License-Identifier: EPL-2.0
 *
 * Copyright Contributors to the Zowe Project.
 *
 */

#include <stdexcept>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include "ztest.hpp"
#include "ztype.h"
#include "zowed.test.hpp"

using namespace std;
using namespace ztst;

struct DaemonHandle
{
  pid_t pid;
  FILE *output_stream;
  int pipe_fd;
};

/**
 * @brief Start a daemon process
 * @param command Command to execute
 * @return DaemonHandle containing process information
 */
DaemonHandle start_daemon(const std::string &command)
{
  int pipefd[2];
  if (pipe(pipefd) == -1)
  {
    throw std::runtime_error("Failed to create pipe");
  }

  pid_t pid = fork();
  if (pid == -1)
  {
    close(pipefd[0]);
    close(pipefd[1]);
    throw std::runtime_error("Failed to fork process");
  }

  if (pid == 0)
  {
    // Child process: redirect stdout/stderr to pipe and execute command
    close(pipefd[0]);
    dup2(pipefd[1], STDOUT_FILENO);
    dup2(pipefd[1], STDERR_FILENO);
    close(pipefd[1]);

    // Use shell from environment, fallback to /bin/sh
    const char *shell = getenv("SHELL");
    if (shell == nullptr || shell[0] == '\0')
    {
      shell = "/bin/sh";
    }

    execl(shell, shell, "-c", command.c_str(), (char *)nullptr);
    _exit(127); // execl failed
  }

  // Parent process: setup pipe for reading
  close(pipefd[1]);

  FILE *pipe = fdopen(pipefd[0], "r");
  if (!pipe)
  {
    close(pipefd[0]);
    kill(pid, SIGKILL);
    waitpid(pid, nullptr, 0);
    throw std::runtime_error("Failed to open pipe stream");
  }

  DaemonHandle handle;
  handle.pid = pid;
  handle.output_stream = pipe;
  handle.pipe_fd = pipefd[0];

  return handle;
}

/**
 * @brief Read output from a running daemon
 * @param handle DaemonHandle from start_daemon
 * @param output String to append output to
 * @param read_all If true, read all available output; if false, read one line
 */
void read_daemon_output(DaemonHandle &handle, std::string &output, bool read_all = false)
{
  char buffer[256];

  if (read_all)
  {
    // Read all available output
    while (fgets(buffer, sizeof(buffer), handle.output_stream) != nullptr)
    {
      output += buffer;
    }
  }
  else
  {
    // Read just one line
    if (fgets(buffer, sizeof(buffer), handle.output_stream) != nullptr)
    {
      output += buffer;
    }
  }
}

/**
 * @brief Stop a running daemon by sending SIGINT
 * @param handle DaemonHandle from start_daemon
 */
void stop_daemon(DaemonHandle &handle)
{
  // Send SIGINT (Ctrl+C) to the daemon
  kill(handle.pid, SIGINT);

  // Give the process a moment to handle the signal
  usleep(100000); // 100ms

  // Read any remaining output
  char buffer[256];
  while (fgets(buffer, sizeof(buffer), handle.output_stream) != nullptr)
  {
    // Discard remaining output
  }

  // Close the pipe
  fclose(handle.output_stream);

  // Wait for process to terminate (don't check exit code)
  int status;
  waitpid(handle.pid, &status, 0);
}

const string zowed_dir = "./../../zowed/build-out";
const string zowed_command = zowed_dir + "/zowed";

void zowed_tests()
{

  describe("zowed tests",
           []() -> void
           {
             it("should print a ready message on startup",
                []() -> void
                {
                  // Start the daemon, read first line of output, then stop it
                  string response;
                  DaemonHandle daemon = start_daemon(zowed_command);
                  read_daemon_output(daemon, response);
                  stop_daemon(daemon);

                  Expect(response).ToContain("\"checksums\":null");
                  Expect(response).ToContain("\"message\":\"zowed is ready to accept input\"");
                  Expect(response).ToContain("\"status\":\"ready\"");
                });
            it("should print a ready message with checksums",
                []() -> void
                {
                  // Create test checksums file
                  string checksums_file = zowed_dir + "/checksums.asc";
                  unlink(checksums_file.c_str());
                  ofstream outfile(checksums_file);
                  outfile << "123 abc" << endl;
                  outfile.close();

                  // Start the daemon, read first line of output, then stop it
                  string response;
                  DaemonHandle daemon = start_daemon(zowed_command);
                  read_daemon_output(daemon, response);
                  stop_daemon(daemon);

                  Expect(response).ToContain("\"checksums\":{\"abc\":\"123\"}");
                  Expect(response).ToContain("\"message\":\"zowed is ready to accept input\"");
                  Expect(response).ToContain("\"status\":\"ready\"");

                  // Cleanup
                  unlink(checksums_file.c_str());
                });
             it("should remain less than 10mb in size",
                []() -> void
                {
                  struct stat st;
                  if (stat(zowed_command.c_str(), &st) != 0)
                  {
                    throw std::runtime_error("Failed to stat file: " + zowed_command);
                  }
                  
                  off_t file_size = st.st_size;
                  Expect(file_size).ToBeLessThan(10 * 1024 * 1024);
                }); });
}
