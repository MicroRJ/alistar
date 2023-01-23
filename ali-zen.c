// Copyright(C) 2022-2023 Dayan Rodriguez, All rights reserved.
#ifndef ALI_ZEN
#define ALI_ZEN

static int
BitsToBytes(unsigned char *bytes, int bits)
{ bytes[0]=((bits&0x80)?(0xff):(0));bytes[4]=((bits&0x08)?(0xff):(0));
  bytes[1]=((bits&0x40)?(0xff):(0));bytes[5]=((bits&0x04)?(0xff):(0));
  bytes[2]=((bits&0x20)?(0xff):(0));bytes[6]=((bits&0x02)?(0xff):(0));
  bytes[3]=((bits&0x10)?(0xff):(0));bytes[7]=((bits&0x01)?(0xff):(0));
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
  { tex=ZenUnboundTexture(DE_CPU_ACCESS,PIXEL_FORMAT_RGB8,data.size.x,data.size.y,data.bytes);
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
  } else
  if(data.bits_per_pixel==32)
  { tex=ZenUnboundTexture(DE_CPU_ACCESS,PIXEL_FORMAT_RGBA8888,data.size.x,data.size.y,data.bytes);
  }

  return tex;
}

static ZenTexture *
CopyImageDataToTexture(ZenCore *Core, ZenTexture *Texture, ImageData data)
{

  if((data.size.x<=0)||(data.size.y<=0)||(data.bytes==0)||(data.bits_per_pixel<=0)) return 0;

  ZenTexture tex;
  tex=AliConvertTexture(Core,data);

  if((!Texture) || (data.size.x != Texture->DimenX) || (data.size.y != Texture->DimenY))
  { TRACE_I("Create Texture");
    Texture=ZenCoreCreateTexture(Core,DE_GPU_READ|DE_CPU_WRITE,tex.Format,tex.DimenX,tex.DimenY,tex.Memory);
  } else
  { if(ZenCoreBorrowTextureChronicle(Core, Texture, ZEN_WRITE))
    { ZenCopyTexture(Texture, &tex);
      ZenCoreReturnTextureChronicle(Core, Texture);
    }
  }

  return Texture;
}

#endif