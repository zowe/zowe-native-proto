#include "zwto.h"
#include "zstorage.metal.test.h"
#include "zstorage.h"

#pragma prolog(STBT31, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(STBT31, " ZWEEPILG ")
void *STBT31(int size)
{
  return storage_obtain31(size);
}

#pragma prolog(STFREE, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(STFREE, " ZWEEPILG ")
int STFREE(void *data, int size)
{
  storage_release(data, size);
  return 0;
}

#pragma prolog(STGET64, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(STGET64, " ZWEEPILG ")
void *STGET64(int size)
{
  return storage_get64(size);
}

#pragma prolog(STREL, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(STREL, " ZWEEPILG ")
int STREL(void *data)
{
  storage_free64(data);
  return 0;
}