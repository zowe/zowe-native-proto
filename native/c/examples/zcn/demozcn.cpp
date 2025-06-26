#include <iostream>
#include <string>
#include "zcn.hpp"
#include "zut.hpp"
#include "ztype.h"

int main()
{

  ZCN zcn = {0};

  int rc = 0;

  std::cout << "Starting..." << std::endl;

  printf("Current key is %02x\n", zut_get_key());

  rc = zcn_activate(&zcn, "DKELOSKY");
  if (0 != rc)
  {
    std::cerr << "Error: Activating console failed with " << rc << " and " << std::string(zcn.diag.e_msg) << std::endl;
    return -1;
  }

  printf("Now key is %02x\n", zut_get_key());

  std::cout << "Console activated" << std::endl;

  rc = zcn_deactivate(&zcn);
  if (0 != rc)
  {
    std::cerr << "Error: Dectivating console failed with " << rc << " and " << std::string(zcn.diag.e_msg) << std::endl;
    return -1;
  }

  std::cout << "Completed!" << std::endl;

  return 0;
}