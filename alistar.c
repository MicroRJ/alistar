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
#include "alistar.h"

static int
BitsToBytes(unsigned char *bytes, int bits)
{ bytes[0]=0xff*(bits&0x80);bytes[4]=0xff*(bits&0x08);
  bytes[1]=0xff*(bits&0x40);bytes[5]=0xff*(bits&0x04);
  bytes[2]=0xff*(bits&0x20);bytes[6]=0xff*(bits&0x02);
  bytes[3]=0xff*(bits&0x10);bytes[7]=0xff*(bits&0x01);
  return 8;
}

// TODO(RJ): REMOVE FROM HERE
// TODO(RJ): THIS IS TEMPORARY
ZEN_FUNCTION ZenTexture // COMPILER(RJ): FORCEINLINE
ZenCoreCreateSoftTextureForThisFrame(ZenCore *Core, i32 Format, i32 DimenX, i32 DimenY)
{ void *Memory = ZenCoreFrameAlloc(Core, ZenComputeTrimmedSizeForTexture(Format, DimenX, DimenY));
  ZenTexture Texture = ZenUnboundTexture(DE_CPU_ACCESS, Format, DimenX, DimenY, Memory);
  // NOTE(RJ): ALLOCATIONS ARE ZEROED ALREADY, BUT I KNOW THAT'LL CHANGE!
  ZenZeroTextureMemory(&Texture);
  return Texture;
}

static ZenTexture
AliConvertTexture(ZenCore *Core, ImageData data)
{
  ZenTexture tex={};
  if(data.bits_per_pixel==1)
  { tex=ZenCoreCreateSoftTextureForThisFrame(Core,PIXEL_FORMAT_RGB8,data.size.x,data.size.y);

    unsigned char *loc,*cur,*end;
    loc=tex.Single;
    end=data.bytes+((data.size.x*data.size.y)>>3);
    for(cur=data.bytes;cur<end;++cur)
    { loc+=BitsToBytes(loc,*cur);
    }
  } else
  if(data.bits_per_pixel==8)
  { tex=ZenUnboundTexture(DE_GPU_ACCESS,PIXEL_FORMAT_RGB8,data.size.x,data.size.y,data.bytes);
  } else
  if(data.bits_per_pixel==24)
  { tex=ZenCoreCreateSoftTextureForThisFrame(Core,PIXEL_FORMAT_RGBA8888,data.size.x,data.size.y);

    unsigned char *loc,*cur,*end;
    loc=tex.Single;

    end=data.bytes+(data.size.x*data.size.y)*3;

    for(cur=data.bytes;cur<end;cur+=3,loc+=4)
    { loc[0]=cur[2];
      loc[1]=cur[1];
      loc[2]=cur[0];
      loc[3]=0xff;
    }
  }

  return tex;
}

static ZenTexture *
CopyImageDataToTexture(ZenCore *Core, ZenTexture *Texture, ImageData data)
{

  if((data.size.x==0)||(data.size.y==0)||(data.bytes==0)||(data.bits_per_pixel==0)) return 0;

  ZenTexture tex;
  tex=AliConvertTexture(Core,data);

  if(!Texture)
  { Texture=ZenCoreCreateTexture(Core,DE_GPU_READ|DE_CPU_WRITE,tex.Format,tex.DimenX,tex.DimenY,tex.Memory);
  } else
  { if(ZenCoreBorrowTextureChronicle(Core, Texture, ZEN_WRITE))
    { ZenCopyTexture(Texture, &tex);
      ZenCoreReturnTextureChronicle(Core, Texture);
    }
  }

  return Texture;
}

#define AliDequeueEventOfTypeAndNotifyAll(Queue,Value,Type) AliDequeueEventOfTypeAndNotifyAll_(ZenGenerateCallerInfo(),Queue,Value,Type)

static int
AliDequeueEventOfTypeAndNotifyAll_(ZenCaller Caller, AlistarQueue *Queue, Response *Value, int Type)
{ // TRACE_I("%s", ZenCallerW(Caller));
  int Success;
  Success=FALSE;
  if(ZenAcquireNativeMutex(Queue->Mutex))
  { if(Queue->CountMin)
    { Response *This,*Last;
      This=Queue->Array;
      Last=Queue->Array+Queue->CountMin-1;
      for(;This<=Last;++This)
      { if(This->Type==Type)
        { *Value=*This;
          if(This!=Last)
          { *This=*Last;
          }
          Queue->CountMin=Queue->CountMin-1;
          Success=TRUE;
          break;
        }
      }
    } else
    {
      // Note: keep the event signaled until you don't have any items?
      ZenResetNativeEvent(Queue->Event);
    }
    ZenReleaseNativeMutexEnsured(Queue->Mutex);
  }
  return Success;
}

