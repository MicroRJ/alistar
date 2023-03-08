// Copyright(C) 2022-2023 Dayan Rodriguez, All rights reserved.
#ifndef _SS_DECODER_ENCODER
#define _SS_DECODER_ENCODER

typedef unsigned char *sscursor_t;

typedef enum ssclass_t
{ ssclass_kTYPELESS=0,
  ssclass_kTYPELESS32,
  ssclass_kTYPELESS64,
  ssclass_kLENGTH,
  ssclass_kRECORD,
  ssclass_kSTRING,
  ssclass_kSIGNED,
  ssclass_kUNSIGNED,
  ssclass_kUNSIGNED32,
  ssclass_kUNSIGNED64,
  ssclass_kSIGNED32,
  ssclass_kSIGNED64,
  ssclass_kFLOAT32,
  ssclass_kFLOAT64,
} ssclass_t;

typedef struct ssblock_t ssblock_t;
typedef struct ssblock_t
{ size_t  len;
  void  * mem;
} ssblock_t;

typedef struct sslabel_t sslabel_t;
typedef struct sslabel_t
{ ssclass_t bit;
  int       tag;
} sslabel_t;

typedef struct ssstack_t ssstack_t;
typedef struct ssstack_t
{ size_t           len;
  unsigned char  * loc;
  unsigned char  * cur;
  sslabel_t        lbl;
  unsigned         has: 1;
} ssstack_t;

typedef struct ssvalue_t ssvalue_t;
typedef struct ssvalue_t
{ ssclass_t      bit; // Note: must be first field
  unsigned int   tag;

  unsigned int   sze;
  unsigned int   enc;

  ssvalue_t     *val;
  uint64_t       num;
  double         flo;

  unsigned int   len;
  void          *mem;
} ssvalue_t;

typedef struct ssread_t ssread_t;
typedef struct ssread_t
{ ssstack_t    stc_[0x10];
  ssstack_t  * stc;
  int          lvl;
  unsigned     cue:1;
  ali_arena    arena;
} ssread_t;

static int
ReturnDecodeError(const wchar_t *msg)
{ TRACE_F("%s",msg);
  Assert(!"error");
  return 0;
}

static int
ssvarintlength(uint64_t val)
{ int len;
  for(len=1;val>=0x80;++len,val>>=7);
  return len;
}

static int
ssload_varint(sscursor_t cur, uint64_t *res)
{ int len,chr;
  uint64_t val,bit;
  len=0;
  val=0;
  do
  { chr=*cur++;
    bit=chr&0x7f;
    val|=bit<<len*7;
    len++;
  } while(chr&0x80);
  *res=val;
  return(len);
}

static sscursor_t
ssstore_varint_(sscursor_t cur, uint64_t val)
{ for(;val>=0x80;val>>=7)
    *cur++=(unsigned char)(val|0x80);

  *cur++=(unsigned char)(val);
  return cur;
}

static sscursor_t
ssstore_float32(sscursor_t cur, float val)
{
  *(float *)cur=val;
  return cur+4;
}

static sscursor_t
ssstore_float64(sscursor_t cur, double val)
{
  *(double *)cur=val;
  return cur+8;
}

static float
ssload_float32(sscursor_t cur)
{
  return *(float *)cur;
}

static double
ssload_float64(sscursor_t cur)
{
  return *(double *)cur;
}

static sscursor_t
ssstore_varint(sscursor_t cur, uint64_t val)
{ unsigned char *res=ssstore_varint_(cur,val);

#ifdef _DEBUG
  int32_t len;
  uint64_t tst;
  len=ssload_varint(cur,&tst);
  Assert(tst==val);
  Assert(len==(res-cur));
#endif

  return res;
}

static int
PeekSignedVarint(unsigned char *cur, int64_t *val)
{ return ssload_varint(cur,(uint64_t *)val);
}



