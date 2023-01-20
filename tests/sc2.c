#pragma warning(disable:4530)
#pragma warning(disable:4267)
#pragma warning(disable:4146)
#pragma warning(disable:4100)
#pragma warning(disable:4127)
#pragma warning(disable:4189)
#pragma warning(disable:4244)

#pragma comment(lib, "vendor\\protobuf\\build\\Debug\\libprotobufd")
#define _CRT_SECURE_NO_WARNINGS
#include <string>
#include <vector>
#include "sc2api.pb.h"

#define ZEN_APPMODE_WINDOWED
#include "brazen.h"
#include "..\\alistar.c"


static int Ready=FALSE;

// NOTE(RJ):
// KEEP TRACK OF ACTIVE STARCRAFT PROCESSES AND THEIR RELATED DATA,
// WE DON'T HAVE TO CREATE A NEW PROCESS IF WE HAVE ONE WITH THE VALID
// CONFIGURATION ALREADY!

static i32
SendRequest(mg_connection *Conn, SC2APIProtocol::Request &Request)
{ u32 ByteCount,ByteWrote;
  void *ByteArray;
  ByteCount=Request.ByteSizeLong();
  ByteArray=mg_malloc(ByteCount);
  Request.SerializeToArray((char*)ByteArray,ByteCount);
  ByteWrote=mg_websocket_write(Conn, MG_WEBSOCKET_OPCODE_BINARY, (char*)ByteArray, ByteCount);
  TRACE_I("Sent: bytes(%i/%i)",ByteWrote,ByteCount);
  mg_free(ByteArray);
  return ByteWrote;
}

static i32
RequestAvailableMaps(mg_connection *Conn)
{ SC2APIProtocol::Request Request;
  SC2APIProtocol::RequestAvailableMaps* Payload = Request.mutable_available_maps();
  return SendRequest(Conn, Request);
}

static i32
RequestCreateGame(mg_connection *Conn)
{
  SC2APIProtocol::Request Request;
  SC2APIProtocol::RequestCreateGame &Payload = *Request.mutable_create_game();

  Payload.set_battlenet_map_name("16-Bit LE");
  Payload.set_realtime(TRUE);

  {
    SC2APIProtocol::PlayerSetup &Setup = *Payload.add_player_setup();
    Setup.set_type(SC2APIProtocol::Participant);
    Setup.set_player_name("RoyJacobs");
  }
  {
    SC2APIProtocol::PlayerSetup &Setup = *Payload.add_player_setup();
    Setup.set_type(SC2APIProtocol::Computer);
    Setup.set_player_name("DakotaBrown");
    Setup.set_race(SC2APIProtocol::Terran);
    Setup.set_ai_build(SC2APIProtocol::Macro);
    Setup.set_difficulty(SC2APIProtocol::VeryHard);
  }

  return SendRequest(Conn, Request);
}

static i32
RequestJoinGameAsObservant(mg_connection *Conn, int PlayerId)
{ SC2APIProtocol::Request Request;
  SC2APIProtocol::RequestJoinGame &Payload = *Request.mutable_join_game();
  Payload.set_observed_player_id(PlayerId);
  return SendRequest(Conn, Request);
}


static i32
RequestJoinGameAsParticipant(mg_connection *Conn, SC2APIProtocol::Race Race, std::string &&Name)
{ SC2APIProtocol::Request Request;
  SC2APIProtocol::RequestJoinGame &Payload = *Request.mutable_join_game();
  Payload.set_race(Race);
  Payload.set_player_name(Name);
  SC2APIProtocol::InterfaceOptions &Options = *Payload.mutable_options();
  Options.set_raw(true);
  return SendRequest(Conn, Request);
}


static i32
RequestGameInfo(mg_connection *Conn)
{ SC2APIProtocol::Request Request;
  SC2APIProtocol::RequestGameInfo* Payload = Request.mutable_game_info();
  return SendRequest(Conn, Request);
}

