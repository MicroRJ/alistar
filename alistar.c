// Copyright(C) 2022-2023 Dayan Rodriguez, All rights reserved.
#ifndef ALISTAR_GLUE
#define ALISTAR_GLUE

// TODO(RJ): REMOVE CIVETWEB
#define MG_EXPERIMENTAL_INTERFACES
#define USE_WEBSOCKET
#define OPENSSL_API_1_0
#include "civetweb.h"
#include "civetweb.c"

#define ali_malloc(size) mg_malloc(size)
#define ali_realloc(memory,size) mg_realloc(memory,size)
#define ali_calloc(size,count) mg_calloc(size,count)
#define ali_free(memory) mg_free(memory)

#include "stretchy_buffer.h"
#include "ali-allocator.c"
#include "ali-process.c"
#include "ali.c"
#include "alistar.h"
#include "ali-zen.c"

ALISTAR_PARSE_FUNCTION void
ParseRequest(xstate *read, Request *val);

static int
AlistarSendPayload(AlistarContext *ctx, int tag, xvalue *pay)
{
  // TODO(RJ): DELETE THE VALUE
  xvalue req={ali_msg_type};
  AddValue(&req,tag,*pay);

  xblock mem;
  mem=SerializeValue(&req);

  xstate read={};
  Apportion(&read,mem.len,mem.mem);
  Request pars={};
  ParseRequest(&read, &pars);

  int snd;
  snd=mg_websocket_write(ctx->conn,MG_WEBSOCKET_OPCODE_BINARY,(char*)mem.mem,mem.len);

  DelBlock(&mem);

  DelValue(&req);

  return snd;
}

static int
AlistarCreateContext(AlistarContext *ctx)
{ ZeroMemory(ctx,sizeof(*ctx));
  ali_thread_init(&ctx->conn_queue);

#if 0
  unsigned int file_size;
  void *file_data;
  file_data=LoadFileData(&file_size,"last_response.txt");
  if(file_data)
  {
    Response res;
    ParseResponse(&res,file_size,file_data);

    ZenTexture tex;
    tex=AliConvertTexture(&Zen,res.Observation.observation.RenderData.Map);

    ZenWriteTextureToFile(&tex, "last_map.png");
  }
#endif

  return 1;
}

static int
AlistarOnSocketDataReceived(mg_connection* conn, int flags, char *data, size_t size, void *user)
{ AlistarContext *ctx;
  ctx=(AlistarContext*)user;

  // Todo:
  WriteFileData((unsigned int)size,data,"last_response.txt");

  xstate read={};
  Apportion(&read,size,data);

  Response *response;
  response=ali_thread_allocate_response(&ctx->conn_queue);

  ParseResponse(&read,response);
  ali_thread_write_message(&ctx->conn_queue,response->Type,read.arena,response);

  return 1;
}

static void
AlistarOnSocketConnectionClosed(const mg_connection* conn, void *user)
{ AlistarContext *ctx;
  ctx=(AlistarContext*)user;

  Response *response;
  response=ali_thread_allocate_response(&ctx->conn_queue);

  ali_thread_write_message(&ctx->conn_queue,RESPONSE_TAG_CONNECTION_ENDED,{},response);
}

static void
AlistarObliterateConnection(AlistarContext *ctx)
{
  ZenTerminateSystemProcessForcefully(&ctx->proc,777);
}