// TODO(RJ): THIS SHOULD TAKE THE LABEL
static unsigned int
ssencode_label(int bit, int tag)
{ unsigned int raw=((tag<<3)|(bit));
  return raw;
}

static sslabel_t
ssdecode_label(size_t raw)
{ sslabel_t label;

  int fmt,tag;
  fmt=(uint32_t)(raw&0b0111);
  tag=(uint32_t)(raw>>3);

  ssclass_t bit=ssclass_kTYPELESS;
  if((fmt==0)) bit=ssclass_kUNSIGNED;   else
  if((fmt==1)) bit=ssclass_kTYPELESS64; else
  if((fmt==2)) bit=ssclass_kLENGTH;     else
  if((fmt==5)) bit=ssclass_kTYPELESS32;

  label.bit=bit;
  label.tag=tag;
  return label;
}

static ssblock_t
NewBlock(uint32_t sze)
{ ssblock_t val;
  val.len=sze;
  val.mem=(unsigned char*)mg_malloc(sze); // TODO(RJ):
  return val;
}

static void
DelBlock(ssblock_t *val)
{ mg_free(val->mem);
  val->len=0;
  val->mem=0;
}

static ssstack_t * // NOTE(COMPILER): FORCEINLINE(ALWAYS)
GetStack(ssread_t *read)
{ return read->stc;
}

static unsigned char * // NOTE(COMPILER): FORCEINLINE(ALWAYS)
AdvanceCursor(ssstack_t *stc, size_t val)
{ unsigned char *res;
  res=stc->cur;
  stc->cur+= (uint32_t) val; // TODO(RJ):
  return res;
}

static uint32_t // NOTE(COMPILER): FORCEINLINE(ALWAYS)
GetUnreadStackSpace(ssstack_t *stc)
{ uint32_t usd,len;
  usd=(uint32_t)(stc->cur-stc->loc);
  len=(uint32_t)(stc->len);
  return len-usd;
}

static int // NOTE(COMPILER): FORCEINLINE(ALWAYS)
StackCursorInBlock(ssstack_t *stc)
{ unsigned char *min,*max;
  min=stc->loc;
  max=stc->loc+stc->len;
  return (stc->cur>=min)&&(stc->cur<max);
}

static int // NOTE(COMPILER): FORCEINLINE(ALWAYS)
StackCursorOnePastBlock(ssstack_t *stc)
{ unsigned char *max;
  max=stc->loc+stc->len;
  return (stc->cur==max);
}

static int
StackInBlockOf(ssstack_t *stc, ssstack_t *chl)
{ unsigned char *min,*max;
  min=stc->loc;
  max=stc->loc+stc->len;
  unsigned char *cur,*loc;
  loc=chl->loc;
  cur=chl->cur;
  return (loc>=min) && (cur>=min) && (loc<=max) && (cur<=max);
}

// Gobble ...

static int64_t
GobbleSignedVarint(ssstack_t *stc)
{
  int64_t val;

  int len;
  len=PeekSignedVarint(stc->cur,&val);

  AdvanceCursor(stc,len);

  return val;
}

static uint64_t
GobbleVarint(ssstack_t *stc)
{
  uint64_t val;

  int len;
  len=ssload_varint(stc->cur,&val);

  AdvanceCursor(stc,len);

  return val;
}

static float
GobbleFloat32(ssstack_t *stc)
{ float val;
  val=ssload_float32(stc->cur);

  AdvanceCursor(stc,4);
  return val;
}

static double
GobbleFloat64(ssstack_t *stc)
{ double val;
  val=ssload_float64(stc->cur);

  AdvanceCursor(stc,8);
  return val;
}

static int
GobbleMemory(ssstack_t *stc, unsigned int len, void *mem)
{ int max;
  max=GetUnreadStackSpace(stc);
  len=len<max?len:max;
  memcpy(mem,stc->cur,len);
  stc->cur+=len;
  return len;
}

// ...

