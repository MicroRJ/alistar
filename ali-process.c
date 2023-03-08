// Copyright(C) 2022-2023 Dayan Rodriguez, All rights reserved.
#ifndef ALI_PROCESS
#define ALI_PROCESS

typedef struct
{ void *process_handle;
  void *process_id;
} ali_process;

#ifndef ali_find_starcraft_installation_directory
# define ali_find_starcraft_installation_directory(context,is_arch64,size,buffer)
#endif
#ifndef ali_find_starcraft_support_directory
# define ali_find_starcraft_support_directory(context,is_arch64,size,buffer)
#endif
#ifndef ali_find_starcraft_executable
# define ali_find_starcraft_executable(context,is_arch64,size,buffer)
#endif
#ifndef ali_launch_starcraft
# define ali_launch_starcraft(context,process,is_arch64,dism,wndx,wndy,wndw,wndh,addr,port)
#endif


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
  int res=FindStarcraftExecutableAndDllsDirectory(inst,arch64,exec,dlls);

  // Note: if you have a double monitor setup this could be useful,
  // however, if you have a faulty GPU card that can barely run
  // two monitors at once, this is useless ...
  if(!res) return {};

  sprintf_s(cmdl,sizeof(cmdl)," -listen %s -port %i -displayMode 0 -windowx 1921 -windowy 0",addr,port);

  //"-windowwidth"
  //"-windowheight"

  ZenSystemProcess proc;
  ZenLaunchSystemProcess(&proc,exec,dlls,cmdl);

  return proc;
}

#endif