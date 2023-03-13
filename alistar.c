/*
**
** -+- AliStar -+-
**
** Copyright(C) Dayan Rodriguez, 2022, All rights reserved.
*/
#ifndef ALISTAR_C
#define ALISTAR_C

void *send_payload(context_t *context, int overlapped, ssvalue_t payload)
{
  ssvalue_t value=(ssvalue_t){sstype_kRECORD};
  ssvalue_member(&value,payload);

  ssblock_t block=ssserialize(&value);
  int snd=mg_websocket_write(context->conn,MG_WEBSOCKET_OPCODE_BINARY,(char*)block.memory,block.length);
  ccfree(block.memory);

  int name=payload.label.name;

  return poll_thread_message(&context->thread,overlapped,name);
}

ssvalue_t CreateParticipantPlayerSetup(int name_, const char *name)
{ ssvalue_t req=(ssvalue_t){sstype_kRECORD,name_};
  ssvalue_member(&req,ssvalue_svar32(1,PARTICIPANT,"participant"));
  ssvalue_member(&req,ssvalue_string(4,name,"name"));
  return req;
}

ssvalue_t CreateComputerPlayerSetup(int name_, int race, int diff, int build, const char *name)
{ ssvalue_t req=(ssvalue_t){sstype_kRECORD,name_};
  ssvalue_member(&req,ssvalue_svar32(1,COMPUTER,"player_type"));
  ssvalue_member(&req,ssvalue_svar32(2,race,"player_race"));
  ssvalue_member(&req,ssvalue_svar32(3,diff,"difficulty"));
  ssvalue_member(&req,ssvalue_string(4,name,"player_name"));
  ssvalue_member(&req,ssvalue_svar32(5,build,"build_type"));
  return req;
}

ssvalue_t ValuePoint2D(int name_, ssr32x2_t val)
{ ssvalue_t result=(ssvalue_t){sstype_kRECORD,name_};
  ssvalue_member(&result,ssvalue_real32(1,val.x,"x"));
  ssvalue_member(&result,ssvalue_real32(2,val.y,"y"));
  return result;
}

ssvalue_t ValueSize2DI(int name_, ssi32x2_t val)
{ ssvalue_t result=(ssvalue_t){sstype_kRECORD,name_};
  ssvalue_member(&result,ssvalue_svar32(1,val.x,"x"));
  ssvalue_member(&result,ssvalue_svar32(2,val.y,"y"));

  ssblock_t serial=ssserialize(&result);
  ssread_t read;
  ssread_apportion(&read,serial.length,serial.memory);
  ssi32x2_t mat;
  ssread_i32x2(&read,&mat);
  ccassert(mat.x==val.x);
  ccassert(mat.y==val.y);

  return result;
}

ssvalue_t AliCameraSetupValueNoMinimap(
  int name_, ssi32x2_t resolution, float width, int crop_to_playable_area, int allow_cheating_layers)
{
  ssvalue_t result=(ssvalue_t){sstype_kRECORD,name_};
  ssvalue_member(&result,ValueSize2DI(2,resolution));
  ssvalue_member(&result,ssvalue_real32(1,width,"width"));
  ssvalue_member(&result,ssvalue_svar32(4,crop_to_playable_area,"crop_to_playable_area"));
  ssvalue_member(&result,ssvalue_svar32(8,allow_cheating_layers,"allow_cheating_layers"));
  return result;
}

void *request_available_maps(context_t *context, int overlapped)
{
  return send_payload(context,overlapped,(ssvalue_t){sstype_kRECORD,17});
}

void *request_create_game(context_t *context, int overlapped)
{
  ssvalue_t payload=(ssvalue_t){sstype_kRECORD,1};
  ssvalue_member(&payload,ssvalue_string(2,"16-Bit LE","map_name"));
  ssvalue_member(&payload,ssvalue_svar32(6,1,"realtime"));
  ssvalue_member(&payload,CreateParticipantPlayerSetup(3,"RoyJacobs"));
  ssvalue_member(&payload,CreateComputerPlayerSetup(3,ZERG,7,5,"DakotaBrown"));
  return send_payload(context,overlapped,payload);
}

ssvalue_t ssvalue_spatial_camera_setup(
  int name_, ssi32x2_t resolution, ssi32x2_t minimap_resolution, float width, int crop_to_playable_area, int allow_cheating_layers)
{
  ssvalue_t result=(ssvalue_t){sstype_kRECORD,name_,0,0,"spatial_camera_setup"};
  ssvalue_member(&result,ValueSize2DI(2,resolution));
  ssvalue_member(&result,ValueSize2DI(3,minimap_resolution));
  ssvalue_member(&result,ssvalue_real32(1,width,"width"));
  ssvalue_member(&result,ssvalue_svar32(4,crop_to_playable_area,"crop_to_playable_area"));
  ssvalue_member(&result,ssvalue_svar32(8,allow_cheating_layers,"allow_cheating_layers"));
  return result;
}

ssvalue_t ssvalue_default_interface_options(int name_)
{ ssvalue_t result=(ssvalue_t){sstype_kRECORD,name_,0,0,"default_interface_options"};
  ssvalue_member(&result,ssvalue_bool32(1,TRUE,"raw"));
  ssvalue_member(&result,ssvalue_bool32(8,TRUE,"show_placeholders"));
  // ssvalue_member(&result,4,AliCameraSetupValueNoMinimap({512,512},8.f,FALSE,TRUE));
  ssvalue_member(&result,ssvalue_spatial_camera_setup(3,(ssi32x2_t){255,255},(ssi32x2_t){256,256},256.f,FALSE,TRUE));
  ssvalue_member(&result,ssvalue_spatial_camera_setup(4,(ssi32x2_t){512,512},(ssi32x2_t){256,256},256.f,FALSE,TRUE));
  return result;
}

