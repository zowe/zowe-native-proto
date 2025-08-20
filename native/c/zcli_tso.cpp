#include "parser.hpp"
#include "ztso.hpp"
#include "zcli.hpp"
#include <unistd.h>

using namespace parser;
using namespace std;

int handle_tso_issue(const ParseResult &result)
{
  int rc = 0;
  string command = result.find_pos_arg_string("command");
  string response;

  rc = ztso_issue(command, response);

  if (0 != rc)
  {
    cerr << "Error running command, rc '" << rc << "'" << endl;
    cerr << "  Details: " << response << endl;
  }

  cout << response;

  return rc;
}
