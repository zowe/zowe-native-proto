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

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <thread>

using namespace std;

int main()
{
  std::ifstream file("input.txt"); // Replace "example.txt" with your file name

  if (!file.is_open())
  {
    std::cerr << "Error opening file!" << std::endl;
    return 1;
  }
  printf("@TEST1 %x\n", file.rdstate());

  // read to the end
  std::string line;
  while (std::getline(file, line))
  {
    // cout << "got a line" << line << endl;
  }

  cout << "end of file" << endl;

  cout << "slieeping for 10 sec" << endl;
  std::this_thread::sleep_for(std::chrono::seconds(10)); // Sleep for 2 seconds

  // file.clear(); // = 0;
  printf("@TEST2 %x\n", file.rdstate());
  // file.clear(file.rdstate() & ~fstream::eofbit);
  // file.clear(file.rdstate() & ~fstream::failbit);
  file.clear();
  printf("@TEST3 %x\n", file.rdstate());
  // file.seekg(0);

  while (std::getline(file, line))
  {
    cout << "got a line after eof " << line << endl;
  }

  cout << "ending after 10 sec" << endl;

  file.close();
  return 0;
}