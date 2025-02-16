#include "zrecovery.h"
#include "zwto.h"

#pragma prolog(ZJBMTEST, "&CCN_MAIN SETB 1 \n MYPROLOG")
int ZJBMTEST()
{
  zwto_debug("@TEST routine called under recovery");
  return 0;
}

int main()
{
  zwto_debug("@TEST calling recovery");
  set_recovery(ZJBMTEST);
}
