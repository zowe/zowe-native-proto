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
#include "../../c/test/ztest.hpp"
#include "../../c/ztype.h"
#include "zowed.test.hpp"

using namespace std;
using namespace ztst;

struct DaemonHandle
{
  pid_t pid;
  FILE *output_stream;
  FILE *input_stream;
};

std::string read_line_from_daemon(DaemonHandle &handle, int timeout_ms = 5000)
{
  fd_set read_fds;
  struct timeval timeout;
  int fd = fileno(handle.output_stream);
  if (fd == -1)
  {
    throw std::runtime_error("Failed to get file descriptor");
  }

  FD_ZERO(&read_fds);
  FD_SET(fd, &read_fds);

  timeout.tv_sec = timeout_ms / 1000;
  timeout.tv_usec = (timeout_ms % 1000) * 1000;

  int result = select(fd + 1, &read_fds, nullptr, nullptr, &timeout);
  if (result <= 0)
  {
    throw std::runtime_error("Timeout waiting for daemon output");
  }

  char buffer[512];
  if (fgets(buffer, sizeof(buffer), handle.output_stream))
  {
    return std::string(buffer);
  }

  throw std::runtime_error("Failed to read from daemon");
}

void write_to_daemon(DaemonHandle &handle, const std::string &input)
{
  if (fputs(input.c_str(), handle.input_stream) == EOF || fflush(handle.input_stream) != 0)
  {
    throw std::runtime_error("Failed to write to daemon");
  }
}

DaemonHandle start_daemon(const std::string &command, bool read_ready_message = false)
{
  int output_pipe[2];
  int input_pipe[2];

  if (pipe(output_pipe) == -1 || pipe(input_pipe) == -1)
  {
    throw std::runtime_error("Failed to create pipes");
  }

  pid_t pid = fork();
  if (pid == -1)
  {
    throw std::runtime_error("Failed to fork process");
  }

  if (pid == 0)
  {
    close(output_pipe[0]);
    close(input_pipe[1]);

    dup2(input_pipe[0], STDIN_FILENO);
    dup2(output_pipe[1], STDOUT_FILENO);
    dup2(output_pipe[1], STDERR_FILENO);

    close(input_pipe[0]);
    close(output_pipe[1]);

    const char *shell = getenv("SHELL") ? getenv("SHELL") : "/bin/sh";
    execl(shell, shell, "-c", command.c_str(), (char *)nullptr);
    _exit(127);
  }

  close(output_pipe[1]);
  close(input_pipe[0]);

  FILE *output_stream = fdopen(output_pipe[0], "r");
  FILE *input_stream = fdopen(input_pipe[1], "w");

  if (!output_stream || !input_stream)
  {
    if (output_stream)
      fclose(output_stream);
    if (input_stream)
      fclose(input_stream);
    kill(pid, SIGKILL);
    waitpid(pid, nullptr, 0);
    throw std::runtime_error("Failed to open streams");
  }

  auto handle = DaemonHandle{pid, output_stream, input_stream};
  if (read_ready_message)
  {
    read_line_from_daemon(handle);
  }

  return handle;
}

void stop_daemon(DaemonHandle &handle)
{
  kill(handle.pid, SIGINT);
  fclose(handle.input_stream);
  fclose(handle.output_stream);
  waitpid(handle.pid, nullptr, 0);
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
                  DaemonHandle daemon = start_daemon(zowed_command);
                  string response = read_line_from_daemon(daemon);
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

                  DaemonHandle daemon = start_daemon(zowed_command);
                  string response = read_line_from_daemon(daemon);
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
                  DaemonHandle daemon = start_daemon(zowed_command, true);
                  write_to_daemon(daemon, "invalid\n");
                  string response = read_line_from_daemon(daemon);
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