static int
AlistarEstablishConnection(AlistarContext *ctx, int is_realtime, int port)
{
  auto addr="127.0.0.1";
  port=5679;
  const char *conn_options[] =
  { "request_timeout_ms",   "100000",
    "websocket_timeout_ms", "100000",
    "num_threads",          "1",
    "tcp_nodelay",          "1", 0,
  };

  mg_callbacks conn_callbacks;
  mg_context *conn_context;

  conn_callbacks={};
  conn_context=mg_start(&conn_callbacks, NULL, conn_options);

  ctx->proc=LaunchStarcraftProcess("G:\\StarCraft II",FALSE,addr,port);

  char errbuf[0x100];
  ZeroMemory(errbuf,sizeof(errbuf));

  i32 tries;
  // Note: establish a proper connection
  for(ctx->conn=0,tries=0;!ctx->conn&&tries<7;++tries)
  { ctx->conn=mg_connect_websocket_client(addr,port,FALSE,errbuf,sizeof(errbuf),
      "/sc2api",NULL,AlistarOnSocketDataReceived,AlistarOnSocketConnectionClosed,ctx);
  }

  ali_thread_message message;
  RequestAvailableMaps(ctx);

  if(ali_thread_await_message(&ctx->conn_queue,RESPONSE_TAG_AVAILABLE_MAPS,TRUE,&message))
  { RequestCreateGame(ctx);

    if(ali_thread_await_message(&ctx->conn_queue,RESPONSE_TAG_CREATE_GAME,TRUE,&message))
    { RequestJoinGameAsParticipant(ctx,TERRAN,"RoyJacobs");

      if(ali_thread_await_message(&ctx->conn_queue,RESPONSE_TAG_JOIN_GAME,TRUE,&message))
      { RequestGameInfo(ctx);

        if(ali_thread_await_message(&ctx->conn_queue,RESPONSE_TAG_GAME_INFO,TRUE,&message))
        {
          ctx->ready_for_obs=1;
        }
      }
    }
  }

  return ctx->conn!=0;
}