static int
InternalPushStack(ssread_t *read, size_t len, unsigned char *loc)
{ if(read->stc)
  { if(read->stc<read->stc_+ARRAYSIZE(read->stc_))
    { ++read->stc;
    } else return ReturnDecodeError(L"stack overflow, too make embedded messages");
  } else
  { read->stc=read->stc_;
  }

  read->stc->len=(unsigned int)len;// TODO(RJ):
  read->stc->loc=loc;
  read->stc->cur=loc;
  return 1;
}

static int
InternalPullStack(ssread_t *read, ssstack_t *com)
{ if(read->stc)
  { read->stc=read->stc-1;
    if(read->stc>=read->stc_)
    { if(StackInBlockOf(read->stc,com))
      { read->stc->cur=com->loc+com->len;
      } else
      { Assert(!"internal error, invalid block, cannot commit");
      }
    } else
    { read->stc=NULL;

    }
    return 1;
  } else return ReturnDecodeError(L"stack underflow, internal error, possibly corrupted data");
}

// TODO(RJ): REMOVE THIS
static ssvalue_t *
AddValueArray(ssvalue_t *dst, int len, ssvalue_t *arr)
{
  ssvalue_t *res;
  res=sb_add(dst->val,len);
  memcpy(res,arr,sizeof(*arr)*len);

  return res;
}

static ssvalue_t * // NOTE(COMPILER): FORCEINLINE(ALWAYS)
AddValue(ssvalue_t *dst, int tag, ssvalue_t add)
{ // TODO(RJ):
  add.tag=tag;

  return AddValueArray(dst,1,&add);
}

#define AddBoolValue(dst,tag,bol) AddVarint32Value(dst,tag,bol)
#define AddEnumValue(dst,tag,enm) AddVarint32Value(dst,tag,enm)

static ssvalue_t *
AddVarint64Value(ssvalue_t *dst, int tag, uint64_t num)
{ ssvalue_t val={ssclass_kUNSIGNED};
  val.tag=tag;
  val.num=num;
  return AddValue(dst,tag,val);
}
static ssvalue_t *
AddVarint32Value(ssvalue_t *dst,int tag, uint32_t num)
{ return AddVarint64Value(dst,tag,num);
}
static ssvalue_t *
AddFloat32Value(ssvalue_t *dst,int tag, float num)
{ ssvalue_t val={ssclass_kFLOAT32};
  val.tag=tag;
  val.flo=num;
  return AddValue(dst,tag,val);
}
static ssvalue_t *
AddStringValue(ssvalue_t *dst,int tag,const char *str)
{ ssvalue_t val={ssclass_kSTRING};
  val.tag=tag;
  val.len=(uint32_t)strlen(str);
  val.mem=(void *)str;
  return AddValue(dst,tag,val);
}
static void
DelValue(ssvalue_t *tar)
{ switch(tar->bit)
  { case ssclass_kRECORD:
    { tar->enc=tar->sze=0;
      ssvalue_t *itr;
      for(itr=tar->val;itr<tar->val+sb_count(tar->val);++itr)
      { DelValue(itr);
      }
      sb_free(tar->val);
    } break;
    case ssclass_kSTRING: // Todo: handle when memory is owned
    { tar->enc=tar->sze=0;
    } break;
    default:
    { tar->enc=tar->sze=0;
    } break;
  }
}
static void
ReadyValueForSerialization(ssvalue_t *tar, unsigned inc)
{ switch(tar->bit)
  { case ssclass_kRECORD:
    { tar->enc=tar->sze=0;
      ssvalue_t *itr;
      for(itr=tar->val;itr<tar->val+sb_count(tar->val);++itr)
      { ReadyValueForSerialization(itr,1);
        // Note: here we include the size of the encoded child into the regular size of the parent.
        tar->sze+=itr->enc;
        tar->enc+=itr->enc;
      }
      if(inc)
      { tar->enc+=ssvarintlength(ssencode_label(2,tar->tag));
        tar->enc+=ssvarintlength(tar->sze);
      }
    } break;
    case ssclass_kSTRING:
    { tar->enc=tar->sze=tar->len;
      if(inc)
      { tar->enc+=ssvarintlength(ssencode_label(2,tar->tag));
        tar->enc+=ssvarintlength(tar->sze);
      }
    } break;
    case ssclass_kTYPELESS32: case ssclass_kFLOAT32: case ssclass_kSIGNED32:
    { tar->enc=tar->sze=4;
      if(inc)
      { tar->enc+=ssvarintlength(ssencode_label(5,tar->tag));
      }
    } break;
    case ssclass_kTYPELESS64: case ssclass_kFLOAT64: case ssclass_kSIGNED64:
    { tar->enc=tar->sze=8;
      if(inc)
      { tar->enc+=ssvarintlength(ssencode_label(1,tar->tag));
      }
    } break;
    case ssclass_kUNSIGNED:
    { tar->enc=tar->sze=ssvarintlength(tar->num);
      if(inc)
      { tar->enc+=ssvarintlength(ssencode_label(1,tar->tag));
      }
    } break;
    default:
    { Assert(!"error");
    } break;
  }
}

