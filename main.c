/*
**
** -+- AliStar -+-
**
** Copyright(C) Dayan Rodriguez, 2022, All rights reserved.
*/
#define _CRT_SECURE_NO_WARNINGS
#define _RX_STANDALONE
#include "vendor\rx\rx.c"
#include "vendor\ss\ss.c"

#include "alistar.h"
#include "alistar.c"

static context_t context;

void socket_closed_callback(const mg_connection* conn, void *user)
{
  cctracewar("connection ended");
}

int socket_received_callback(mg_connection* conn, int flags, char *data, size_t size, void *user)
{
  // Todo:
  acquire_mutex(context.thread.mutex);
  memcpy(context.thread.memory,data,size);

  ssread_t read;
  ssread_apportion(&read,size,context.thread.memory);

  ssresponse_t *response=ccmalloc_T(ssresponse_t);
  ParseResponse(&read,response);

  if(!response->Type)
  { if(response->Error)
      cctraceerr("%.*s",response->Error[-1],response->Error);
    else
      cctraceerr("unknown error",0);
  }
  release_mutex(context.thread.mutex);

  push_thread_message(&context.thread,response->Type,response);
  return 1;
}

void convert_texture(rxtexture_t *texture, int w, int h, int b, unsigned char *m)
{
  if(w!=0&&h!=0&&b!=0&&m!=0)
  {
    if(!texture->resource)
    {
      *texture=rxcreate_texture(w,h,rxRGBA8888);
    }

    rxborrowed_t r=rxborrow_texture(*texture);
    unsigned char *d=r.memory;
    for(int i=0;i<w*h;++i)
    { d[i*4+0]=m[i*3+2];
      d[i*4+1]=m[i*3+1];
      d[i*4+2]=m[i*3+0];
      d[i*4+3]=0xff;
    }
    rxreturn(r);
  }
}

INT32 WINAPI
WinMain(HINSTANCE inst, HINSTANCE prev_inst, PSTR cmdl, INT cmds)
{
  (void)inst;
  (void)prev_inst;
  (void)cmdl;
  (void)cmds;

  rxinit(L"Alistar - Command Center");
  context.thread.mutex=create_mutex();
  context.thread.memory=ccmalloc(1024*1024*4);

  const char *addr="127.0.0.1";
  int port=5679;
  const char *conn_options[] =
  { "request_timeout_ms", "100000",
    "websocket_timeout_ms", "100000",
    "num_threads", "1",
    "tcp_nodelay", "1", 0,
  };

  process_t sc;
  launch_starcraft_process(&sc,"G:\\StarCraft II",FALSE,addr,port);

  mg_callbacks callbacks;
  ZeroMemory(&callbacks,sizeof(callbacks));
  mg_start(&callbacks,NULL,conn_options);
  char errbuf[0x100];
  ZeroMemory(errbuf,sizeof(errbuf));
  context.conn=0;
  for(int tries=0;tries<7;++tries)
  { context.conn=mg_connect_websocket_client(addr,port,FALSE,errbuf,sizeof(errbuf),
      "/sc2api",NULL,socket_received_callback,socket_closed_callback,NULL);

    if(context.conn!=0)
      break;
  }

  ccassert(context.conn!=0 ||
    cctraceerr("connection failed"));

  ssresponse_t *response;
  response=request_available_maps(&context,ccfalse);
  response=request_create_game(&context,ccfalse);
  response=request_join_game_as_participant(&context,ccfalse,TERRAN,"dakota brown");
  response=request_game_info(&context,ccfalse);


  rxtexture_t logo=rxload_texture_file("media\\sclogo.png");

  rxtexture_t texture_mm=(rxtexture_t){0};
  rxtexture_t texture_mp=(rxtexture_t){0};
  for(;;)
  { response=request_observation(&context,0,0,cctrue);

    if(response)
    { { ImageData image_mm=response->Observation.RenderMinimap;
        convert_texture(&texture_mm,image_mm.size.x,image_mm.size.y,image_mm.bits_per_pixel,image_mm.bytes);
      }
      { ImageData image_mp=response->Observation.RenderMap;
        convert_texture(&texture_mp,image_mp.size.x,image_mp.size.y,image_mp.bits_per_pixel,image_mp.bytes);
      }
    }

    rxdraw_texture(logo,rx.linear_sampler,
      rx.center_x-(logo.size_x>>1),rx.center_y-(logo.size_y>>1),logo.size_x,logo.size_y);

    if(texture_mm.resource)
    { rxdraw_texture(texture_mp,rx.linear_sampler,0,0,512,512);
      rxdraw_texture(texture_mm,rx.linear_sampler,0,0,256,256);
    }

    rxtick();
  }
}
