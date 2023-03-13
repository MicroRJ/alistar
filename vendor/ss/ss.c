/*
**
** -+- ss | Simple Serial | 2022 -+-
**
** Copyright(C) J. Dayan Rodriguez, 2022, All rights reserved.
**
** Supports Google's protocol buffers...
**
*/
#ifndef _SS_SIMPLE_SERIAL
#define _SS_SIMPLE_SERIAL

typedef enum sstype_t
{ sstype_kTYPELESS=0,
  sstype_kLENGTH,
  sstype_kSTRING,
  sstype_kRECORD,
  sstype_kTYPELESS32,
  sstype_kTYPELESS64,
  sstype_kSVARINT,
  sstype_kUVARINT,
  sstype_kBOOLEAN32,
  sstype_kUNSIGNED32,
  sstype_kUNSIGNED64,
  sstype_kSIGNED32,
  sstype_kSIGNED64,
  sstype_kREAL32,
  sstype_kREAL64,
} sstype_t;

int sstype_is_length_encoded(sstype_t);

typedef struct ssblock_t ssblock_t;
typedef struct ssblock_t
{ size_t  length;
  void  * memory;
} ssblock_t;

typedef struct ssstack_t ssstack_t;
typedef struct ssstack_t
{ union
  { ssblock_t    serial;
    struct
    {  size_t    length;
      ccu8_t  * memory;
    };
  };
  ccu8_t      * cursor;
} ssstack_t;

ccu32_t ssload_uvar32(ssstack_t *);
 cci32_t ssload_svar32(ssstack_t *);

ccu64_t ssload_uvar64(ssstack_t *);
cci64_t ssload_svar64(ssstack_t *);
ccf32_t ssload_real32(ssstack_t *);
ccf64_t ssload_real64(ssstack_t *);
ccu64_t ssload_uint64(ssstack_t *);
cci64_t ssload_sint64(ssstack_t *);
ccu32_t ssload_uint32(ssstack_t *);
cci32_t ssload_sint32(ssstack_t *);
ccu16_t ssload_uint16(ssstack_t *);
cci16_t ssload_sint16(ssstack_t *);
 ccu8_t  ssload_uint8(ssstack_t *);
 cci8_t  ssload_sint8(ssstack_t *);

void  ssstore_uvari(ssstack_t *, ccu64_t);
void  ssstore_svari(ssstack_t *,  cci64_t);

typedef struct sslabel_t sslabel_t;
typedef struct sslabel_t
{ sstype_t  type;
  int       name;
} sslabel_t;

typedef struct ssread_t ssread_t;
typedef struct ssread_t
{ ssstack_t stack[0x10];
  int       level;
  sslabel_t label;
  unsigned  cue:1;
  unsigned  has:1;
} ssread_t;

int ssread_uvar32(ssread_t *, ccu32_t *);
int ssread_svar32(ssread_t *,  cci32_t *);

int ssread_uvar64(ssread_t *, ccu64_t *);
int ssread_svar64(ssread_t *,  cci64_t *);

int   ssread_uvar(ssread_t *, ccu64_t *);
int   ssread_svar(ssread_t *,  cci64_t *);
int ssread_uint32(ssread_t *, ccu32_t *);
int ssread_sint32(ssread_t *,  cci32_t *);
int ssread_uint64(ssread_t *, ccu64_t *);
int ssread_sint64(ssread_t *,  cci64_t *);
int ssread_uint16(ssread_t *, ccu16_t *);
int ssread_sint16(ssread_t *,  cci16_t *);
int  ssread_uint8(ssread_t *,  ccu8_t *);
int  ssread_sint8(ssread_t *,   cci8_t *);
int ssread_real32(ssread_t *,    ccf32_t *);
int ssread_real64(ssread_t *,   ccf64_t *);

typedef struct ssvalue_t ssvalue_t;
typedef struct ssvalue_t
{
  sslabel_t       label;
  unsigned int    length;
  union
  { ccu64_t     uint64;
    ccf64_t       real64;
    void       * memory;
    ssvalue_t  * member;
  };
  const char    * string;
} ssvalue_t;