static void
SerializeValueInternal(ssblock_t *mem, unsigned char **cur, ssvalue_t *tar, int inc)
{ switch(tar->bit)
  { case ssclass_kRECORD:
    { if(inc)
      { *cur=ssstore_varint(*cur,ssencode_label(2,tar->tag));
        *cur=ssstore_varint(*cur,tar->sze);
      }
      ssvalue_t *itr;
      for(itr=tar->val;itr<tar->val+sb_count(tar->val);++itr)
      { SerializeValueInternal(mem,cur,itr,1);
      }
    } break;
    case ssclass_kSTRING:
    { if(inc)
      { *cur=ssstore_varint(*cur,ssencode_label(2,tar->tag));
        *cur=ssstore_varint(*cur,tar->sze);
      }
      memcpy(*cur,tar->mem,tar->sze);
      *cur+=tar->sze;
    } break;
    case ssclass_kTYPELESS64: case ssclass_kFLOAT64: case ssclass_kSIGNED64:
    { if(inc)
      { *cur=ssstore_varint(*cur,ssencode_label(1,tar->tag));
      }
      *cur=ssstore_float64(*cur,(double)tar->flo);
    } break;
    case ssclass_kTYPELESS32: case ssclass_kFLOAT32: case ssclass_kSIGNED32:
    { if(inc)
      { *cur=ssstore_varint(*cur,ssencode_label(5,tar->tag));
      }
      *cur=ssstore_float32(*cur,(float)tar->flo);
    } break;
    case ssclass_kUNSIGNED:
    { if(inc)
      { *cur=ssstore_varint(*cur,ssencode_label(0,tar->tag));
      }
      *cur=ssstore_varint(*cur,tar->num);
    } break;
    default:
    { Assert(!"error");
    } break;
  }
}

static ssblock_t
SerializeValue(ssvalue_t *tar)
{ ReadyValueForSerialization(tar,0);

  ssblock_t mem;
  mem=NewBlock(tar->enc);

  unsigned char *cur;
  cur=(unsigned char*)mem.mem;

  SerializeValueInternal(&mem,&cur,tar,0);
  return mem;
}

static sslabel_t
InternalConsumeLabel(ssread_t *read)
{ Assert(read->stc->has);

  sslabel_t label;
  label=read->stc->lbl;

  read->stc->has=0;
  read->stc->lbl={};
  return label;
}

