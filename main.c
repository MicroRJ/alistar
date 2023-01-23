// Unrelated todos:
// fix text rendering ...
// fix gui layering ...
// fix latent input ...
// cross-platform compatibility ...
#define ZEN_APPMODE_WINDOWED
#include "brazen.h"
#include "alistar.c"

static AlistarContext Ali;

static i32
ZenBackgroundFunc(ZenCore *Core, void*)
{
  AlistarEstablishConnection(&Ali,true,5679);

  return 1;
}

static void
ZenMain(int _, char **)
{
  ZenInitialize({}, L"Alistar Command Center");

  AlistarCreateContext(&Ali);

  ZenBackground(ZenBackgroundFunc);

  ZenView view_pool[0x20]={};

  // TODO(RJ):
  // Ali.Game.Workers.CountMin = 12;
  // Ali.Game.Minerals = 50;
  // Ali.Game.Vespene  = 0;

  Point2D build_location;
  (void) build_location;

  while(!Zen.Quitted)
  {
    ZenWindow *Window;
    Window=ZenGetActiveWindow();

    ZEN_IMGLUI *Glui;
    Glui=&Zen.Graphics.ImGlui;
    ZenFontDebugUI(Glui);

    if(AlistarTick(&Ali))
    {

      for(int i=0;i<ARRAYSIZE(Ali.Game.Textures);++i)
      {
        ZenTexture *tex;
        tex=Ali.Game.Textures[i];
        if(tex)
        { view_pool[i].Location.Max.X=tex->DimenX;
          view_pool[i].Location.Max.Y=tex->DimenY;
          view_pool[i].Title=AliFieldStrings_Game[i];

          if(ZenGluiPushWindow(Glui, &view_pool[i]))
          {   ZenGluiTextureView(Glui, NULL, 0, {tex->DimenX,tex->DimenY}, tex);
            ZenGluiPullWindow(Glui);
          }
        }
      }
    }

    ZenTick();
  }
  AlistarObliterateConnection(&Ali);
}



