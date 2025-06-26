
#include "zwto.h"
#include "zmetal.h"

#pragma prolog(main, " ZWEPROLG NEWDSA=(YES,128) ")
#pragma epilog(main, " ZWEEPILG ")

int main()
{
  unsigned char key = get_key();

  PSW psw = {0};

  zwto_debug("key initial %02x", key);

  set_key(key);

  // unsigned char env = get_env();
  // zwto_debug("env %02x", env);

  unsigned long long int psw_raw = get_psw();
  memcpy(&psw, &psw_raw, sizeof(PSW));
  zwto_debug("psw %016llx", psw_raw);

  if (psw.p)
  {
    zwto_debug("problem state");
  }
  else
  {
    zwto_debug("supervisor state");
  }

  if (psw.ea)
  {
    zwto_debug("extended addressing mode");
  }
  if (psw.ba)
  {
    zwto_debug("basic addressing mode");
  }

  key = get_key();
  zwto_debug("key after %02x", key);

  return 0;
}