static void
InternalGetUnknownValue(ssread_t *read)
{
  ssstack_t *block;
  block=GetStack(read);

  sslabel_t label;
  label=InternalConsumeLabel(read);

  int32_t isl;
  isl=(label.bit==ssclass_kLENGTH) || (label.bit==ssclass_kRECORD) || (label.bit==ssclass_kSTRING);

  uint64_t val;
  if((label.bit==ssclass_kUNSIGNED) || isl)
  { val=GobbleVarint(block);
    if(isl) AdvanceCursor(block,val);
  } else
  if((label.bit==ssclass_kTYPELESS32))
  { AdvanceCursor(block,4);
  } else
  if((label.bit==ssclass_kTYPELESS64))
  { AdvanceCursor(block,8);
  } else
  {
    Assert(!"error");
  }
}

static int
GetSignedVarintValue64(ssread_t *read, int64_t *value)
{
  ssstack_t *block;
  block=GetStack(read);

  sslabel_t label;
  label=InternalConsumeLabel(read);

  if((label.bit!=ssclass_kUNSIGNED)) return ReturnDecodeError(L"invalid call, tag is too different from type");

  int64_t val;
  val=GobbleSignedVarint(block);

  if(value)*value=val;
  return 1;
}

static int
GetVarint64Value(ssread_t *read, uint64_t *value)
{
  ssstack_t *block;
  block=GetStack(read);

  sslabel_t label;
  label=InternalConsumeLabel(read);

  if((label.bit!=ssclass_kUNSIGNED)) return ReturnDecodeError(L"invalid call, tag is too different from type");

  uint64_t val;
  val=GobbleVarint(block);

  if(value)*value=val;
  return 1;
}

static int
GetVarint32Value(ssread_t *read, unsigned int *val32)
{
  uint64_t val64;

  int res;
  res=GetVarint64Value(read,&val64);

  if(val32) *val32 = (uint32_t) val64;

  return res;
}

// Todo: IMPLEMENT
static int
GetSignedVarint32Value(ssread_t *read, int *val32)
{
  return GetVarint32Value(read,(unsigned int *) val32);
}

static int
GetVarintValue(ssread_t *read, unsigned int *val)
{
  return GetVarint32Value(read,val);
}

// TODO(RJ): OVERLOAD, REMOVE!
static int
GetVarintValue(ssread_t *read, int *val)
{
  return GetVarint32Value(read,(unsigned int *)val);
}

static int
GetFloat32Value(ssread_t *read, float *val)
{
  ssstack_t *block;
  block=GetStack(read);

  sslabel_t label;
  label=InternalConsumeLabel(read);

  if((label.bit!=ssclass_kFLOAT32)&&(label.bit!=ssclass_kTYPELESS32))
    return ReturnDecodeError(L"invalid call, tag is too different from type");

  float res;
  res=GobbleFloat32(block);

  if(val) *val=res;
  return 1;
}

static int
GetFloat64Value(ssread_t *read, double *val)
{
  ssstack_t *block;
  block=GetStack(read);

  sslabel_t label;
  label=InternalConsumeLabel(read);

  if((label.bit!=ssclass_kFLOAT64)&&(label.bit!=ssclass_kTYPELESS64))
    return ReturnDecodeError(L"invalid call, tag is too different from type");

  double res;
  res=GobbleFloat64(block);

  if(val) *val=res;
  return 1;
}

static int
GetEnumValue(ssread_t *read, void *value)
{ return GetVarintValue(read, (unsigned int *) value); // TODO(RJ):
}

static int
GetStringValue(ssread_t *read, char **val)
{
  ssstack_t *block;
  block=GetStack(read);

  sslabel_t label;
  label=InternalConsumeLabel(read);

  if((label.bit!=ssclass_kLENGTH) && (label.bit!=ssclass_kSTRING))
  {
    return ReturnDecodeError(L"invalid call, tag is too different from type");
  }

  size_t len;
  len=GobbleVarint(block);

  void *str;
  str=AdvanceCursor(block,len);

  char *mem;
  mem=ali_arena_sadd_(&read->arena,len,len+1);

  memcpy(mem,str,len);
  mem[len]=0;

  *val=mem;

  return 1;
}

