// Copyright(C) 2022-2023 Dayan Rodriguez, All rights reserved.
#ifndef ALI_ALLOCATOR
#define ALI_ALLOCATOR

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

// Note: static block metadata
typedef struct ali_sbm
{ size_t   size;
} ali_sbm;

// Note: static linked block
typedef struct ali_slb
{ ali_slb *next;
} ali_slb;

// Note: dynamic linked block
typedef struct ali_dlb
{ uint16_t slot_rax;
  size_t   size_min;
  size_t   size_max;
} ali_dlb;

typedef struct ali_arena
{ ali_dlb *dlb[0x100]; // Todo:
  int16_t  dlb_rax;

  ali_slb *slb; // Todo: switch to dynamic array too?
  size_t   slb_rem;
  size_t   slb_len;
} ali_arena;

#define AliArrayAdd(stt,arr) ((arr)+ali_dynarr_add_(&(stt)->arena,(void **)&(arr),sizeof(*(arr)),1,1))

#define ali_memsze(mem) ((size_t *)mem)[-1]

#define ali_tally(arre) (ali_memlen(arr)/sizeof(*(arr)))

static void
ali_rst_(ali_arena *stt)
{
  ali_slb *itr,*next;
  for(itr=stt->slb;itr;itr=next)
  { next=itr->next;
    mg_free(itr);
  }
  stt->slb=0;
  stt->slb_rem=0;


  ali_dlb **dlb;
  for(dlb=stt->dlb;dlb<stt->dlb+stt->dlb_rax;++dlb)
  { mg_free(*dlb);
  }

  ZeroMemory(stt->dlb,ARRAYSIZE(stt->dlb));
  stt->dlb_rax=0;
}

static size_t
ali_arena_dadd_(ali_arena *stt, ali_dlb **dlb_ptr, size_t res, size_t com)
{
  ali_dlb *dlb;
  dlb=*dlb_ptr;

  size_t   size_max,size_min;
  uint16_t slot_rax;

  slot_rax=dlb?dlb->slot_rax:stt->dlb_rax++;
  size_max=dlb?dlb->size_max:0;
  size_min=dlb?dlb->size_min:0;

  Assert(stt->dlb_rax<ARRAYSIZE(stt->dlb)); // Todo:


  if(size_min+com>size_max)
  { size_max<<=1;
    if(size_min+com>size_max)
    { size_max=size_min+com;
    }

    dlb=(ali_dlb*)mg_realloc(dlb,sizeof(*dlb)+size_max);
    stt->dlb[slot_rax]=dlb;

    *dlb_ptr=dlb;
  }

  dlb->slot_rax=slot_rax;
  dlb->size_max=size_max;
  dlb->size_min=size_min+res;
  return size_min;
}

static char *
ali_arena_sadd_(ali_arena *stt, size_t res, size_t com)
{
#ifndef STRING_BLOCK_SIZE_MIN
# define STRING_BLOCK_SIZE_MIN 0x200
#endif
#ifndef STRING_BLOCK_SIZE_MAX
# define STRING_BLOCK_SIZE_MAX 0xffff
#endif

  size_t external_size;
  external_size=sizeof(ali_sbm)+com;

  if (external_size>stt->slb_rem)
  { size_t internal_size;
    internal_size=stt->slb_len;
    internal_size=STRING_BLOCK_SIZE_MIN<<(internal_size>>1);
    if (internal_size>STRING_BLOCK_SIZE_MAX)
    { ++stt->slb_len;
    }


    if (external_size>internal_size)
    { ali_slb *blc;
      blc=(ali_slb *)mg_malloc(sizeof(*blc)+external_size);

      if (stt->slb)
      { // Note: block is full already, put _after_ the first one then ...
        blc->next=stt->slb->next;
        stt->slb->next=blc;
      } else
      {
        blc->next=0;
        stt->slb_rem=0;
        stt->slb=blc;
      }

      ali_sbm *sbm;
      sbm=(ali_sbm *)((char*)(blc+1));
      sbm->size=res;
      return(char*)(sbm+1);
    } else
    { ali_slb *blc;
      blc=(ali_slb *)mg_malloc(sizeof(*blc)+internal_size);

      size_t rem;
      rem=internal_size-external_size;

      if(rem>stt->slb_rem) // Note: this buffer has more free space than the previous buffer, set it as first ...
      { blc->next=stt->slb;
        stt->slb=blc;
      } else
      { blc->next=stt->slb->next;
        stt->slb->next=blc;
      }

      stt->slb_rem=rem; // TODO(RJ):

      ali_sbm *sbm;
      sbm=(ali_sbm *)((char*)(blc+1)+(rem));
      sbm->size=res;
      return(char*)(sbm+1);
    }
  } else
  { ali_slb *blc;
    blc=(ali_slb *)stt->slb;

    size_t rem;
    rem=stt->slb_rem-external_size;

    stt->slb_rem=rem;

    ali_sbm *sbm;
    sbm=(ali_sbm *)((char*)(blc+1)+(rem));
    sbm->size=res;
    return(char*)(sbm+1);
  }
}

static size_t
ali_dynarr_add_(ali_arena *stt, void **arr, size_t isze, size_t cres, size_t ccom)
{ ali_dlb *dlb;
  dlb=(*arr)?((ali_dlb *)*arr-1):(0);

  size_t res;
  res=ali_arena_dadd_(stt,&dlb,isze*cres,isze*ccom);

  *arr=dlb+1;
  return cres / isze;
}

#endif