// Copyright(C) 2022-2023 Dayan Rodriguez, All rights reserved.
#ifndef XVAL_C
#define XVAL_C

#ifndef ali_malloc
# define ali_malloc(size) malloc(size)
#endif
#ifndef ali_realloc
# define ali_realloc(memory,size) realloc(memory,size)
#endif
#ifndef ali_calloc
# define ali_calloc(size,count) calloc(size,count)
#endif
#ifndef ali_free
# define ali_free(memory) free(memory)
#endif

// Todo: support for signed integer types
typedef enum xtype
{ ali_typeless = 0,
  ali_int32_type,
  ali_int64_type,
  ali_flo32_type,
  ali_flo64_type,
  ali_var_type,
  ali_msg_type,
  ali_str_type,
  ali_len_typeless,
  ali_fix32_typeless,
  ali_fix64_typeless,
} xtype;
typedef struct xblock
{ size_t  len;
  void  * mem;
} xblock;
typedef struct xlabel
{ xtype bit;
  int   tag;
} xlabel;
typedef struct xstack
{ size_t     len;
  uint8_t  * loc;
  uint8_t  * cur;
  xlabel     lbl;
  unsigned   has: 1;
} xstack;
typedef struct xvalue
{ xtype          bit; // Note: must be first field
  unsigned int   tag;

  unsigned int   sze;
  unsigned int   enc;

  xvalue        *val;
  uint64_t       num;
  double         flo;

  unsigned int   len;
  void          *mem;
} xvalue;
// TODO(RJ): PADDING ...
typedef struct zslabel
{ unsigned int len;
  char         mem[4];
} zslabel;
typedef struct zsblock
{ zsblock *nex;
  char     mem[8];
} zsblock;
typedef struct zsarena
{ unsigned int rem;
  unsigned int len;
  zsblock     *lis;
} zsarena;
typedef struct xstate
{ xstack   stc_[0x10];
  xstack  *stc;
  zsarena  str;
  int      lvl;
  unsigned cue:1;
} xstate;

static unsigned int
zslen(const char *str)
{
  return str? ((unsigned int *)str)[-1] :0;
}

static char *
zsmov(void *mem,size_t len,const void *str)
{ zslabel *lbl;
  lbl=(zslabel *)mem;

  lbl->len=(uint32_t)len;// TODO(RJ):

  memmove(lbl->mem,str,len);
  lbl->mem[len]=0;

  return lbl->mem;
}

static void
zsdel(zsarena *mem)
{
  zsblock *itr,*nex;
  for(itr=mem->lis;itr;itr=nex)
  { nex=itr->nex;
    mg_free(itr);
  }
}

static char *
zsnew(zsarena *mem, size_t len, const void *str)
{
#ifndef STRING_BLOCK_SIZE_MIN
# define STRING_BLOCK_SIZE_MIN 0x200
#endif
#ifndef STRING_BLOCK_SIZE_MAX
# define STRING_BLOCK_SIZE_MAX 0xffff
#endif

  size_t enc;
  enc=len+sizeof(zslabel);

  if (enc>mem->rem)
  { // Note: there's not enough space in the first buffer. The first buffer has
    // the largest amount of free space, if any. And ideally, it should be the only
    // buffer with free space ...
    unsigned int sze;
    sze=mem->len;
    sze=STRING_BLOCK_SIZE_MIN<<(sze>>1);
    if (sze>STRING_BLOCK_SIZE_MAX)
    { ++mem->len;
    }

    zsblock *blc;
    if (enc>sze)
    { // Note: the length of the string is larger than the buffer size ...
      blc=(zsblock *)mg_malloc(sizeof(*blc)-sizeof(blc->mem)+enc);


      if (mem->lis)
      { // Since this block is full already, put it after the first one, if there is a first one,
        // otherwise, this becomes the first block in the else case ...
        blc->nex=mem->lis->nex;
        mem->lis->nex=blc;
      } else
      {
        blc->nex=0;
        mem->rem=0;
        mem->lis=blc;
      }

      char *res;
      res=zsmov(blc->mem,len,str);

      return res;
    } else
    { // Note: We allocated a buffer that was bigger than the requested string
      // and thus we have some remaining space ...
      // Note: The remaining field is set to the size, this is because the length of the
      // payload will be subtracted from it afterwards ...
      blc=(zsblock *)mg_malloc(sizeof(*blc)-sizeof(blc->mem)+sze);

      size_t rem;
      rem=sze-enc;

      if(rem>mem->rem)
      { // Note: this buffer has more free space than the previous buffer, set it as first ...
        blc->nex=mem->lis;
        mem->lis=blc;
      } else
      { // Note: this buffer isn't as big as the first one but we won't check
        // the subsequent buffers since we're going for speed, and on the
        // following allocation ewe only check the first buffer anyways ...
        //
        blc->nex=mem->lis->nex;
        mem->lis->nex=blc;
      }

      mem->rem=(uint32_t)rem; // TODO(RJ):

      // Note: we use the higher addresses of the buffer first ...
      char *res;
      res=blc->mem+rem;
      res=zsmov(res,len,str);

      return res;
    }
  } else
  {
    // Note: we use the higher addresses of the buffer first ...
    char *res;
    res=mem->lis->mem+mem->rem-enc;
    mem->rem-=(uint32_t)enc; // TODO(RJ)

    res=zsmov(res,len,str);
    return res;
  }
}