static int
GetStringValueNoCopy(ssread_t *read, char **val)
{
  ssstack_t *block;
  block=GetStack(read);

  sslabel_t label;
  label=InternalConsumeLabel(read);

  if((label.bit!=ssclass_kLENGTH) && (label.bit!=ssclass_kSTRING))
  {
    return ReturnDecodeError(L"invalid call, tag is too different from type");
  }

  size_t len;
  len=GobbleVarint(block);

  unsigned char *str;
  str=AdvanceCursor(block,len);

  *val=(char*)str;

  return 1;
}

static int
GetBytesValue(ssread_t *read, unsigned char **val)
{
  ssstack_t *block;
  block=GetStack(read);

  sslabel_t label;
  label=InternalConsumeLabel(read);

  if((label.bit!=ssclass_kLENGTH) && (label.bit!=ssclass_kSTRING))
  {
    return ReturnDecodeError(L"invalid call, tag is too different from type");
  }

  size_t len;
  len=GobbleVarint(block);

  void *str;
  str=AdvanceCursor(block,len);


  char *mem;
  mem=ali_arena_sadd_(&read->arena,len,len);
  memcpy(mem,str,len);

  *val=(uint8_t*)mem;

  return 1;
}

static int
GetBytesValueNoCopy(ssread_t *read, unsigned char **val)
{
  ssstack_t *block;
  block=GetStack(read);

  sslabel_t label;
  label=InternalConsumeLabel(read);

  if((label.bit!=ssclass_kLENGTH) && (label.bit!=ssclass_kSTRING))
  {
    return ReturnDecodeError(L"invalid call, tag is too different from type");
  }

  size_t len;
  len=GobbleVarint(block);

  unsigned char *str;
  str=AdvanceCursor(block,len);

  *val=str;
  return 1;
}

static int
NextField(ssread_t *read)
{
  if(read->stc->has)
  { // Note: this means we have a label, in other words, the user had already called next_field
    // but the value wasn't consumed ...

    // Note: check if we have a cue, this tells us to push a message block onto the stack ..
    if(read->cue)
    { read->cue=0;

      sslabel_t label;
      label=InternalConsumeLabel(read);

      if((label.bit!=ssclass_kLENGTH) && (label.bit!=ssclass_kRECORD))
      {
        return ReturnDecodeError(L"invalid call, tag is too different from type");
      }

      size_t len;
      len=GobbleVarint(read->stc);

      InternalPushStack(read,len,read->stc->cur);

    } else
    {
      // Note: otherwise, skip this value, the user does not care about it ...
      InternalGetUnknownValue(read);
    }
  } else
  {
    // Note: ...
  }

  if(StackCursorInBlock(read->stc))
  {
    size_t raw;
    raw=GobbleVarint(read->stc);

    sslabel_t label;
    label=ssdecode_label(raw);

    read->stc->lbl=label;
    read->stc->has=1;
    return 1;
  } else
  if(StackCursorOnePastBlock(read->stc))
  {
    InternalPullStack(read,read->stc);

    read->lvl--;

    return 0;
  } else
  { Assert(!"error, cursor out of bounds");
    return 0;
  }
}


static void
MessageCue(ssread_t *read)
{
  // Note: this could happen if the user manually specified a cue
  if(!read->cue)
  {
    // Note: this doesn't apply when we are at the very top level
    if(read->lvl)
    { read->cue=1;
    }
    read->lvl++;
  }
}
static int
GetTag(ssread_t *read)
{
  Assert(read->stc->has);
  return read->stc->lbl.tag;
}

#define ForMessageField(read) MessageCue((read)); while(NextField((read))) switch(GetTag((read)))


static void
Apportion(ssread_t *read, size_t len, const void *cur)
{ // ali_rst_(read);
  InternalPushStack(read,len,(unsigned char *)cur);
}



#endif