// Copyright(C) 2022-2023 Dayan Rodriguez, All rights reserved.
#ifndef ALI_THREAD
#define ALI_THREAD

#ifndef ali_create_thread_access_entity
# define ali_create_thread_access_entity() ZenCreateNativeMutex(FALSE,NULL)
#endif
#ifndef ali_acquire_single_thread_access
# define ali_acquire_single_thread_access(entity) ZenAcquireNativeMutex(entity)
#endif
#ifndef ali_release_single_thread_access
# define ali_release_single_thread_access(entity) ZenReleaseNativeMutex(entity)
#endif

#define ALI_TYPE_FIRST -1
#define ALI_TYPE_LAST  -2

typedef void *ali_thread_access_entity;

// Todo:
typedef struct ali_thread_message
{ int32_t   type;
  ali_arena arena;

              union
{ ali_response *response;
};
} ali_thread_message;

typedef struct ali_thread
{ // Note: assuming there's only one thread producing responses here ...
  ali_response *first_response;
  ali_response *first_free_response;

  // Todo: probably replace with a ticket styled lock ...
  ali_thread_access_entity   message_access;

  ali_thread_message  *message_array;
  int32_t              message_count_min;
  int32_t              message_count_max;
} ali_thread;

static ali_response *
ali_thread_allocate_response(ali_thread *thread)
{ ali_response *result;
  result=thread->first_free_response;
  if(!result)
  { result=(ali_response *)ali_malloc(sizeof(*result));
    result->next=thread->first_response;
    thread->first_response=result;
  }
  return result;
}

static void
ali_thread_release_message(ali_thread_message *message)
{
  ali_rst_(&message->arena);
  ali_free(message->response);
}

static int
ali_thread_init(ali_thread *thread)
{
  thread->message_access=ali_create_thread_access_entity();

  // Todo: memory ...
  thread->message_count_max=0x100;
  thread->message_count_min=0;
  thread->message_array=(ali_thread_message*)mg_malloc(sizeof(*thread->message_array)*thread->message_count_max);

  return TRUE;
}

static int
ali_thread_read_message(ali_thread *thread, int32_t type, int32_t remove, ali_thread_message *message)
{ ali_thread_message *mssg;
  mssg=0;
  if(ali_acquire_single_thread_access(thread->message_access))
  { if(thread->message_count_min)
    { ali_thread_message *iter,*last;
      iter=thread->message_array;
      last=thread->message_array+thread->message_count_min-1;
      if(type==ALI_TYPE_FIRST)
      { mssg=iter;
      } else
      if(type==ALI_TYPE_LAST)
      { mssg=last;
      } else
      { while((iter->type!=type) && (iter<=last)) ++iter;
        if(iter->type==type) mssg=iter;
      }
      if(mssg)
      { *message=*mssg;
        if(remove)
        { if(mssg!=last) *mssg=*last;
          -- thread->message_count_min;
        }
      }
    }
    ali_release_single_thread_access(thread->message_access);
  }
  return mssg!=0;
}

static int
ali_thread_write_message(ali_thread *thread, int32_t type, ali_arena arena, ali_response *response)
{
  if(ali_acquire_single_thread_access(thread->message_access))
  {
    if(thread->message_count_min<thread->message_count_max)
    { ali_thread_message *write;
      write=thread->message_array+thread->message_count_min++;
      write->type=type;
      write->arena=arena;
      write->response=response;
    } else
    {
      // Todo: resize the queue
      Assert(!"error");
    }

    ali_release_single_thread_access(thread->message_access);
    return 1;
  }
  return 0;
}

static int
ali_thread_await_message(ali_thread *thread, int32_t type, int32_t remove, ali_thread_message *message)
{
  // Todo:
  while(!ali_thread_read_message(thread,type,remove,message));

  return 1;
}


#endif