static int
ReturnDecodeError(const wchar_t *msg)
{ TRACE_F("%s",msg);
  Assert(!"error");
  return 0;
}

static int // NOTE(COMPILER): FORCEINLINE(ALWAYS)
VarintSize(uint64_t val)
{ int len;
  for(len=1;val>=0x80;++len,val>>=7);
  return len;
}

static int
PeekVarint(unsigned char *cur, uint64_t *res)
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

static unsigned char *
WriteFloat32(unsigned char *cur, float val)
{
  *(float *)cur=val;
  cur+=4;
  return cur;
}

static unsigned char *
WriteFloat64(unsigned char *cur, double val)
{
  *(double *)cur=val;
  cur+=4;
  return cur;
}

static unsigned char *
WriteVarint_(unsigned char *cur, uint64_t val)
{ for(;val>=0x80;val>>=7)
  { *cur++=(unsigned char)(val|0x80);
  }
  *cur++=(unsigned char)(val);
  return cur;
}

static unsigned char *
WriteVarint(unsigned char *cur, uint64_t val)
{ unsigned char *res;
  res=WriteVarint_(cur,val);

#ifdef _DEBUG
  int32_t len;
  uint64_t tst;
  len=PeekVarint(cur,&tst);
  Assert(tst==val);
  Assert(len==(res-cur));
#endif

  return res;
}

static int
PeekSignedVarint(unsigned char *cur, int64_t *val)
{ return PeekVarint(cur,(uint64_t *)val);
}

static float
PeekFloat32(unsigned char *cur)
{ return*(float *)cur;
}

static double
PeekFloat64(unsigned char *cur)
{ return*(double *)cur;
}

// TODO(RJ):
// THIS SHOULD TAKE THE LABEL
static unsigned int // NOTE(COMPILER): INLINE(ALWAYS)
EncodeLabel(int bit, int tag)
{ unsigned int raw;
  raw=((tag<<3)|(bit));
  return raw;
}

static xlabel
DecodeLabel(size_t raw)
{ xlabel label;

  int fmt,tag;
  fmt=(uint32_t)(raw&0b0111);
  tag=(uint32_t)(raw>>3);

  xtype bit;
  if((fmt==0)) bit=ali_var_type;        else
  if((fmt==1)) bit=ali_fix64_typeless;  else
  if((fmt==2)) bit=ali_len_typeless;    else
  if((fmt==5)) bit=ali_fix32_typeless;  else bit=ali_typeless;

  label.bit=bit;
  label.tag=tag;
  return label;
}

static xblock // NOTE(COMPILER): INLINE(ALWAYS)
NewBlock(uint32_t sze)
{ xblock val;
  val.len=sze;
  val.mem=(unsigned char*)mg_malloc(sze); // TODO(RJ):
  return val;
}

static void // NOTE(COMPILER): INLINE(ALWAYS)
DelBlock(xblock *val)
{ mg_free(val->mem);
  val->len=0;
  val->mem=0;
}

static xstack * // NOTE(COMPILER): FORCEINLINE(ALWAYS)
GetStack(xstate *read)
{ return read->stc;
}

static unsigned char * // NOTE(COMPILER): FORCEINLINE(ALWAYS)
AdvanceCursor(xstack *stc, size_t val)
{ unsigned char *res;
  res=stc->cur;
  stc->cur+= (uint32_t) val; // TODO(RJ):
  return res;
}

