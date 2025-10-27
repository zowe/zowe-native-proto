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
#include <fstream>
#include <fcntl.h>
#include "ztest.hpp"
#include "ztype.h"
#include "zowed.test.hpp"

using namespace std;
using namespace ztst;

struct DaemonHandle
{
  pid_t pid;
  FILE *output_stream; // For reading stdout/stderr from daemon
  FILE *input_stream;  // For writing stdin to daemon
  int output_pipe_fd;
  int input_pipe_fd;
};

/**
 * @brief Start a daemon process
 * @param command Command to execute
 * @return DaemonHandle containing process information
 */
DaemonHandle start_daemon(const std::string &command)
{
  int output_pipe[2]; // Parent reads from output_pipe[0], child writes to output_pipe[1]
  int input_pipe[2];  // Parent writes to input_pipe[1], child reads from input_pipe[0]

  if (pipe(output_pipe) == -1)
  {
    throw std::runtime_error("Failed to create output pipe");
  }

  if (pipe(input_pipe) == -1)
  {
    close(output_pipe[0]);
    close(output_pipe[1]);
    throw std::runtime_error("Failed to create input pipe");
  }

  pid_t pid = fork();
  if (pid == -1)
  {
    close(output_pipe[0]);
    close(output_pipe[1]);
    close(input_pipe[0]);
    close(input_pipe[1]);
    throw std::runtime_error("Failed to fork process");
  }

  if (pid == 0)
  {
    // Child process: redirect stdin from input_pipe, stdout/stderr to output_pipe
    close(output_pipe[0]); // Close read end of output pipe
    close(input_pipe[1]);  // Close write end of input pipe

    dup2(input_pipe[0], STDIN_FILENO);   // Redirect stdin
    dup2(output_pipe[1], STDOUT_FILENO); // Redirect stdout
    dup2(output_pipe[1], STDERR_FILENO); // Redirect stderr

    close(input_pipe[0]);
    close(output_pipe[1]);

    // Use shell from environment, fallback to /bin/sh
    const char *shell = getenv("SHELL");
    if (shell == nullptr || shell[0] == '\0')
    {
      shell = "/bin/sh";
    }

    execl(shell, shell, "-c", command.c_str(), (char *)nullptr);
    _exit(127); // execl failed
  }

  // Parent process: close unused ends of pipes
  close(output_pipe[1]); // Close write end of output pipe
  close(input_pipe[0]);  // Close read end of input pipe

  // Set output pipe to non-blocking mode to prevent hanging
  int flags = fcntl(output_pipe[0], F_GETFL, 0);
  fcntl(output_pipe[0], F_SETFL, flags | O_NONBLOCK);

  FILE *output_stream = fdopen(output_pipe[0], "r");
  if (!output_stream)
  {
    close(output_pipe[0]);
    close(input_pipe[1]);
    kill(pid, SIGKILL);
    waitpid(pid, nullptr, 0);
    throw std::runtime_error("Failed to open output pipe stream");
  }

  FILE *input_stream = fdopen(input_pipe[1], "w");
  if (!input_stream)
  {
    fclose(output_stream);
    close(input_pipe[1]);
    kill(pid, SIGKILL);
    waitpid(pid, nullptr, 0);
    throw std::runtime_error("Failed to open input pipe stream");
  }

  DaemonHandle handle;
  handle.pid = pid;
  handle.output_stream = output_stream;
  handle.input_stream = input_stream;
  handle.output_pipe_fd = output_pipe[0];
  handle.input_pipe_fd = input_pipe[1];

  return handle;
}

/**
 * @brief Read output from a running daemon
 * @param handle DaemonHandle from start_daemon
 * @param output String to append output to
 * @param timeout_ms Timeout in milliseconds (default 5000ms = 5 seconds)
 */
void read_daemon_output(DaemonHandle &handle, std::string &output, int timeout_ms = 5000)
{
  char buffer[256];
  int attempts = 0;
  int max_attempts = timeout_ms / 10; // Check every 10ms

  // Read just one line with timeout
  while (attempts < max_attempts)
  {
    char *result = fgets(buffer, sizeof(buffer), handle.output_stream);
    if (result != nullptr)
    {
      output += buffer;
      return; // Successfully read a line
    }

    // If errno is EAGAIN/EWOULDBLOCK, the read would block (non-blocking mode)
    if (errno == EAGAIN || errno == EWOULDBLOCK)
    {
      usleep(10000); // Sleep 10ms and try again
      attempts++;
      continue;
    }

    // Other error occurred
    break;
  }

  if (attempts >= max_attempts)
  {
    throw std::runtime_error("Timeout waiting for daemon output");
  }
}

/**
 * @brief Write input to a running daemon
 * @param handle DaemonHandle from start_daemon
 * @param input String to write to daemon's stdin
 * @param flush If true, flush the stream immediately (default true)
 */
void write_to_daemon(DaemonHandle &handle, const std::string &input, bool flush = true)
{
  if (fputs(input.c_str(), handle.input_stream) == EOF)
  {
    throw std::runtime_error("Failed to write to daemon stdin");
  }

  if (flush)
  {
    if (fflush(handle.input_stream) != 0)
    {
      throw std::runtime_error("Failed to flush daemon stdin");
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

  // Close the pipes
  if (handle.input_stream)
  {
    fclose(handle.input_stream);
  }
  if (handle.output_stream)
  {
    fclose(handle.output_stream);
  }

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
             it("should print ready message on startup",
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
             it("should print ready message with checksums",
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
             it("should return error message for invalid JSON input",
                []() -> void
                {
                  // Start the daemon, read first line of output, then stop it
                  string response;
                  DaemonHandle daemon = start_daemon(zowed_command);
                  read_daemon_output(daemon, response);
                  write_to_daemon(daemon, "invalid\n");
                  read_daemon_output(daemon, response);
                  stop_daemon(daemon);

                  Expect(response).ToContain("\"code\":-32700");
                  Expect(response).ToContain("\"message\":\"Failed to parse command request\"");
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
                });
           });
}