static int
OnSocketDataReceived(mg_connection* Conn, int Flags, char *Data, size_t DataSize, void *UserData)
{
  SC2APIProtocol::Response Response;
  SC2APIProtocol::Response::ResponseCase ResponseCase;

  Response.ParseFromArray(Data, DataSize);
  ResponseCase=Response.response_case();

  switch(ResponseCase)
  { case SC2APIProtocol::Response::ResponseCase::kCreateGame:
    { SC2APIProtocol::ResponseCreateGame Payload;
      SC2APIProtocol::ResponseCreateGame_Error Error;
      std::string ErrorDetails;

      Payload=Response.create_game();

      if(Payload.has_error())
      { Error=Payload.error();
        ErrorDetails=Payload.error_details();

        __debugbreak();
      } else
      {
        RequestJoinGameAsParticipant(Conn,SC2APIProtocol::Race::Terran,"RoyJacobs");
      }
    } break;
  case SC2APIProtocol::Response::ResponseCase::kJoinGame:
    {
      SC2APIProtocol::ResponseJoinGame Payload;
      SC2APIProtocol::ResponseJoinGame_Error Error;
      std::string ErrorDetails;
      uint32_t PlayerId;

      Payload=Response.join_game();
      PlayerId=Payload.player_id();

      if(Payload.has_error())
      { Error=Payload.error();
        ErrorDetails=Payload.error_details();
        __debugbreak();
      }

      RequestGameInfo(Conn);

    } break;
  case SC2APIProtocol::Response::ResponseCase::kObservation:
  {
    SC2APIProtocol::ResponseObservation Payload;
    Payload=Response.observation();

    SC2APIProtocol::Observation Obs = Payload.observation();
    SC2APIProtocol::ObservationRaw ObsRaw = Obs.raw_data();

    int Size=ObsRaw.units_size();
    for(int i=0; i<Size; ++i)
    {
      SC2APIProtocol::Unit Unit = ObsRaw.units(i);
      uint64_t tag = Unit.tag();
      uint64_t type = Unit.unit_type();


      int BREAKPOINT;
      BREAKPOINT=0;
    }



  } break;

  case SC2APIProtocol::Response::ResponseCase::kGameInfo:
    { SC2APIProtocol::ResponseGameInfo Payload;
      SC2APIProtocol::PlayerInfo PlayerInfo;
      std::string BattlenetMapName,LocalMapName,ModName;
      i32 PlayerInfoCount,PlayerId,i;
      PlayerInfoCount=Payload.player_info_size();
      for(i=0;i<PlayerInfoCount;++i)
      { PlayerInfo=Payload.player_info(i);
        PlayerId=PlayerInfo.player_id();
        RequestJoinGameAsObservant(Conn,PlayerId);
        break;
      }

      // std::string message="gl hf";
      // SC2APIProtocol::Request request={};
      // SC2APIProtocol::RequestAction *request_action = request.mutable_action();
      // SC2APIProtocol::Action* action = request_action->add_actions();
      // SC2APIProtocol::ActionChat* action_chat = action->mutable_action_chat();
      // action_chat->set_message(message);
      // SC2APIProtocol::ActionChat::Channel channel_proto=SC2APIProtocol::ActionChat_Channel_Broadcast;
      // action_chat->set_channel(channel_proto);
      // SendRequest(Conn,request);

      Ready=TRUE;


      // SC2APIProtocol::Request request={};
      // SC2APIProtocol::RequestAction *request_action = request.mutable_action();
      // SC2APIProtocol::Action *action = request_action->add_actions();
      // SC2APIProtocol::ActionRaw *action_raw = action->mutable_action_raw();
      // SC2APIProtocol::ActionRawUnitCommand *raw_unit_command = action_raw->mutable_unit_command();
      // raw_unit_command->set_ability_id(524);
      // raw_unit_command->add_unit_tags(51904513);
      // SendRequest(Conn,request);


    } break;
    case SC2APIProtocol::Response::ResponseCase::kAvailableMaps:
    { SC2APIProtocol::ResponseAvailableMaps Payload;
      int LocalMapCount,BattlenetMapCount,i;
      std::string Map;
      Payload=Response.available_maps();
      LocalMapCount=Payload.local_map_paths_size();
      BattlenetMapCount=Payload.battlenet_map_names_size();

      for(i=0;i<LocalMapCount;++i)
      { Map=Payload.local_map_paths(i);
        TRACE_I("Local Map %s", Widen(-1,Map.c_str()));
      }

      for(i=0;i<BattlenetMapCount;++i)
      { Map=Payload.battlenet_map_names(i);
        TRACE_I("Battlenet Map %s", Widen(-1,Map.c_str()));
      }
    } break;
    default:
    {
      __debugbreak();
      Response.ParseFromArray(Data, DataSize);
      ResponseCase=Response.response_case();
      __debugbreak();
    } break;
  }
  return 1;
}