static uint32_t // NOTE(COMPILER): FORCEINLINE(ALWAYS)
GetUnreadStackSpace(xstack *stc)
{ uint32_t usd,len;
  usd=(uint32_t)(stc->cur-stc->loc);
  len=(uint32_t)(stc->len);
  return len-usd;
}

static int // NOTE(COMPILER): FORCEINLINE(ALWAYS)
StackCursorInBlock(xstack *stc)
{ unsigned char *min,*max;
  min=stc->loc;
  max=stc->loc+stc->len;
  return (stc->cur>=min)&&(stc->cur<max);
}

static int // NOTE(COMPILER): FORCEINLINE(ALWAYS)
StackCursorOnePastBlock(xstack *stc)
{ unsigned char *max;
  max=stc->loc+stc->len;
  return (stc->cur==max);
}

static int
StackInBlockOf(xstack *stc, xstack *chl)
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
GobbleSignedVarint(xstack *stc)
{
  int64_t val;

  int len;
  len=PeekSignedVarint(stc->cur,&val);

  AdvanceCursor(stc,len);

  return val;
}

static uint64_t
GobbleVarint(xstack *stc)
{
  uint64_t val;

  int len;
  len=PeekVarint(stc->cur,&val);

  AdvanceCursor(stc,len);

  return val;
}

static float
GobbleFloat32(xstack *stc)
{ float val;
  val=PeekFloat32(stc->cur);

  AdvanceCursor(stc,4);
  return val;
}

static double
GobbleFloat64(xstack *stc)
{ double val;
  val=PeekFloat64(stc->cur);

  AdvanceCursor(stc,8);
  return val;
}

static int
GobbleMemory(xstack *stc, unsigned int len, void *mem)
{ int max;
  max=GetUnreadStackSpace(stc);
  len=len<max?len:max;
  memcpy(mem,stc->cur,len);
  stc->cur+=len;
  return len;
}

// ...

static int
InternalPushStack(xstate *read, size_t len, unsigned char *loc)
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
InternalPullStack(xstate *read, xstack *com)
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
static xvalue *
AddValueArray(xvalue *dst, int len, xvalue *arr)
{
  xvalue *res;
  res=sb_add(dst->val,len);
  memcpy(res,arr,sizeof(*arr)*len);

  return res;
}

static xvalue * // NOTE(COMPILER): FORCEINLINE(ALWAYS)
AddValue(xvalue *dst, int tag, xvalue add)
{ // TODO(RJ):
  add.tag=tag;

  return AddValueArray(dst,1,&add);
}

#define AddBoolValue(dst,tag,bol) AddVarint32Value(dst,tag,bol)
#define AddEnumValue(dst,tag,enm) AddVarint32Value(dst,tag,enm)

static xvalue *
AddVarint64Value(xvalue *dst, int tag, uint64_t num)
{ xvalue val={ali_var_type};
  val.tag=tag;
  val.num=num;
  return AddValue(dst,tag,val);
}
static xvalue *
AddVarint32Value(xvalue *dst,int tag, uint32_t num)
{ return AddVarint64Value(dst,tag,num);
}
static xvalue *
AddFloat32Value(xvalue *dst,int tag, float num)
{ xvalue val={ali_flo32_type};
  val.tag=tag;
  val.flo=num;
  return AddValue(dst,tag,val);
}
static xvalue *
AddStringValue(xvalue *dst,int tag,const char *str)
{ xvalue val={ali_str_type};
  val.tag=tag;
  val.len=(uint32_t)strlen(str);
  val.mem=(void *)str;
  return AddValue(dst,tag,val);
}
static void
DelValue(xvalue *tar)
{ switch(tar->bit)
  { case ali_msg_type:
    { tar->enc=tar->sze=0;
      xvalue *itr;
      for(itr=tar->val;itr<tar->val+sb_count(tar->val);++itr)
      { DelValue(itr);
      }
      sb_free(tar->val);
    } break;
    case ali_str_type: // Todo: handle when memory is owned
    { tar->enc=tar->sze=0;
    } break;
    default:
    { tar->enc=tar->sze=0;
    } break;
  }
}
static void
ReadyValueForSerialization(xvalue *tar, unsigned inc)
{ switch(tar->bit)
  { case ali_msg_type:
    { tar->enc=tar->sze=0;
      xvalue *itr;
      for(itr=tar->val;itr<tar->val+sb_count(tar->val);++itr)
      { ReadyValueForSerialization(itr,1);
        // Note: here we include the size of the encoded child into the regular size of the parent.
        tar->sze+=itr->enc;
        tar->enc+=itr->enc;
      }
      if(inc)
      { tar->enc+=VarintSize(EncodeLabel(2,tar->tag));
        tar->enc+=VarintSize(tar->sze);
      }
    } break;
    case ali_str_type:
    { tar->enc=tar->sze=tar->len;
      if(inc)
      { tar->enc+=VarintSize(EncodeLabel(2,tar->tag));
        tar->enc+=VarintSize(tar->sze);
      }
    } break;
    case ali_fix32_typeless: case ali_flo32_type: case ali_int32_type:
    { tar->enc=tar->sze=4;
      if(inc)
      { tar->enc+=VarintSize(EncodeLabel(5,tar->tag));
      }
    } break;
    case ali_fix64_typeless: case ali_flo64_type: case ali_int64_type:
    { tar->enc=tar->sze=8;
      if(inc)
      { tar->enc+=VarintSize(EncodeLabel(1,tar->tag));
      }
    } break;
    case ali_var_type:
    { tar->enc=tar->sze=VarintSize(tar->num);
      if(inc)
      { tar->enc+=VarintSize(EncodeLabel(1,tar->tag));
      }
    } break;
    default:
    { Assert(!"error");
    } break;
  }
}

