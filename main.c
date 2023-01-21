// ZenTexture *SomeTexture;
// SomeTexture=CreateTexture(ZEN_GPU_READ,1,2,3,SomeMemory);
// ZenTextureMemory TextureMemory;
// TextureMemory=BorrowTextureMemory(SomeTexture);
// ReturnTextureMemory(SomeTexture);

// TODO(RJ):
// Fix proper shader for draw calls ...
// TODO(RJ):
// Fix text rendering ...
// TODO(RJ):
// Fix gui layering ...
// TODO(RJ):
// Fix latent input ...


// Todo: if the dx device were to fail during game startup, the variable config file
// could end up being corrupted because the game didn't have time to write out proper data
// or for some other reason, so stay on the look out, you might have to delete the Variables.txt file ...


#define ZEN_APPMODE_WINDOWED
#include "brazen.h"
#include "alistar.c"

static AlistarContext Ali;

static Unit *
GetUnitByType(AlistarContext *ctx, int type)
{
  ObservationRaw *raw;
  raw=&ctx->last_response.Observation.observation.RawData;

  Unit *units;
  units=raw->units;

  int count=sb_count(units);

  for(int i=0;i<count;++i)
  { if(units[i].unit_type==type)
    { return &units[i];
    }
  }
  return NULL;
}


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

  ZenTexture
    *TerrainTexture=NULL,
    *PlacementGridTexture=NULL,
    *PathingGridTexture=NULL,
    *VisibilityTexture=NULL,
    *CreepTexture=NULL;
  ZenView GameView,PlayerView,
    TerrainTextureView, PathingGridTextureView,PlacementGridTextureView,
      VisibilityTextureView,CreepTextureView;
  PlayerView={ {0,0, 16 * 64, 16 * 32}, "Player" };
  GameView={ {0,0, 16 * 64, 16 * 32}, "Game" };
  TerrainTextureView={ {0,0, 16 * 64, 16 * 32}, "Terrain" };
  PathingGridTextureView={ {0,0, 16 * 64, 16 * 32}, "Pathing" };
  PlacementGridTextureView={ {0,0, 16 * 64, 16 * 32}, "Placement" };
  VisibilityTextureView={ {0,0, 16 * 64, 16 * 32}, "Visibility" };
  CreepTextureView={ {0,0, 16 * 64, 16 * 32}, "Creep" };


  // TODO(RJ):
  Ali.Game.Workers.CountMin = 12;
  Ali.Game.Minerals = 50;
  Ali.Game.Vespene  = 0;

  Point2D build_location;
  (void) build_location;


  while(! Zen.Quitted)
  { ZenWindow *Window;
    Window=ZenGetActiveWindow();

    ZEN_IMGLUI *Glui;
    Glui=&Zen.Graphics.ImGlui;
    ZenFontDebugUI(Glui);

    if(AlistarTick(&Ali))
    {

      VisibilityTexture=CopyImageDataToTexture(&Zen,VisibilityTexture,Ali.last_response.Observation.observation.RenderData.Map);
#if 0
      Unit *cc;
      cc=GetUnitByType(&Ali,18);

      static int sent_chat;
      if(!sent_chat)
      { sent_chat=1;
        AlistarSendChat(&Ali,CHANNEL_BROADCAST,"gl hf");


        build_location={cc->pos.x, cc->pos.y};
        build_location.x += 3;
        build_location.y += 3;
      }


      if(cc)
      {
        int32_t SupplyMin;
        SupplyMin=UnitArrayCount(&Ali.Game.Workers);

        int32_t SupplyMax;
        SupplyMax=15+UnitArrayCount(&Ali.Game.SupplyDepots)*8;


        if(SupplyMin+Ali.Game.WorkersQueued>=SupplyMax+Ali.Game.SupplyDepotsQueued*8)
        {
          if(Ali.Game.Minerals>=100)
          {
            // Todo: Get nearest SCV ...
            Unit *scv;
            scv=GetUnitByType(&Ali,45);

            AlistarRequestBuildDepot(&Ali,scv->tag,build_location);

            build_location.x += 3;
            build_location.y += 3;

            Response res;
            if(!AwaitResponseOfType(&Ali.conn_queue,&res,RESPONSE_TAG_ACTION))
            { __debugbreak();
            }

            Ali.Game.Minerals-=100;
            Ali.Game.SupplyDepotsQueued++;
          }
        }
        if(SupplyMin+Ali.Game.WorkersQueued<SupplyMax)
        { if(Ali.Game.Minerals>=50)
          { if(!Ali.Game.WorkersQueued)
            { AlistarRequestTrainSCV(&Ali,cc->tag);

              Response res;
              if(AwaitResponseOfType(&Ali.conn_queue,&res,RESPONSE_TAG_ACTION))
              { ActionResult result;
                result=res.Action.results[0];

                if(result==ActionResultSuccess)
                { Ali.Game.WorkersQueued++;
                  Ali.Game.Minerals-=50;
                } else
                { __debugbreak();
                }
              }
            }
          }
        }
      }
#endif
    }
    if(0)
    {
#if 0
      if(DequeueResponseOfTypeAndNotifyAll(&ConnQueue,&LastResponse,RESPONSE_TAG_OBSERVATION))
      { GameLoopCounter++;
        HasIt=TRUE;
        Asked=FALSE;

        VisibilityTexture=CopyImageDataToTexture(&Zen,VisibilityTexture,LastResponse.Observation.observation.raw_data.map_state.visibility);
        CreepTexture=CopyImageDataToTexture(&Zen,CreepTexture,LastResponse.Observation.observation.raw_data.map_state.creep);
      }

      if(!TerrainTexture) TerrainTexture=CopyImageDataToTexture(&Zen,TerrainTexture,GameInfo.start_raw.terrain_height);
      if(!PathingGridTexture) PathingGridTexture=CopyImageDataToTexture(&Zen,PathingGridTexture,GameInfo.start_raw.pathing_grid);
      if(!PlacementGridTexture) PlacementGridTexture=CopyImageDataToTexture(&Zen,PlacementGridTexture,GameInfo.start_raw.placement_grid);
#endif
    }

    if(VisibilityTexture)
    {
      VisibilityTextureView.Location.Max.X=VisibilityTexture->DimenX;
      VisibilityTextureView.Location.Max.Y=VisibilityTexture->DimenY;
      if(ZenGluiPushWindow(Glui, &VisibilityTextureView))
      {   ZenGluiTextureView(Glui, NULL, 0, {VisibilityTexture->DimenX,VisibilityTexture->DimenY}, VisibilityTexture);
        ZenGluiPullWindow(Glui);
      }
    }

#if 0
    if(TerrainTexture)
    {
      TerrainTextureView.Location.Max.X=TerrainTexture->DimenX;
      TerrainTextureView.Location.Max.Y=TerrainTexture->DimenY;
      if(ZenGluiPushWindow(Glui, &TerrainTextureView))
      {   ZenGluiTextureView(Glui, NULL, 0, {TerrainTexture->DimenX,TerrainTexture->DimenY}, TerrainTexture);
        ZenGluiPullWindow(Glui);
      }
    }

    if(PathingGridTexture)
    {
      PathingGridTextureView.Location.Max.X=PathingGridTexture->DimenX;
      PathingGridTextureView.Location.Max.Y=PathingGridTexture->DimenY;
      if(ZenGluiPushWindow(Glui, &PathingGridTextureView))
      {   ZenGluiTextureView(Glui, NULL, 0, {PathingGridTexture->DimenX,PathingGridTexture->DimenY}, PathingGridTexture);
        ZenGluiPullWindow(Glui);
      }
    }

    if(PlacementGridTexture)
    {
      PlacementGridTextureView.Location.Max.X=PlacementGridTexture->DimenX;
      PlacementGridTextureView.Location.Max.Y=PlacementGridTexture->DimenY;
      if(ZenGluiPushWindow(Glui, &PlacementGridTextureView))
      {   ZenGluiTextureView(Glui, NULL, 0, {PlacementGridTexture->DimenX,PlacementGridTexture->DimenY}, PlacementGridTexture);
        ZenGluiPullWindow(Glui);
      }
    }


    if(CreepTexture)
    {
      CreepTextureView.Location.Max.X=CreepTexture->DimenX;
      CreepTextureView.Location.Max.Y=CreepTexture->DimenY;
      if(ZenGluiPushWindow(Glui, &CreepTextureView))
      {   ZenGluiTextureView(Glui, NULL, 0, {CreepTexture->DimenX,CreepTexture->DimenY}, CreepTexture);
        ZenGluiPullWindow(Glui);
      }
    }

    if(HasIt)
    { PlayerCommon *Player;
      Player=&LastResponse.Observation.observation.player_common;

      PlayerView.Title=FormatA("Player %i", Player->player_id);
      if(ZenGluiPushWindow(Glui, &PlayerView))
      {   ZenGluiTextView(Glui,NULL,0,32.f,FormatA("PlayerId: %i", Player->player_id));
          ZenGluiTextView(Glui,NULL,0,32.f,FormatA("Minerals: %i", Player->minerals));
          ZenGluiTextView(Glui,NULL,0,32.f,FormatA("Vespene: %i", Player->vespene));
          ZenGluiTextView(Glui,NULL,0,32.f,FormatA("FoodCap: %i", Player->food_cap));
          ZenGluiTextView(Glui,NULL,0,32.f,FormatA("FoodUsed: %i", Player->food_used));
          ZenGluiTextView(Glui,NULL,0,32.f,FormatA("FoodArmy: %i", Player->food_army));
          ZenGluiTextView(Glui,NULL,0,32.f,FormatA("FoodWorkers: %i", Player->food_workers));
          ZenGluiTextView(Glui,NULL,0,32.f,FormatA("IdleWorkerCount: %i", Player->idle_worker_count));
          ZenGluiTextView(Glui,NULL,0,32.f,FormatA("ArmyCount: %i", Player->army_count));
          ZenGluiTextView(Glui,NULL,0,32.f,FormatA("WarpGateCount: %i", Player->warp_gate_count));
          ZenGluiTextView(Glui,NULL,0,32.f,FormatA("LarvaCount: %i", Player->larva_count));
        ZenGluiPullWindow(Glui);
      }
    }
    if(ZenGluiPushWindow(Glui, &GameView))
    {   ZenGluiTextView(Glui,NULL,0,32.f,FormatA("GameLoopCounter: %i", GameLoopCounter));
      ZenGluiPullWindow(Glui);
    }
#endif

    ZenTick();
  }

  AlistarObliterateConnection(&Ali);

}