static int
AliDequeueEventAndNotifyAll_(ZenCaller Caller, AlistarQueue *Queue, Response *Value)
{ // TRACE_I("%s", ZenCallerW(Caller));

  int Success;
  Success=FALSE;
  if(ZenAcquireNativeMutex(Queue->Mutex))
  { if(Queue->CountMin)
    { Queue->CountMin --;
      Response *Read;
      Read=Queue->Array + Queue->CountMin;
      *Value=*Read;
      Success=TRUE;
    } else
    {
      // Note: keep the event signaled until you don't have any items?
      ZenResetNativeEvent(Queue->Event);
    }
    ZenReleaseNativeMutexEnsured(Queue->Mutex);
  }
  return Success;
}

#define AliQueueEventAndNotifyAll(Queue,Value) AliQueueEventAndNotifyAll_(ZenGenerateCallerInfo(),Queue,Value)

static int
AliQueueEventAndNotifyAll_(ZenCaller Caller, AlistarQueue *Queue, Response *Value)
{ // TRACE_I("%s", ZenCallerW(Caller));

  if(ZenAcquireNativeMutex(Queue->Mutex))
  {
    if(Queue->CountMin<Queue->CountMax)
    { Response *Write;
      Write=Queue->Array+Queue->CountMin++;
      *Write=*Value;
    } else
    {
      TRACE_E("Queue Filled Already");
    }

    ZenReleaseNativeMutexEnsured(Queue->Mutex);
    ZenProduceNativeEvent(Queue->Event);
    return 1;
  }
  return 0;
}

#if 0
#define AwaitResponseForever(Queue,Value) AwaitResponseForever_(ZenGenerateCallerInfo(),Queue,Value)
static int
AwaitResponseForever_(ZenCaller Caller, AlistarQueue *Queue, Response *Value)
{ while(!AliDequeueEventAndNotifyAll_(Caller,Queue,Value))
  { ZenConsumeNativeEvent(Queue->Event);
  }
  return 1;
}
#endif

#define AwaitResponseOfType(Queue,Value,Type) AwaitResponseOfType_(ZenGenerateCallerInfo(),Queue,Value,Type)

static int
AwaitResponseOfType_(ZenCaller Caller, AlistarQueue *Queue, Response *Value, int Type)
{
  if(!AliDequeueEventOfTypeAndNotifyAll_(Caller,Queue,Value,Type))
  { ZenConsumeNativeEvent(Queue->Event);
    return AliDequeueEventOfTypeAndNotifyAll_(Caller,Queue,Value,Type);
  }

  return 1;
}

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
#if 0
  uint64_t test_value, test_compare;
  test_value=4346871809;
  xvalue test={xval_t};
  AddVarint64Value(&test, 1, test_value);
  xblock seri;
  seri=SerializeValue(&test);
  xstate read={};
  Apportion(&read,seri.len,seri.mem);
  ForMessageField(&read)
  { case 1: GetVarint64Value(&read, &test_compare); break;
  }
  Assert(test_value==test_compare);
#endif

  // TODO(RJ):
  AlistarQueue que;
  que.Mutex=ZenCreateNativeMutex(FALSE,NULL);
  que.Event=ZenCreateNativeEvent(FALSE,FALSE,NULL);
  que.CountMax=0x1000;
  que.CountMin=0;
  que.Array=(Response*)mg_malloc(sizeof(*que.Array)*que.CountMax);
  ctx->conn_queue=que;

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

  return 1;
}

static ZenSystemProcess
LaunchStarcraftProcess(const char *inst, int arch64, const char *addr, int port);

static int
AlistarOnSocketDataReceived(mg_connection* conn, int flags, char *data, size_t size, void *user)
{

  WriteFileData((unsigned int)size,data,"last_response.txt");



  AlistarContext *ctx;
  ctx=(AlistarContext*)user;
  ctx->last_response.Type=RESPONSE_TAG_CONNECTION_ENDED;

  ParseResponse(&ctx->last_response,size,data);

  AliQueueEventAndNotifyAll(&ctx->conn_queue,&ctx->last_response);
  return 1;
}