static void
SerializeValueInternal(xblock *mem, unsigned char **cur, xvalue *tar, int inc)
{ switch(tar->bit)
  { case ali_msg_type:
    { if(inc)
      { *cur=WriteVarint(*cur,EncodeLabel(2,tar->tag));
        *cur=WriteVarint(*cur,tar->sze);
      }
      xvalue *itr;
      for(itr=tar->val;itr<tar->val+sb_count(tar->val);++itr)
      { SerializeValueInternal(mem,cur,itr,1);
      }
    } break;
    case ali_str_type:
    { if(inc)
      { *cur=WriteVarint(*cur,EncodeLabel(2,tar->tag));
        *cur=WriteVarint(*cur,tar->sze);
      }
      memcpy(*cur,tar->mem,tar->sze);
      *cur+=tar->sze;
    } break;
    case ali_fix64_typeless: case ali_flo64_type: case ali_int64_type:
    { if(inc)
      { *cur=WriteVarint(*cur,EncodeLabel(1,tar->tag));
      }
      *cur=WriteFloat64(*cur,(double)tar->flo);
    } break;
    case ali_fix32_typeless: case ali_flo32_type: case ali_int32_type:
    { if(inc)
      { *cur=WriteVarint(*cur,EncodeLabel(5,tar->tag));
      }
      *cur=WriteFloat32(*cur,(float)tar->flo);
    } break;
    case ali_var_type:
    { if(inc)
      { *cur=WriteVarint(*cur,EncodeLabel(0,tar->tag));
      }
      *cur=WriteVarint(*cur,tar->num);
    } break;
    default:
    { Assert(!"error");
    } break;
  }
}

static xblock
SerializeValue(xvalue *tar)
{ ReadyValueForSerialization(tar,0);

  xblock mem;
  mem=NewBlock(tar->enc);

  unsigned char *cur;
  cur=(unsigned char*)mem.mem;

  SerializeValueInternal(&mem,&cur,tar,0);
  return mem;
}

static xlabel
InternalConsumeLabel(xstate *read)
{ Assert(read->stc->has);

  xlabel label;
  label=read->stc->lbl;

  read->stc->has=0;
  read->stc->lbl={};
  return label;
}

static void
InternalGetUnknownValue(xstate *read)
{
  xstack *block;
  block=GetStack(read);

  xlabel label;
  label=InternalConsumeLabel(read);

  int32_t isl;
  isl=(label.bit==ali_len_typeless) || (label.bit==ali_msg_type) || (label.bit==ali_str_type);

  uint64_t val;
  if((label.bit==ali_var_type) || isl)
  { val=GobbleVarint(block);
    if(isl) AdvanceCursor(block,val);
  } else
  if((label.bit==ali_fix32_typeless))
  { AdvanceCursor(block,4);
  } else
  if((label.bit==ali_fix64_typeless))
  { AdvanceCursor(block,8);
  } else
  {
    Assert(!"error");
  }
}