ssvalue_t ssvalue_string(int, const char *, const char *);
ssvalue_t ssvalue_real32(int, ccf32_t, const char *);
ssvalue_t ssvalue_real64(int, ccf64_t, const char *);
ssvalue_t ssvalue_uvar64(int, ccu64_t, const char *);
ssvalue_t ssvalue_svar64(int, cci64_t, const char *);
ssvalue_t ssvalue_svar32(int, cci32_t, const char *);
ssvalue_t ssvalue_uvar32(int, ccu32_t, const char *);
ssvalue_t ssvalue_bool32(int, cci32_t, const char *);

typedef struct ssi32x2_t ssi32x2_t;
typedef struct ssi32x2_t
{
  int x,y;
} ssi32x2_t;

typedef struct ssi32x4_t ssi32x4_t;
typedef struct ssi32x4_t
{
  ssi32x2_t min;
  ssi32x2_t max;
} ssi32x4_t;

typedef struct ssr32x2_t ssr32x2_t;
typedef struct ssr32x2_t
{
  ccf32_t x,y;
} ssr32x2_t;

typedef struct ssr32x3_t ssr32x3_t;
typedef struct ssr32x3_t
{
  ccf32_t x,y,z;
} ssr32x3_t;

int ssvarlen(ccu64_t value)
{ int length;
  for(length=1;value>=0x80;++length)
    value>>=7;
  return length;
}

int sstype_is_length_encoded(sstype_t type)
{
  return (type>=sstype_kLENGTH)&&(type<=sstype_kRECORD);
}

ssvalue_t ssvalue_uvar64(int name, ccu64_t value, const char *string)
{
  return (ssvalue_t){sstype_kUVARINT,name,ssvarlen(value),value,string};
}

ssvalue_t ssvalue_svar64(int name, cci64_t value, const char *string)
{
  return (ssvalue_t){sstype_kSVARINT,name,ssvarlen(value),value,string};
}

ssvalue_t ssvalue_uvar32(int name, ccu32_t value, const char *string)
{
  return (ssvalue_t){sstype_kUVARINT,name,ssvarlen(value),value,string};
}

ssvalue_t ssvalue_svar32(int name, cci32_t value, const char *string)
{
  return (ssvalue_t){sstype_kSVARINT,name,ssvarlen(value),value,string};
}

ssvalue_t ssvalue_bool32(int name, cci32_t value, const char *string)
{
  return (ssvalue_t){sstype_kBOOLEAN32,name,1,value,string};
}

ssvalue_t ssvalue_real32(int name, ccf32_t value, const char *string)
{
  ssvalue_t result=(ssvalue_t){sstype_kREAL32,name,sizeof(value),0,string};
  result.real64=value;
  return result;
}

ssvalue_t ssvalue_real64(int name, ccf64_t value, const char *string)
{
  ssvalue_t result=(ssvalue_t){sstype_kREAL64,name,sizeof(value),0,string};
  result.real64=value;
  return result;
}

ssvalue_t ssvalue_string(int name, const char *value, const char *string)
{
  return (ssvalue_t){sstype_kSTRING,name,ccCstrlenS(value),(ccu64_t)value,string};
}

ccu64_t ssload_uvar64(ssstack_t *stack)
{
  ccu64_t value=0;
  for(int i=0; i<10; ++i)
  {
    int code=*stack->cursor++;
    value|=(code&0x7f)<<i*7;

    if(!(code&0x80))
      break;
  }

  return(value);
}

// Todo:
cci64_t ssload_svar64(ssstack_t *stack)
{
  return (cci64_t) ssload_uvar64(stack);
}

ccu32_t ssload_uvar32(ssstack_t *stack)
{
  return (ccu32_t) ssload_uvar64(stack);
}

ccu32_t ssload_uint32(ssstack_t *stack)
{ ccu32_t value=*(ccu32_t *)stack->cursor;
  stack->cursor+=sizeof(value);
  return value;
}

cci32_t ssload_sint32(ssstack_t *stack)
{ cci32_t value=*(cci32_t *)stack->cursor;
  stack->cursor+=sizeof(value);
  return value;
}

ccu64_t ssload_uint64(ssstack_t *stack)
{ ccu64_t value=*(ccu64_t *)stack->cursor;
  stack->cursor+=sizeof(value);
  return value;
}

