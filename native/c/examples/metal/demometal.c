
#include "zwto.h"
#include "zmetal.h"

#pragma prolog(main, " ZWEPROLG NEWDSA=(YES,128) ")
#pragma epilog(main, " ZWEEPILG ")

void test()
{
  zwto_debug("test called");
}

int main()
{
  PSW psw = {0};
  get_psw(&psw);
  int mode_switch = psw.p ? 0 : 1;

  return 0;
}