// static i32
// RequestJoinGameAsObservant(mg_connection *Conn)
// { SC2APIProtocol::Request Request;
//   SC2APIProtocol::RequestJoinGame* Payload = Request.mutable_join_game();
//   return SendRequest(Conn, Request);
// }


struct WindowsProcess {
    STARTUPINFO si_;
    PROCESS_INFORMATION pi_;
};
std::vector<WindowsProcess> windows_processes;

uint64_t StartProcess(const std::string& process_path, const std::vector<std::string>& command_line) {
    static const unsigned int buffer_size = (1 << 16) + 1;

    WindowsProcess process;
    std::memset(&process, 0, sizeof(process));
    process.si_.cb = sizeof(process.si_);

    char current_directory[buffer_size];
    char support_directory[buffer_size];
    GetCurrentDirectory(buffer_size, current_directory);
    strcpy_s(support_directory, process_path.c_str());
    int slashcount = 0;
    for (size_t i = strlen(support_directory); i > 0 && slashcount < 3; --i) {
        if (support_directory[i] == '/' || support_directory[i] == '\\') {
            ++slashcount;
            if (slashcount > 1)
                support_directory[i + 1] = 0;
        }
    }
    if (slashcount != 3)
        return false;

    std::string exe_name = process_path.substr(process_path.find_last_of("/\\"));
    bool is64bit = exe_name.find("_x64") != std::string::npos;
    if (is64bit)
        strcat_s(support_directory, "Support64");
    else
        strcat_s(support_directory, "Support");

    SetCurrentDirectory(support_directory);

    char buffer[buffer_size];
    std::memset(buffer, 0, buffer_size);
    for (int i = 0; i < command_line.size(); ++i) {
        strcat_s(buffer, " ");
        strcat_s(buffer, command_line[i].c_str());
    }

    if (!CreateProcess(process_path.c_str(), // Module name
        buffer,                 // Command line
        NULL,                   // Process handle not inheritable
        NULL,                   // Thread handle not inheritable
        FALSE,                  // Set handle inheritance to FALSE
        0,                      // No creation flags
        NULL,                   // Use parent's environment block
        NULL,                   // Use parent's starting directory
        &process.si_,           // Pointer to STARTUPINFO structure
        &process.pi_)          // Pointer to PROCESS_INFORMATION structure
        )
    {
        SetCurrentDirectory(current_directory);
        return uint64_t(0);
    }

    windows_processes.push_back(process);
    SetCurrentDirectory(current_directory);
    Sleep(1000);

    return static_cast<uint64_t>(process.pi_.dwProcessId);
}



// #include "sc2api.pb.h"
// #include "sc2api.pb.cc"

#define WIRETYPE_VARINT            0
#define WIRETYPE_FIXED64           1
#define WIRETYPE_LENGTH_DELIMITED  2
#define WIRETYPE_START_GROUP       3
#define WIRETYPE_END_GROUP         4
#define WIRETYPE_FIXED32           5