cci64_t ssload_sint64(ssstack_t *stack)
{ cci64_t value=*(cci64_t *)stack->cursor;
  stack->cursor+=sizeof(value);
  return value;
}

ccu16_t ssload_uint16(ssstack_t *stack)
{ ccu16_t value=*(ccu16_t *)stack->cursor;
  stack->cursor+=sizeof(value);
  return value;
}

cci16_t ssload_sint16(ssstack_t *stack)
{ cci16_t value=*(cci16_t *)stack->cursor;
  stack->cursor+=sizeof(value);
  return value;
}

ccu8_t ssload_uint8(ssstack_t *stack)
{ ccu8_t value=*(ccu8_t *)stack->cursor;
  stack->cursor+=sizeof(value);
  return value;
}

cci8_t ssload_sint8(ssstack_t *stack)
{ cci8_t value=*(cci8_t *)stack->cursor;
  stack->cursor+=sizeof(value);
  return value;
}

ccf32_t ssload_real32(ssstack_t *stack)
{ ccf32_t value=*(ccf32_t *)stack->cursor;
  stack->cursor+=sizeof(value);
  return value;
}

ccf64_t ssload_real64(ssstack_t *stack)
{ ccf64_t value=*(ccf64_t *)stack->cursor;
  stack->cursor+=sizeof(value);
  return value;
}

void ssstore_svari(ssstack_t *stack, cci64_t value)
{
  for(;value>=0x80;value>>=7)
  {
    *stack->cursor++=(ccu8_t)(value|0x80);
  }

  *stack->cursor++=(ccu8_t)(value);
}

void ssstore_uvari(ssstack_t *stack, ccu64_t value)
{
  for(;value>=0x80;value>>=7)
  {
    *stack->cursor++=(ccu8_t)(value|0x80);
  }

  *stack->cursor++=(ccu8_t)(value);
}

ccu64_t ssencode_label(sslabel_t label)
{
  int bit=0;
  switch(label.type)
  { case sstype_kTYPELESS32:
    case sstype_kUNSIGNED32:
    case sstype_kSIGNED32:
    case sstype_kREAL32: bit=5;
    break;
    case sstype_kTYPELESS64:
    case sstype_kUNSIGNED64:
    case sstype_kSIGNED64:
    case sstype_kREAL64: bit=1;
    break;
    case sstype_kLENGTH:
    case sstype_kRECORD:
    case sstype_kSTRING: bit=2;
    break;
    case sstype_kBOOLEAN32:
    case sstype_kSVARINT:
    case sstype_kUVARINT: bit=0;
    break;
  }

  return (label.name<<3)|(bit);
}

sslabel_t ssdecode_label(ccu64_t raw)
{ sslabel_t label;

  int fmt,tag;
  fmt=(ccu32_t)(raw&0b0111);
  tag=(ccu32_t)(raw>>3);

  sstype_t bit=sstype_kTYPELESS;
  if((fmt==0)) bit=sstype_kUVARINT;    else
  if((fmt==1)) bit=sstype_kTYPELESS64; else
  if((fmt==2)) bit=sstype_kLENGTH;     else
  if((fmt==5)) bit=sstype_kTYPELESS32;

  label.type=bit;
  label.name=tag;
  return label;
}

void ssread_record(ssread_t *read, size_t length, unsigned char *memory)
{ if(read->level<ARRAYSIZE(read->stack))
  { ssstack_t *stack=read->stack+read->level++;
    stack->length=length;
    stack->memory=memory;
    stack->cursor=memory;
  } else
    cctraceerr("stack overflow",0);
}

void ssread_record_end(ssread_t *read)
{ if(read->level>=1)
  { ssstack_t *inner=read->stack +-- read->level;
    if(read->level>=1)
    { ssstack_t *super=inner-1;
      super->cursor=inner->memory+inner->length;
    }
  } else
    cctraceerr("stack underflow",0);
}

void ssvalue_member(ssvalue_t *record, ssvalue_t value)
{
  *ccarradd(record->member,1)=value;
}

void ssvalue_delete(ssvalue_t *value)
{
  if(value->label.type==sstype_kRECORD)
  { ssvalue_t *member;
    ccarreach(value->member,member)
      ssvalue_delete(member);
    ccarrdel(value->member);
  }
}

