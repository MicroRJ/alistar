#include "rx\rx.c"


INT32 WINAPI
WinMain(HINSTANCE inst, HINSTANCE prev_inst, PSTR cmdl, INT cmds)
{
  (void)inst;
  (void)prev_inst;
  (void)cmdl;
  (void)cmds;

  rxinit(L"Alistar - Command Center");

  rxtexture_t texture=rxload_texture_file("media\\alistar_0.1.PNG");

  for(;;)
  {
    rxdraw(texture,rx.LinearSampler,0,0,1080,720);

    rxtick();
  }
}



#if 0
#define ZEN_APPMODE_WINDOWED
#include "brazen.h"
#include "alistar.c"
static AlistarContext Ali;
static i32
ZenBackgroundFunc(ZenCore *Core, void*)
{

  return 1;
}

static void
ZenMain(int _, char **)
{
  ZenInitialize(L"Alistar Command Center");

  AlistarCreateContext(&Ali);
  AlistarEstablishConnection(&Ali,true,5679);

  // ZenBackground(ZenBackgroundFunc);

  ZenView view_pool[0x20]={};

  // TODO(RJ):
  // Ali.Game.Workers.CountMin = 12;
  // Ali.Game.Minerals = 50;
  // Ali.Game.Vespene  = 0;

  Point2D build_location;
  (void) build_location;

  do
  {
    ZenWindow *Window;
    Window=ZenGetActiveWindow();

    ZEN_IMGLUI *Glui;
    Glui=&Zen.ImGlui;

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

  } while(ZenTick());

  AlistarObliterateConnection(&Ali);
}
#endif