static void
WriteVarint32ToArray(u8 **bytes, u32 value)
{
  *bytes+=v32tos(value,*bytes);
}

static void
WriteTagToArray(u8 **bytes, u32 type, u32 name)
{
  WriteVarint32ToArray(bytes,MOCK_TAG(type,name));
}

static void
WriteMessageToArray(u8 **bytes, u32 name, u32 size, void *data)
{
  WriteTagToArray(bytes, name, WIRETYPE_LENGTH_DELIMITED);
  WriteVarint32ToArray(bytes, size);
}


KLARK_FUNC StrA
NewFormatExtendedA(rx_i32 FormatLen, ConstStrA FormatStr, va_list Args, rx_i32 *Len)
{ rx_c8 *R = HeapAllocA(true,FormatLen + 0x100);
  rx_i32 L = vsprintf_s(R,FormatLen+0x100,FormatStr,Args);
  if(Len) *Len = L;
  return(R);
}

// TODO(RJ): UNSAFE CAST
KLARK_FUNC StrA
NewFormatA(ConstStrA Format, ...)
{ va_list Args;
  va_start(Args, Format);
  rx_c8 *R = NewFormatExtendedA((rx_i32) strlen(Format), Format,Args,NULL);
  va_end(Args);
  return (R);
}


enum
{ sc2_terran,
  sc2_zerg,
  sc2_protoss,
  sc2_random
};

typedef struct StrPair
{ ConstStrA A;
  ConstStrA B;
} StrPair;

typedef struct
{ i32        ThreadCount;        // 0-max_thread_count
  i32        SocketTimeout;      // ms
  i32        SocketNodelay;      // 0-1
  StrPair    SocketOptions[0x08];
  ConstStrA  SocketAddress;
  i32        SocketPort;
  i32        RequestTimeout;     // ms
} sc2_ctx;

static i32
write_fld_tag(u8 **cur, u32 name, u32 type)
{ i32 r=0;
  *cur += r=v32tos(MOCK_TAG(name,type), *cur);
  return(r);
}

static i32
write_string(u8 **cur, u32 name, u32 size, void *data)
{ i32 r = 0;
  r+=write_fld_tag(cur,2,name);
  *cur += r+=v32tos(size,*cur);
  memcpy(*cur,data,size);
  *cur += size;
  return(r);
}

static int
sc2_socket_connect_handler(
  const struct mg_connection *, void *)
{
  TRACE_I(".");
  return 0; // keep the connection alive
}

static void
sc2_socket_ready_handler(
  struct mg_connection *, void *)
{
  TRACE_I(".");
}


static int
sc2_socket_data_handler(
  struct mg_connection *, int bits, char *data, size_t size, void *)
{
  u32 r = 0;
  v32toi(&r, (u8 *)data);
  TRACE_I(".");
  __debugbreak();
  return 1; // keep the connection alive
}

static void
sc2_socket_close_handler(const struct mg_connection *, void *)
{
  TRACE_I(".");
  __debugbreak();
}

static void
CiwConnClosedRoutine(const mg_connection* Conn, void *UserData)
{
  TRACE_I(".");

  __debugbreak();
}



// int (*websocket_data)(struct mg_connection *,
//                       int bits,
//                       char *data,
//                       size_t data_len);

static i32  WebsocketDataCallback(
  struct mg_connection *, int bits, char *data, size_t data_len)
{
  __debugbreak();
  return 1;
}