static int
GetSignedVarintValue64(xstate *read, int64_t *value)
{
  xstack *block;
  block=GetStack(read);

  xlabel label;
  label=InternalConsumeLabel(read);

  if((label.bit!=ali_var_type)) return ReturnDecodeError(L"invalid call, tag is too different from type");

  int64_t val;
  val=GobbleSignedVarint(block);

  if(value)*value=val;
  return 1;
}

static int
GetVarint64Value(xstate *read, uint64_t *value)
{
  xstack *block;
  block=GetStack(read);

  xlabel label;
  label=InternalConsumeLabel(read);

  if((label.bit!=ali_var_type)) return ReturnDecodeError(L"invalid call, tag is too different from type");

  uint64_t val;
  val=GobbleVarint(block);

  if(value)*value=val;
  return 1;
}

static int
GetVarint32Value(xstate *read, unsigned int *val32)
{
  uint64_t val64;

  int res;
  res=GetVarint64Value(read,&val64);

  if(val32) *val32 = (uint32_t) val64;

  return res;
}

static int
GetVarintValue(xstate *read, unsigned int *val)
{
  return GetVarint32Value(read,val);
}

// TODO(RJ): OVERLOAD, REMOVE!
static int
GetVarintValue(xstate *read, int *val)
{
  return GetVarint32Value(read,(unsigned int *)val);
}

static int
GetFloat32Value(xstate *read, float *val)
{
  xstack *block;
  block=GetStack(read);

  xlabel label;
  label=InternalConsumeLabel(read);

  if((label.bit!=ali_flo32_type)&&(label.bit!=ali_fix32_typeless))
    return ReturnDecodeError(L"invalid call, tag is too different from type");

  float res;
  res=GobbleFloat32(block);

  if(val) *val=res;
  return 1;
}

static int
GetFloat64Value(xstate *read, double *val)
{
  xstack *block;
  block=GetStack(read);

  xlabel label;
  label=InternalConsumeLabel(read);

  if((label.bit!=ali_flo64_type)&&(label.bit!=ali_fix64_typeless))
    return ReturnDecodeError(L"invalid call, tag is too different from type");

  double res;
  res=GobbleFloat64(block);

  if(val) *val=res;
  return 1;
}

static int
GetEnumValue(xstate *read, void *value)
{ return GetVarintValue(read, (unsigned int *) value); // TODO(RJ):
}

static int
GetStringValue(xstate *read, char **val)
{
  xstack *block;
  block=GetStack(read);

  xlabel label;
  label=InternalConsumeLabel(read);

  if((label.bit!=ali_len_typeless) && (label.bit!=ali_str_type))
  {
    return ReturnDecodeError(L"invalid call, tag is too different from type");
  }

  size_t len;
  len=GobbleVarint(block);

  void *str;
  str=AdvanceCursor(block,len);

  *val=zsnew(&read->str,len,str);

  return 1;
}

static int
GetBytesValue(xstate *read, unsigned char **val)
{
  xstack *block;
  block=GetStack(read);

  xlabel label;
  label=InternalConsumeLabel(read);

  if((label.bit!=ali_len_typeless) && (label.bit!=ali_str_type))
  {
    return ReturnDecodeError(L"invalid call, tag is too different from type");
  }

  size_t len;
  len=GobbleVarint(block);

  void *str;
  str=AdvanceCursor(block,len);

  *val=(unsigned char*)zsnew(&read->str,len,str);
  return 1;
}

static int
NextField(xstate *read)
{
  if(read->stc->has)
  { // Note: this means we have a label, in other words, the user had already called next_field
    // but the value wasn't consumed ...

    // Note: check if we have a cue, this tells us to push a message block onto the stack ..
    if(read->cue)
    { read->cue=0;

      xlabel label;
      label=InternalConsumeLabel(read);

      if((label.bit!=ali_len_typeless) && (label.bit!=ali_msg_type))
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

    xlabel label;
    label=DecodeLabel(raw);

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
MessageCue(xstate *read)
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
GetTag(xstate *read)
{
  Assert(read->stc->has);
  return read->stc->lbl.tag;
}

#define ForMessageField(read) MessageCue((read)); while(NextField((read))) switch(GetTag((read)))


static void
Apportion(xstate *read, size_t len, const void *cur)
{ InternalPushStack(read,len,(unsigned char *)cur);
}



#endif