void *request_join_game_as_participant(context_t *context, int overlapped, int race, const char *name)
{ ssvalue_t payload={sstype_kRECORD,2};
  ssvalue_member(&payload,ssvalue_svar32(1,race,"race"));
  ssvalue_member(&payload,ssvalue_default_interface_options(3));
  ssvalue_member(&payload,ssvalue_string(7,name,"player_name"));
  return send_payload(context,overlapped,payload);
}

void *request_game_info(context_t *context, int overlapped)
{
  return send_payload(context,overlapped,(ssvalue_t){sstype_kRECORD,9});
}

void *request_observation(context_t *context, int overlapped, int disable_fog, int game_loop)
{ ssvalue_t payload={sstype_kRECORD,10};
  ssvalue_member(&payload,ssvalue_svar32(1,disable_fog,"disable_fog"));
  // AddVarint32Value(&pay,/* tag */ 2, game_loop);
  return send_payload(context,overlapped,payload);
}

void push_thread_message(thread_t *thread, int type, void *value)
{
  acquire_mutex(thread->mutex);
  thread_message_t *message=ccarradd(thread->array,1);
  message->type=type;
  message->value=value;
  release_mutex(thread->mutex);
}

void *poll_thread_message(thread_t *thread, int overlapped, int remove)
{
  void *result=ccnull;

  do
  { acquire_mutex(thread->mutex);

    int arrlen=ccarrlen(thread->array);

    thread_message_t *head=thread->array+thread->count;
    thread_message_t *tail=thread->array+arrlen;

    if(head!=tail)
    {
      if(remove>=0)
      {
        for(;head<tail;++head)
        {
          if(!head->type||head->type==remove)
          {
            result=head->value;
            thread->count++;
            break;
          }
        }
      } else
      {
        result=tail[remove].value;
        thread->count++;
      }
    }

    release_mutex(thread->mutex);

  } while(!result && !overlapped);

  return result;
}

#if defined(_WIN32)

mutex_t create_mutex()
{
  mutex_t result=(mutex_t)CreateMutexA(NULL,FALSE,NULL);
  return result;
}

void acquire_mutex(mutex_t mutex)
{
  while(WaitForSingleObject((HANDLE)mutex,INFINITE)==WAIT_TIMEOUT);
}

void release_mutex(mutex_t mutex)
{
  ReleaseMutex((HANDLE)mutex);
}

pipe_t create_system_pipe(int read_inheritable, int write_inheritable)
{ SECURITY_ATTRIBUTES secattr;
  secattr.nLength=sizeof(SECURITY_ATTRIBUTES);
  secattr.lpSecurityDescriptor=0;
  secattr.bInheritHandle=0;
  pipe_t pipe;
  CreatePipe(&pipe.read,&pipe.write,&secattr,0);
  SetHandleInformation(pipe.read,HANDLE_FLAG_INHERIT,read_inheritable!=0);
  SetHandleInformation(pipe.write,HANDLE_FLAG_INHERIT,write_inheritable!=0);
  return pipe;
}

int launch_system_process(process_t *process, const char *exe, const char *pwd, const char *cmd)
{
  process->Input=create_system_pipe(TRUE,FALSE);
  process->Output=create_system_pipe(FALSE,TRUE);
  process->Error=create_system_pipe(FALSE,TRUE);

  STARTUPINFOA u;
  ZeroMemory(&u,sizeof(u));
  u.cb=sizeof(STARTUPINFOA);
  u.dwFlags=STARTF_USESTDHANDLES;
  u.hStdInput=process->Input.read;
  u.hStdOutput=process->Output.write;
  u.hStdError=process->Error.write;

  PROCESS_INFORMATION i;
  ZeroMemory(&i,sizeof(i));
  if(CreateProcessA((char*)exe,(char*)cmd,NULL,NULL,TRUE,0x00,NULL,(char*)pwd,&u,&i))
  { process->ProcessHandle=(void*)i.hProcess;
    process->ProcessId=i.dwProcessId;

    process->ThreadHandle=(void*)i.hThread;
    process->ThreadId=i.dwThreadId;
    return 1;
  }
  return 0;
}

int launch_starcraft_process(process_t *process, const char *installation, int arch64, const char *addr, int port)
{ char exec[MAX_PATH];
  char dlls[MAX_PATH];
  char cmdl[MAX_PATH];
  if(GetFileAttributesA(installation)==INVALID_FILE_ATTRIBUTES)
  { cctraceerr("'%s': invalid installation directory",installation);
    return 0;
  }
  sprintf_s(exec,MAX_PATH,"%s\\versions\\Base89165\\SC2%s.exe",installation,(arch64?"_x64":""));
  sprintf_s(dlls,MAX_PATH,"%s\\Support%s",installation,(arch64?"64":""));

  if(GetFileAttributesA(exec)==INVALID_FILE_ATTRIBUTES)
  { cctraceerr("'%s': executable not found",exec);
    return 0;
  }
  if(GetFileAttributesA(dlls)==INVALID_FILE_ATTRIBUTES)
  { cctraceerr("'%s': support directory not found",dlls);
    return 0;
  }
  sprintf_s(cmdl,sizeof(cmdl)," -listen %s -port %i -displayMode 0 -windowx 1921 -windowy 0",addr,port);
  return launch_system_process(process,exec,dlls,cmdl);
}
#endif

#endif