void ZenMain(int,char**)
{
  u8 S[5]={0};
  u32 T=0,B=0,I=0;
  I = v32tos(T=0xffffff,S);
  I = v32toi(&B,S);
  Assert(T==B);
  (void) B;
  (void) I;


  ConstStrA MapName = "W:\\cpp\\_extern\\s2client-api\\maps\\Example\\MarineMicro.SC2Map";
  AssertW(GetFileAttributesA(MapName) != INVALID_FILE_ATTRIBUTES, L"File Does Not Exist");

  // CreateGame(MapName, FALSE);


  sc2_ctx Context = {};

  Context.ThreadCount    = 4;
  Context.SocketTimeout  = 120000;
  Context.SocketNodelay  = 1;
  Context.SocketAddress  = "127.0.0.1";
  Context.SocketPort     = 5679;
  Context.RequestTimeout = 100000;

  // TODO(RJ): REMOVE UNSAFE USE OF FORMAT A
  // Context.SocketOptions[0x00] = {"listening_ports",      FormatA("%i", Context.SocketPort)};
  Context.SocketOptions[0x00] = {"request_timeout_ms",   NewFormatA("%i", Context.RequestTimeout)};
  Context.SocketOptions[0x01] = {"websocket_timeout_ms", NewFormatA("%i", Context.SocketTimeout)};
  Context.SocketOptions[0x02] = {"num_threads",          NewFormatA("%i", Context.ThreadCount)};
  Context.SocketOptions[0x03] = {"tcp_nodelay",          NewFormatA("%i", Context.SocketNodelay)};


   std::vector<std::string> cl = {
      "-listen",       Context.SocketAddress,
      "-port",         "5679"
    };

  ConstStrA ProcessPath = "G:\\StarCraft II\\Versions\\Base89165\\SC2_x64.exe";

  StartProcess(ProcessPath, cl);

  // mg_set_websocket_handler(CiwCtx, "/sc2api",
  //   sc2_socket_connect_handler,
  //   sc2_socket_ready_handler,
  //   sc2_socket_data_handler,
  //   sc2_socket_close_handler, &Context);

  // ZenSystemProcess Process;
  // if(!ZenLaunchSystemProcess(&Process, "G:\\StarCraft II\\StarCraft II.exe", ".", NewFormatA(
  //     " "
  //     "-listen %s"         " "
  //     "-port %i"           " "
  //     "-displayMode 0"     " "
  //     "-dataVersion 0"     " "
  //     "-windowwidth 1080"  " "
  //     "-windowheight 720"  " "
  //     "-windowx 0"         " "
  //     "-windowy 0"         " ", Context.SocketAddress, Context.SocketPort)))
  // { TRACE_E("Error");
  //   __debugbreak();
  // }

  // ConnectToStarcraft2();

  mg_callbacks SocketCallbacks = {};
  SocketCallbacks.websocket_data = WebsocketDataCallback;

  mg_start(&SocketCallbacks, NULL, (ConstStrA *) Context.SocketOptions);
  char SocketErrorBuffer[0x100] = { 0 };
  i32 ConnRetryCounter = 0;
  mg_connection *Connection = NULL;
  do
  { TRACE_I("Attempting Connection %s:%i, Attempt %i",Widen(-1,Context.SocketAddress),Context.SocketPort,ConnRetryCounter);

    Connection = mg_connect_websocket_client(
      Context.SocketAddress, Context.SocketPort, FALSE, SocketErrorBuffer, sizeof(SocketErrorBuffer),
      "/sc2api", NULL, OnSocketDataReceived, CiwConnClosedRoutine, 0x00);

    if(!Connection)
    { ConnRetryCounter ++;
    }
  } while(!Connection);

  // RequestAvailableMaps(Connection);
  RequestCreateGame(Connection);

  ZenInitialize({}, L"zen");

  ZenWindow *Window = ZenGetActiveWindow();
  (void) Window;

  while(! Zen.Quitted)
  {

    if(Ready)
    {
      SC2APIProtocol::Request request={};
      SC2APIProtocol::RequestObservation *request_obs = request.mutable_observation();
      request_obs->set_disable_fog(true);
      SendRequest(Connection,request);

      Sleep(200);
    }

    ZenTick();
  }
}