size_t ssvalue_serialsizeof(ssvalue_t *value)
{
  size_t result=0;
  if(value->label.type==sstype_kRECORD)
  { ssvalue_t *member;
    ccarreach(value->member,member)
    { result+=ssvarlen(ssencode_label(member->label));
      size_t content=ssvalue_serialsizeof(member);
      if(sstype_is_length_encoded(member->label.type))
      {
        result+=ssvarlen(content);
      }
      result+=content;
    }
  } else
    result=value->length;

  return result;
}

// Todo: speed!
void ssvalue_serialize_ex(ssstack_t *stack, ssvalue_t *value)
{
  switch(value->label.type)
  { case sstype_kRECORD:
    { ssvalue_t *member;
      ccarreach(value->member,member)
      { ssstore_uvari(stack,ssencode_label(member->label));
        if(sstype_is_length_encoded(member->label.type))
          ssstore_uvari(stack,ssvalue_serialsizeof(member));
        ssvalue_serialize_ex(stack,member);
      }
    } break;
    case sstype_kLENGTH:
    case sstype_kSTRING:
    { ccassert(value->length!=0);
      ccassert(value->memory!=0);
      memcpy(stack->cursor,value->memory,value->length);
      stack->cursor+=value->length;
    } break;
    case sstype_kUVARINT:
    { ssstore_uvari(stack,value->uint64);
    } break;
    case sstype_kSVARINT:
    { ssstore_svari(stack,value->uint64);
    } break;
    case sstype_kTYPELESS64:
    case sstype_kUNSIGNED64:
    case   sstype_kSIGNED64:
    case    sstype_kREAL64:
    case  sstype_kBOOLEAN32:
    case sstype_kTYPELESS32:
    case sstype_kUNSIGNED32:
    case   sstype_kSIGNED32:
    case    sstype_kREAL32:
    { ccassert(value->length!=0);
      memcpy(stack->cursor,&value->memory,value->length);
      stack->cursor+=value->length;
    } break;
    default:
    { ccassert(!"error");
    } break;
  }
}

ssblock_t ssserialize(ssvalue_t *value)
{ size_t length=ssvalue_serialsizeof(value);
  void  *memory=ccmalloc(length);

  ssstack_t stack;
  stack.memory=memory;
  stack.cursor=memory;
  stack.length=length;
  ssvalue_serialize_ex(&stack,value);
  return stack.serial;
}

sslabel_t ssread_fieldlabel(ssread_t *reader)
{
  ccassert(reader->has);

  sslabel_t label=reader->label;

  reader->has=0;
  reader->label.type=0;
  reader->label.name=0;
  return label;
}

int ssread_fieldname(ssread_t *read)
{
  ccassert(read->has);
  return read->label.name;
}

void ssread_typeless(ssread_t *read)
{ sslabel_t label=ssread_fieldlabel(read);
  ssstack_t *stack=read->stack+read->level-1;
  switch(label.type)
  { case sstype_kUVARINT:
    case sstype_kSVARINT:
    { ssload_uvar64(stack);
    } break;
    case sstype_kLENGTH:
    case sstype_kSTRING:
    case sstype_kRECORD:
    { stack->cursor+=ssload_uvar64(stack);
    } break;
    case sstype_kTYPELESS32:
    { stack->cursor+=4;
    } break;
    case sstype_kTYPELESS64:
    { stack->cursor+=8;
    } break;
  }
}

int ssread_real64(ssread_t *read, ccf64_t *value)
{ sslabel_t label=ssread_fieldlabel(read);
  ssstack_t *stack=read->stack+read->level-1;
  if((label.type==sstype_kTYPELESS64)||
     (label.type==sstype_kREAL64)||
     (label.type==sstype_kSIGNED64)||
     (label.type==sstype_kUNSIGNED64))
  {
    *value=ssload_real64(stack);
    return 1;
  } else
    cctraceerr("type specified is too different",0);
  return 0;
}

