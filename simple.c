// Copyright(C) 2022-2023 Dayan Rodriguez, All rights reserved.
#define ZEN_APPMODE_CONSOLE
#define _ZEN_GLOBAL
#include "brazen.h"

#include "alistar.c"

// Note: Alistar context, one per server-client connection ...
static AlistarContext Ali;

static void
ZenMain(int _, char **)
{
  AlistarCreateContext(&Ali);
  AlistarEstablishConnection(&Ali,true,5679);

  for(;;)
  {
    // Note: update data with previously requested info, request more info
    if(AlistarTick(&Ali))
    {
      // Note: now we have some data to work with ...

      TRACE_I("tick...");
    }

    // Note: sleep here for no reason
    Sleep(250);
  }

  AlistarObliterateConnection(&Ali);

}



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
#if 0

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




