#define ZEN_APPMODE_CONSOLE
#include "brazen.h"

#include "alistar.c"

static AlistarContext Ali;

static void
ZenMain(int _, char **)
{
  AlistarCreateContext(&Ali);
  AlistarEstablishConnection(&Ali,true,5679);

  for(;;)
  { if(AlistarTick(&Ali))
    {
      TRACE_I("tick...");
    }
    Sleep(250);
  }

  AlistarObliterateConnection(&Ali);

}