int ssread_real32(ssread_t *read, ccf32_t *value)
{ sslabel_t label=ssread_fieldlabel(read);
  ssstack_t *stack=read->stack+read->level-1;
  if((label.type==sstype_kTYPELESS32)||
     (label.type==sstype_kREAL32)||
     (label.type==sstype_kSIGNED32)||
     (label.type==sstype_kUNSIGNED32))
  {
    *value=ssload_real32(stack);
    return 1;
  } else
    cctraceerr("type specified is too different",0);
  return 0;
}

int ssread_uvar64(ssread_t *read, ccu64_t *value)
{ sslabel_t label=ssread_fieldlabel(read);
  ssstack_t *stack=read->stack+read->level-1;
  if(label.type==sstype_kUVARINT)
  { *value=ssload_uvar64(stack);
    return 1;
  } else
    cctraceerr("type specified is too different",0);
  return 0;
}

int ssread_uvar32(ssread_t *read, ccu32_t *value)
{ sslabel_t label=ssread_fieldlabel(read);
  ssstack_t *stack=read->stack+read->level-1;
  if(label.type==sstype_kUVARINT)
  { *value=(ccu32_t)ssload_uvar32(stack);
    return 1;
  } else
    cctraceerr("type specified is too different",0);
  return 0;
}

// Todo:
int ssread_svar32(ssread_t *read, cci32_t *value)
{ sslabel_t label=ssread_fieldlabel(read);
  ssstack_t *stack=read->stack+read->level-1;
  if(label.type==sstype_kUVARINT)
  { *value=(cci32_t)ssload_uvar32(stack);
    return 1;
  } else
    cctraceerr("type specified is too different",0);
  return 0;
}

// Todo:
int ssread_enumerator(ssread_t *read, void *value)
{
  return ssread_uvar32(read,(unsigned int *)value);
}

int ssread_string(ssread_t *read, char **value)
{ sslabel_t label=ssread_fieldlabel(read);
  ssstack_t *stack=read->stack+read->level-1;
  if((label.type==sstype_kLENGTH)||
     (label.type==sstype_kSTRING))
  { ccu64_t length=ssload_uvar64(stack);
    *value=(char *)stack->cursor;
    stack->cursor+=length;
    return 1;
  } else
    cctraceerr("type specified is too different",0);
  return 0;
}

int ssread_bytes(ssread_t *read, unsigned char **value)
{ sslabel_t label=ssread_fieldlabel(read);
  ssstack_t *stack=read->stack+read->level-1;
  if((label.type==sstype_kLENGTH) ||
     (label.type==sstype_kSTRING))
  { ccu64_t length=ssload_uvar64(stack);
    *value=stack->cursor;
    stack->cursor+=length;
    return 1;
  } else
    cctraceerr("type specified is too different",0);
  return 0;
}

int ssread_field(ssread_t *read)
{
  if(read->has!=0)
  { if(read->cue!=0)
    { sslabel_t label=ssread_fieldlabel(read);
      if((label.type==sstype_kLENGTH) ||
         (label.type==sstype_kRECORD))
      { ssstack_t *stack=read->stack+read->level-1;
        ccu64_t length=ssload_uvar64(stack);
        ssread_record(read,length,stack->cursor);
      } else
        cctraceerr("type specified is too different",0);
    } else
      ssread_typeless(read);
  }

  read->cue=ccfalse;

  ssstack_t *stack=read->stack+read->level-1;

  unsigned char *cursor_min,*cursor_max;
  cursor_min=stack->memory;
  cursor_max=stack->memory+stack->length;

  if((stack->cursor>=cursor_min)&&(stack->cursor<cursor_max))
  { read->label=ssdecode_label(ssload_uvar64(stack));
    read->has=1;
  } else
  if((stack->cursor==cursor_max))
  { ssread_record_end(read);
    return 0;
  } else
  { cctraceerr("error, cursor out of bounds",0);
    return 0;
  }

  return 1;
}

void ssread_record_cue(ssread_t *read)
{
  ccassert(!read->cue);
  read->cue=1;
}

void ssread_apportion(ssread_t *read, size_t length, void *memory)
{
  memset(read,0,sizeof(*read));
  ssread_record(read,length,memory);
}

#define ForMessageField(read) ssread_record_cue((read)); while(ssread_field((read))) switch(ssread_fieldname((read)))

#endif