static void
AlistarOnSocketConnectionClosed(const mg_connection* conn, void *user)
{
  AlistarContext *ctx;
  ctx=(AlistarContext*)user;
  ctx->last_response.Type=RESPONSE_TAG_CONNECTION_ENDED;

  AliQueueEventAndNotifyAll(&ctx->conn_queue,&ctx->last_response);
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

  Response response;
  RequestAvailableMaps(ctx);
  if(AwaitResponseOfType(&ctx->conn_queue,&response,RESPONSE_TAG_AVAILABLE_MAPS))
  { RequestCreateGame(ctx);

    if(AwaitResponseOfType(&ctx->conn_queue,&response,RESPONSE_TAG_CREATE_GAME))
    { RequestJoinGameAsParticipant(ctx,TERRAN,"RoyJacobs");

      if(AwaitResponseOfType(&ctx->conn_queue,&response,RESPONSE_TAG_JOIN_GAME))
      { RequestGameInfo(ctx);

        if(AwaitResponseOfType(&ctx->conn_queue,&response,RESPONSE_TAG_GAME_INFO))
        {
          ctx->ready_for_obs=1;
        }
      }
    }
  }

  return ctx->conn!=0;
}

static unsigned int
UnitArrayCount(AlistarUnitArray *Array)
{
  return sb_count(Array->Array);
}

static void
UnitArrayFree(AlistarUnitArray *Array)
{
  sb_free(Array->Array);
  ZeroMemory(Array,sizeof(*Array));
}

static Unit *
UnitArrayAdd(AlistarUnitArray *Array)
{
  Unit *res;
  res=sb_add(Array->Array,1);
  Array->CountMin++;

  return res;
}

static int
AlistarTick(AlistarContext *ctx)
{
  if((ctx->ready_for_obs) && (!ctx->requested_obs))
  { ctx->requested_obs=1;
    RequestObservation(ctx,TRUE,ctx->Game.loop);
  }

  Response response;

  // Todo: handle all events in the queue ...
  if(AliDequeueEventOfTypeAndNotifyAll(&ctx->conn_queue,&response,RESPONSE_TAG_OBSERVATION))
  {
    UnitArrayFree(&ctx->Game.CommandCenters);
    UnitArrayFree(&ctx->Game.SupplyDepots);
    UnitArrayFree(&ctx->Game.Workers);
    UnitArrayFree(&ctx->Game.WorkersIdling);
    UnitArrayFree(&ctx->Game.WorkersHarvesting);
    UnitArrayFree(&ctx->Game.WorkersBuilding);
    UnitArrayFree(&ctx->Game.WorkersRepairing);

    ctx->Game.WorkersQueued=0;
    ctx->Game.SupplyDepotsQueued=0;

    ctx->requested_obs=0;

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
  }

  return ctx->last_response.Type==RESPONSE_TAG_OBSERVATION;
}


// Todo: this is something that a backend should implement
static int
FindStarcraftExecutableAndDllsDirectory(const char *inst, int arch64, char *exec, char *dlls)
{ if(!IsFileNameReal(inst))
  { TRACE_F("invalid installation directory, %s, not found!", Widen(-1,inst));
    return 0;
  }

  sprintf_s(exec,MAX_PATH,"%s\\versions\\Base89165\\SC2%s.exe",inst,(arch64?"_x64":""));
  sprintf_s(dlls,MAX_PATH,"%s\\Support%s", inst,(arch64?"64":""));

  if(!IsFileNameReal(exec))
  { TRACE_F("Executable not found, %s", Widen(-1,exec));
    return 0;
  }
  if(!IsFileNameReal(dlls))
  { TRACE_F("Support directory not found, %s", Widen(-1,dlls));
    return 0;
  }

  return 1;
}

static ZenSystemProcess
LaunchStarcraftProcess(const char *inst, int arch64, const char *addr, int port)
{
  char exec[MAX_PATH];
  char dlls[MAX_PATH];
  char cmdl[MAX_PATH];

  // Todo: probably cache this ...
  int res;
  res=FindStarcraftExecutableAndDllsDirectory(inst,arch64,exec,dlls);

  // Note: if you have a double monitor setup this could be useful,
  // however, if you have a faulty GPU card that can barely run
  // two monitors at once, this could be useless ...
  if(!res) return {};

  sprintf_s(cmdl,sizeof(cmdl)," -listen %s -port %i -displayMode 0 -windowx 1921 -windowy 0",addr,port);

  //"-windowwidth"
  //"-windowheight"

  ZenSystemProcess proc;
  res=ZenLaunchSystemProcess(&proc,exec,dlls,cmdl);

  return proc;
}



#endif