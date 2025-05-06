#include "zmetal.h"
#include "zwto.h"

int main()
{
  char mod[] = "IEFBR15";
  void *ep = load_module(mod);
  if (ep)
  {
    zwto_debug("@TEST loaded");
  }
  else
  {
    zwto_debug("@TEST not loaded");
  }
  return 5;
}