static int
AlistarTick(AlistarContext *ctx)
{
  if((ctx->ready_for_obs) && (!ctx->requested_obs))
  { ctx->requested_obs=1;
    RequestObservation(ctx,TRUE,ctx->Game.loop);
  }

  // Todo: handle all events in the queue ...
  ali_thread_message message;
  if(ali_thread_read_message(&ctx->conn_queue,RESPONSE_TAG_OBSERVATION,TRUE,&message))
  {
    ctx->requested_obs=0;

    // Note: check if we have at least one message cached ...
    if(ctx->tick_ready) ali_thread_release_message(&ctx->tick_message);

    ctx->tick_message=message;

    ctx->tick_ready=1;


    ctx->Game.TextureMap=CopyImageDataToTexture(&Zen,ctx->Game.TextureMap,
      message.response->Observation.observation.RenderData.Map);
    ctx->Game.TextureMinimap=CopyImageDataToTexture(&Zen,ctx->Game.TextureMinimap,
      message.response->Observation.observation.RenderData.Minimap);


    AliFeatureLayers Renders;
    Renders=message.response->Observation.observation.FeatureLayerData.Renders;

    ctx->Game.TextureHeightMap=CopyImageDataToTexture(&Zen,ctx->Game.TextureHeightMap, Renders.HeightMap);
    ctx->Game.TextureVisibilityMap=CopyImageDataToTexture(&Zen,ctx->Game.TextureVisibilityMap, Renders.VisibilityMap);
    // ctx->Game.TextureCreep=CopyImageDataToTexture(&Zen,ctx->Game.TextureCreep, Renders.Creep);
    // ctx->Game.TexturePower=CopyImageDataToTexture(&Zen,ctx->Game.TexturePower, Renders.Power);
    // ctx->Game.TexturePlayerId=CopyImageDataToTexture(&Zen,ctx->Game.TexturePlayerId, Renders.PlayerId);
    // ctx->Game.TextureUnitType=CopyImageDataToTexture(&Zen,ctx->Game.TextureUnitType, Renders.UnitType);
    // ctx->Game.TextureSelected=CopyImageDataToTexture(&Zen,ctx->Game.TextureSelected, Renders.Selected);
    ctx->Game.TextureUnitHitPoints=CopyImageDataToTexture(&Zen,ctx->Game.TextureUnitHitPoints, Renders.UnitHitPoints);
    // ctx->Game.TextureUnitHitPointsRatio=CopyImageDataToTexture(&Zen,ctx->Game.TextureUnitHitPointsRatio, Renders.UnitHitPointsRatio);
    // ctx->Game.TextureUnitEnergy=CopyImageDataToTexture(&Zen,ctx->Game.TextureUnitEnergy, Renders.UnitEnergy);
    // ctx->Game.TextureUnitEnergyRatio=CopyImageDataToTexture(&Zen,ctx->Game.TextureUnitEnergyRatio, Renders.UnitEnergyRatio);
    // ctx->Game.TextureUnitShields=CopyImageDataToTexture(&Zen,ctx->Game.TextureUnitShields, Renders.UnitShields);
    // ctx->Game.TextureUnitShieldsRatio=CopyImageDataToTexture(&Zen,ctx->Game.TextureUnitShieldsRatio, Renders.UnitShieldsRatio);
    // ctx->Game.TexturePlayerRelative=CopyImageDataToTexture(&Zen,ctx->Game.TexturePlayerRelative, Renders.PlayerRelative);
    ctx->Game.TextureUnitDensityAA=CopyImageDataToTexture(&Zen,ctx->Game.TextureUnitDensityAA, Renders.UnitDensityAA);
    // ctx->Game.TextureUnitDensity=CopyImageDataToTexture(&Zen,ctx->Game.TextureUnitDensity, Renders.UnitDensity);
    // ctx->Game.TextureEffects=CopyImageDataToTexture(&Zen,ctx->Game.TextureEffects, Renders.Effects);
    // ctx->Game.TextureHallucinations=CopyImageDataToTexture(&Zen,ctx->Game.TextureHallucinations, Renders.Hallucinations);
    // ctx->Game.TextureCloaked=CopyImageDataToTexture(&Zen,ctx->Game.TextureCloaked, Renders.Cloaked);
    // ctx->Game.TextureBlip=CopyImageDataToTexture(&Zen,ctx->Game.TextureBlip, Renders.Blip);
    // ctx->Game.TextureBuffs=CopyImageDataToTexture(&Zen,ctx->Game.TextureBuffs, Renders.Buffs);
    // ctx->Game.TextureBuffDuration=CopyImageDataToTexture(&Zen,ctx->Game.TextureBuffDuration, Renders.BuffDuration);
    // ctx->Game.TextureActive=CopyImageDataToTexture(&Zen,ctx->Game.TextureActive, Renders.Active);
    // ctx->Game.TextureBuildProgress=CopyImageDataToTexture(&Zen,ctx->Game.TextureBuildProgress, Renders.BuildProgress);
    ctx->Game.TextureBuildable=CopyImageDataToTexture(&Zen,ctx->Game.TextureBuildable, Renders.Buildable);
    ctx->Game.TexturePathable=CopyImageDataToTexture(&Zen,ctx->Game.TexturePathable, Renders.Pathable);
    ctx->Game.TexturePlaceholder=CopyImageDataToTexture(&Zen,ctx->Game.TexturePlaceholder, Renders.Placeholder);

#if 0
    UnitArrayFree(&ctx->Game.CommandCenters);
    UnitArrayFree(&ctx->Game.SupplyDepots);
    UnitArrayFree(&ctx->Game.Workers);
    UnitArrayFree(&ctx->Game.WorkersIdling);
    UnitArrayFree(&ctx->Game.WorkersHarvesting);
    UnitArrayFree(&ctx->Game.WorkersBuilding);
    UnitArrayFree(&ctx->Game.WorkersRepairing);

    ctx->Game.WorkersQueued=0;
    ctx->Game.SupplyDepotsQueued=0;

    ctx->Game.loop++;

    Observation obs;
    obs=response.Observation.observation;

    ObservationRaw raw;
    raw=obs.RawData;

    Unit *unit_array;
    unit_array=raw.units;

    int unit_count;
    unit_count=sb_count(unit_array);

    for(int i=0;i<unit_count;++i)
    { Unit *unit;
      unit=unit_array+i;

      if(unit->unit_type==18)
      { *UnitArrayAdd(&ctx->Game.CommandCenters)=*unit;

        for(int i=0;i<sb_count(unit->orders);++i)
        { if(unit->orders[i].ability_id==524)
          { ctx->Game.WorkersQueued++;
          }
        }
      } else
      if(unit->unit_type==19)
      { *UnitArrayAdd(&ctx->Game.SupplyDepots)=*unit;
      } else
      if(unit->unit_type==45)
      { *UnitArrayAdd(&ctx->Game.Workers)=*unit;

        if(!sb_count(unit->orders))
        { *UnitArrayAdd(&ctx->Game.WorkersIdling)=*unit;
        } else
        {
          for(int i=0;i<sb_count(unit->orders);++i)
          { if(unit->orders[i].ability_id==319)
            { ctx->Game.SupplyDepotsQueued++;
            }
          }
        }
      }
    }


    PlayerCommon common;
    common=obs.PlayerCommon;
    ctx->Game.Minerals=common.minerals;
    ctx->Game.Vespene=common.vespene;
#endif
  }

  return ctx->tick_ready;
}





#endif