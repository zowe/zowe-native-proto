#include <iostream>
#include <string>
#include "zcn.hpp"
#include "zut.hpp"
#include "ztype.h"

int main()
{

  ZCN zcn = {0};

  int rc = 0;

  printf("Starting, current key is %02x\n", zut_get_key());

  rc = zcn_activate(&zcn, "DKELOSKY");
  if (0 != rc)
  {
    std::cerr << "Error: Activating console failed with " << rc << " and " << std::string(zcn.diag.e_msg) << std::endl;
    return -1;
  }

  printf("Console activated, now key is %02x\n", zut_get_key());

  rc = zcn_put(&zcn, "D IPLINFO");
  if (0 != rc)
  {
    std::cerr << "Error: Dectivating console failed with " << rc << " and " << std::string(zcn.diag.e_msg) << std::endl;
    return -1;
  }

  printf("Console put, now key is %02x\n", zut_get_key());

  rc = zcn_deactivate(&zcn);
  if (0 != rc)
  {
    std::cerr << "Error: Putting to console failed with " << rc << " and " << std::string(zcn.diag.e_msg) << std::endl;
    return -1;
  }

  printf("Console deactivate, now key is %02x\n", zut_get_key());

  return 0;
}
