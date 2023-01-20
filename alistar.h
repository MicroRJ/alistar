#if !defined(ALISTAR_HEADER)

#define ALISTAR_HEADER
#include "stretchy_buffer.h"
#include "ali.c"

#define ALISTAR_PARSE_FUNCTION static

// Note: common types, these come from zen:: in my case ...
typedef i32x2 Point2DI, Size2DI;
typedef f32x2 Point2D;
typedef f32x3 Point3D;
typedef i32x4 RectangleI;

// Note: you must define this function for your own backend
static int
AlistarSendRequest(struct AlistarContext *, xvalue *);


static int
AlistarSendPayload(struct AlistarContext *, int tag, xvalue *);


// Todo: make enums
#define NORACE  0
#define TERRAN  1
#define ZERG    2
#define PROTOSS 3
#define RANDOM  4

#define AI_BUILD_RANDOM  1
#define AI_BUILD_RUSH    2
#define AI_BUILD_TIMING  3
#define AI_BUILD_POWER   4
#define AI_BUILD_MACRO   5
#define AI_BUILD_AIR     6

#define PARTICIPANT 1
#define COMPUTER    2
#define OBSERVER    3

#define STATUS_LAUNCHED  1 // Game has been launch and is not yet doing anything.
#define STATUS_INIT_GAME 2 // Create game has been called, and the host is awaiting players.
#define STATUS_IN_GAME   3 // In a single or multiplayer game.
#define STATUS_IN_REPLAY 4 // In a replay.
#define STATUS_ENDED     5 // Game has ended, can still request game info, but ready for a new game.
#define STATUS_QUIT      6 // Application is shutting down.
#define STATUS_UNKNOWN   9

#define CREATE_ERROR_MISSING_MAP             1
#define CREATE_ERROR_INVALID_MAP_PATH        2
#define CREATE_ERROR_INVALID_MAP_DATA        3
#define CREATE_ERROR_INVALID_MAP_NAME        4
#define CREATE_ERROR_INVALID_MAP_HANDLE      5
#define CREATE_ERROR_MISSING_PLAYER_SETUP    6
#define CREATE_ERROR_INVALID_PLAYER_SETUP    7
#define CREATE_ERROR_MULTIPLAYER_UNSUPPORTED 8

#define JOIN_ERROR_MISSING_PARTICIPATION       1
#define JOIN_ERROR_INVALID_OBSERVED_PLAYER_ID  2
#define JOIN_ERROR_MISSING_OPTIONS             3
#define JOIN_ERROR_MISSING_PORTS               4
#define JOIN_ERROR_GAME_FULL                   5
#define JOIN_ERROR_LAUNCH_ERROR                6
#define JOIN_ERROR_FEATURE_UNSUPPORTED         7
#define JOIN_ERROR_NO_SPACE_FOR_USER           8
#define JOIN_ERROR_MAP_DOES_NOT_EXIST          9
#define JOIN_ERROR_CANNOT_OPEN_MAP             0
#define JOIN_ERROR_CHECKSUM_ERROR              1
#define JOIN_ERROR_NETWORK_ERROR               2
#define JOIN_ERROR_OTHER_ERROR                 3

#define RESPONSE_TAG_CREATE_GAME      1
#define RESPONSE_TAG_JOIN_GAME        2
#define RESPONSE_TAG_RESTART_GAME     3
#define RESPONSE_TAG_START_REPLAY     4
#define RESPONSE_TAG_LEAVE_GAME       5
#define RESPONSE_TAG_QUICK_SAVE       6
#define RESPONSE_TAG_QUICK_LOAD       7
#define RESPONSE_TAG_QUIT             8
#define RESPONSE_TAG_GAME_INFO        9
#define RESPONSE_TAG_OBSERVATION      10
#define RESPONSE_TAG_ACTION           11
#define RESPONSE_TAG_OBSERVER_ACTION  21
#define RESPONSE_TAG_STEP             12
#define RESPONSE_TAG_DATA             13
#define RESPONSE_TAG_QUERY            14
#define RESPONSE_TAG_SAVE_REPLAY      15
#define RESPONSE_TAG_REPLAY_INFO      16
#define RESPONSE_TAG_AVAILABLE_MAPS   17
#define RESPONSE_TAG_SAVE_MAP         18
#define RESPONSE_TAG_MAP_COMMAND      22
#define RESPONSE_TAG_PING             19
#define RESPONSE_TAG_DEBUG            20
#define RESPONSE_TAG_CONNECTION_ENDED 771

#define SELECT_SELECT       1 // Equivalent to normal click. Changes selection to unit.
#define SELECT_TOGGLE       2 // Equivalent to shift+click. Toggle selection of unit.
#define SELECT_ALL_TYPE     3 // Equivalent to control+click. Selects all units of a given type.
#define SELECT_ADD_ALL_TYPE 4 // Equivalent to shift+control+click. Selects all units of a given type.




typedef struct ImageData
{ unsigned int   bits_per_pixel;
  Size2DI        size;
  unsigned char *bytes;
} ImageData;

// Action Types ...
typedef enum Alert
{ AlertError = 3,
  AddOnComplete = 4,
  BuildingComplete = 5,
  BuildingUnderAttack = 6,
  LarvaHatched = 7,
  MergeComplete = 8,
  MineralsExhausted = 9,
  MorphComplete = 10,
  MothershipComplete = 11,
  MULEExpired = 12,
  NuclearLaunchDetected = 1,
  NukeComplete = 13,
  NydusWormDetected = 2,
  ResearchComplete = 14,
  TrainError = 15,
  TrainUnitComplete = 16,
  TrainWorkerComplete = 17,
  TransformationComplete = 18,
  UnitUnderAttack = 19,
  UpgradeComplete = 20,
  VespeneExhausted = 21,
  WarpInComplete = 22,
} Alert;

#if defined(CODE) || defined(NAME)
# error
#endif

#define ALISTAR_TO_STRING(A) #A
#define ALISTAR_STRING_CONCAT(A,B) A##B

typedef enum ActionResult
{
#define ALISTAR_ACTION_RESULT(CODE,NAME) ALISTAR_STRING_CONCAT(ActionResult,NAME)=CODE,
# include "alistar.h"
#undef ALISTAR_ACTION_RESULT
} ActionResult;

typedef struct ActionRawUnitCommand
{ int32_t    ability_id;
  union
  { Point2D  target_world_space_pos; // worldspace
    uint64_t target_unit_tag;
  };
  uint64_t  *unit_tags;
  uint32_t   is_target_unit_tag;
  int        queue_command;
} ActionRawUnitCommand;
typedef struct ActionRawCameraMove
{  Point2D loca; // center, worldspace
} ActionRawCameraMove;
typedef struct ActionRawToggleAutocast
{ int32_t   ability_id;
  uint64_t *unit_tags;
} ActionRawToggleAutocast;
typedef struct ActionRaw
{ int32_t type;
  union
  { ActionRawUnitCommand    unit_command;
    ActionRawCameraMove     camera_move;
    ActionRawToggleAutocast toggle_autocast;
  };
} ActionRaw;
typedef struct ActionSpatialUnitCommand
{ unsigned int ability_id;
  Point2DI       coord; // screen or minimap space
  unsigned     is_minimap_coord: 1;
  unsigned     is_queue_command: 1;
} ActionSpatialUnitCommand;
typedef struct ActionSpatialCameraMove
{ Point2DI       minimap;
} ActionSpatialCameraMove;
typedef struct ActionSpatialUnitSelectionPoint
{ Point2DI screen;
  int    select;
} ActionSpatialUnitSelectionPoint;
typedef struct ActionSpatialUnitSelectionRect
{ RectangleI   screen[0x10];
  unsigned int length;
  unsigned int is_add: 1; // shift+drag
} ActionSpatialUnitSelectionRect;
typedef struct ActionSpatial
{ int type; // 1,2,3,4
  union
  { ActionSpatialUnitCommand        unit_command;
    ActionSpatialCameraMove         camera_move;
    ActionSpatialUnitSelectionPoint unit_selection_point;
    ActionSpatialUnitSelectionRect  unit_selection_rect;
  };
} ActionSpatial;
typedef enum
{ CHANNEL_BROADCAST = 1,
  CHANNEL_TEAM      = 2,
} ChannelType;
typedef struct ActionChat
{ ChannelType   channel;
  char         *message;
} ActionChat;
typedef struct Action
{ ActionRaw     action_raw;

#if 0
  int           action_feature_layer;
  int           action_render;
  int           action_ui;
#endif
  ActionChat    action_chat;
  uint32_t      game_loop;
} Action;
typedef struct ErrorAction
{ uint64_t     unit_tag;                             // Only populated when using raw interface.
  uint64_t     ability_id;
  ActionResult result;
} ErrorAction;
typedef struct PlayerInfo
{ int diff;
  int build;
  int race_r; // requested race
  int race_a; // actual race
  int type;
  unsigned int iden;
  char *name;
} PlayerInfo;
typedef struct MapName
{ unsigned    is_local:1;
  char       *string;
} MapName;
typedef enum GameResult
{ Victory   = 1,
  Defeat    = 2,
  Tie       = 3,
  Undecided = 4,
} GameResult;
typedef struct PlayerResult
{ uint32_t   player_id;
  GameResult result;
} PlayerResult;
typedef struct PlayerCommon
{ uint32_t player_id;
  uint32_t minerals;
  uint32_t vespene;
  uint32_t food_cap;
  uint32_t food_used;
  uint32_t food_army;
  uint32_t food_workers;
  uint32_t idle_worker_count;
  uint32_t army_count;
  uint32_t warp_gate_count;
  uint32_t larva_count;
} PlayerCommon;
typedef struct ChatReceived
{ uint32_t player_id;
  char    *message;
} ChatReceived;
typedef struct MapState
{ ImageData visibility; // optional:1 1 byte visibility layer.
  ImageData creep;      // optional:2 1 bit creep layer.
} MapState;
typedef struct UnitOrder
{ uint32_t ability_id;               // optional:1
  union
  { uint64_t target_unit_tag;        // optional:3
    Point3D  target_world_space_pos; // optional:2
  };
  float    progress;                 // optional:4
  unsigned is_unit_tag: 1;           // conditional:[3,2] if 3
} UnitOrder;
typedef enum DisplayType
{ Visible      = 1,                            // Fully visible
  Snapshot     = 2,                           // Dimmed version of unit left behind after entering fog of war
  Hidden       = 3,                             // Fully hidden
  Placeholder  = 4,                        // Building that hasn't started construction.
} DisplayType;
typedef enum Alliance
{ Self    = 1,
  Ally    = 2,
  Neutral = 3,
  Enemy   = 4,
} Alliance;
typedef enum CloakState
{ CloakedUnknown  = 0,  // Under the fog, so unknown whether it's cloaked or not.
  Cloaked         = 1,
  CloakedDetected = 2,
  NotCloaked      = 3,
  CloakedAllied   = 4,
} CloakState;
typedef struct PassengerUnit
{ float      health;      // 2
  float      health_max;  // 3
  float      shield;      // 4
  float      shield_max;  // 7
  float      energy;      // 5
  float      energy_max;  // 8
  uint64_t   tag;         // 1
  uint32_t   unit_type;   // 6
} PassengerUnit;
typedef struct RallyTarget
{ Point2D   point; // 1   Will always be filled.
  uint64_t  tag;   // 2   Only if it's targeting a unit.
} RallyTarget;
typedef struct Unit
{
  float facing;            // optional:7
  float radius;            // optional:8
  float build_progress;    // optional:9   Range: [0.0, 1.0]
  float detect_range;      // optional:31
  float radar_range;       // optional:32
  float health;            // optional:14  (not populated for snapshots)
  float health_max;        // optional:15  (not populated for snapshots)
  float shield;            // optional:16  (not populated for snapshots)
  float shield_max;        // optional:36  (not populated for snapshots)
  float energy;            // optional:17  (not populated for snapshots)
  float energy_max;        // optional:37  (not populated for snapshots)
  float weapon_cooldown;   // optional:30  (not populated for enemies)

  uint64_t tag;                  // optional:3  Unique identifier for a unit
  uint64_t add_on_tag;           // optional:23 (not populated for enemies)
  uint64_t engaged_target_tag;   // optional:34 (not populated for enemies)

  uint32_t unit_type;            // optional:4

  int32_t attack_upgrade_level;  // optional:40;
  int32_t armor_upgrade_level;   // optional:41;
  int32_t shield_upgrade_level;  // optional:42;
  int32_t mineral_contents;      // optional:18  (not populated for snapshots)
  int32_t vespene_contents;      // optional:19  (not populated for snapshots)
  int32_t cargo_space_taken;     // optional:25; (not populated for enemies)
  int32_t cargo_space_max;       // optional:26; (not populated for enemies)
  int32_t assigned_harvesters;   // optional:28; (not populated for enemies)
  int32_t ideal_harvesters;      // optional:29; (not populated for enemies)
  int32_t buff_duration_remain;  // optional:43; (not populated for enemies)  How long a buff or unit is still around (eg mule, broodling, chronoboost).
  int32_t buff_duration_max;     // optional:44; (not populated for enemies)  How long the buff or unit is still around (eg mule, broodling, chronoboost).
  int32_t owner;                 // optional:5

  int is_flying;        // optional:20 (not populated for snapshots)
  int is_burrowed;      // optional:21 (not populated for snapshots)
  int is_hallucination; // optional:38 (not populated for snapshots)
  int is_selected;      // optional:11;
  int is_on_screen;     // optional:12;  Visible and within the camera frustrum.
  int is_blip;          // optional:13;  Detected by sensor tower
  int is_powered;       // optional:35;
  int is_active;        // optional:39;  Building is training/researching (ie animated).

  CloakState       cloak;          // optional:10
  DisplayType      display_type;   // optional:1
  Alliance         alliance;       // optional:2

  Point3D          pos;            // optional:6

  uint32_t       * buff_ids;       // optional:27
  UnitOrder      * orders;         // repeated:22 (not populated for enemies)
  PassengerUnit  * passengers;     // repeated:24 (not populated for enemies)
  RallyTarget    * rally_targets;  // repeated:45 (not populated for enemies)
} Unit;
typedef struct ObservationRaw
{ // PlayerRaw player;  // optional:1;
  Unit     *units;      // repeated:2;
  MapState  map_state;  // optional:3; Fog of war, creep and so on. Board stuff that changes per frame.
  // Event event;       // optional:4;
  // Effect effects;    // repeated:5;
  // RadarRing radar;   // repeated:6;
} ObservationRaw;
typedef struct Observation
{ uint32_t        game_loop;
  PlayerCommon    player_common;
  Alert          *alerts;
  ObservationRaw  raw_data;
} Observation;
typedef struct ResponseObservation
{ Action       *actions; // array
  ErrorAction  *error_actions; // array
  Observation   observation;
  PlayerResult *player_results; // array
  ChatReceived *chats; // array
} ResponseObservation;
typedef struct ResponseAction
{ ActionResult *results; // array
} ResponseAction;
typedef struct StartRaw
{ Size2DI     map_size;
  ImageData   pathing_grid;
  ImageData   terrain_height;
  ImageData   placement_grid;
  RectangleI  playable_area;
  Point2D    *start_locations;
} StartRaw;
typedef struct ResponseGameInfo
{ MapName      *maps;
  char       * *mods;
  PlayerInfo   *players;
  StartRaw      start_raw;
} ResponseGameInfo;
typedef struct ResponseCreateGame
{ int   errbit;
  char *errstr;
} ResponseCreateGame;
typedef struct ResponseJoinGame
{ unsigned int player;
  int          errbit;
  char        *errstr;
} ResponseJoinGame;
typedef struct ResponseAvailableMaps
{ MapName *maps;
} ResponseAvailableMaps;
typedef struct Response
{
union
{ ResponseGameInfo      GameInfo;
  ResponseCreateGame    CreateGame;
  ResponseJoinGame      JoinGame;
  ResponseAvailableMaps AvailableMaps;
  ResponseObservation   Observation;
  ResponseAction        Action;
};
  int Status;
  int Type;
  unsigned int Id;
  char *Error;
} Response;

typedef struct RequestAction
{ Action *actions;
} RequestAction;

typedef struct Request
{ union
  { RequestAction action;
  };
} Request;

typedef struct AlistarQueue
{ int64_t   CountMax;
  int64_t   CountMin;
  Response  *Array;
  void      *Mutex;
  void      *Event;
} AlistarQueue;

typedef struct AlistarContext
{
  ZenSystemProcess  proc;
  mg_connection   * conn;
  const char      * conn_addr;
  int32_t           conn_port;
  AlistarQueue      conn_queue;
  Response          last_response;
  int32_t           requested_obs;
  int32_t           ready_for_obs;
  struct
  { int32_t   loop;
    int32_t   worker_count;
    Unit     *my_worker_units;
  } Game;
} AlistarContext;


static int
RequestObservation(AlistarContext *ctx, int disable_fog, int game_loop);
static i32
RequestGameInfo(AlistarContext *ctx);
static i32
RequestAvailableMaps(AlistarContext *ctx);
static i32
RequestCreateGame(AlistarContext *ctx);
static i32
RequestJoinGameAsParticipant(AlistarContext *ctx, int race, const char *name);


ALISTAR_PARSE_FUNCTION void
ParsePoint2DI(xstate *read, Point2DI *val);
ALISTAR_PARSE_FUNCTION void
ParsePoint2D(xstate *read, Point2D *val);
ALISTAR_PARSE_FUNCTION void
ParsePoint3D(xstate *read, Point3D *val);
ALISTAR_PARSE_FUNCTION void
ParseSize2DI(xstate *read, Size2DI *val);
ALISTAR_PARSE_FUNCTION void
ParseRectangleI(xstate *read, RectangleI *val);
ALISTAR_PARSE_FUNCTION void
ParseImageData(xstate *read, ImageData *val);
ALISTAR_PARSE_FUNCTION void
ParsePlayerInfo(xstate *read, PlayerInfo *val);
ALISTAR_PARSE_FUNCTION void
ParseResponseCreateGame(xstate *read, ResponseCreateGame *val);
ALISTAR_PARSE_FUNCTION void
ParseResponseJoinGame(xstate *read, ResponseJoinGame *val);
ALISTAR_PARSE_FUNCTION void
ParsePlayerCommon(xstate *read, PlayerCommon *val);
ALISTAR_PARSE_FUNCTION void
ParseStartRaw(xstate *read, StartRaw *val);
ALISTAR_PARSE_FUNCTION void
ParseResponseGameInfo(xstate *read, ResponseGameInfo *val);
ALISTAR_PARSE_FUNCTION void
ParseResponseAvailableMaps(xstate *read, ResponseAvailableMaps *val);
ALISTAR_PARSE_FUNCTION void
ParseActionChat(xstate *read, ActionChat *val);
ALISTAR_PARSE_FUNCTION void
ParseAction(xstate *read, Action *val);
ALISTAR_PARSE_FUNCTION void
ParseErrorAction(xstate *read, ErrorAction *val);
ALISTAR_PARSE_FUNCTION void
ParseChatReceived(xstate *read, ChatReceived *val);
ALISTAR_PARSE_FUNCTION void
ParseUnit(xstate *read, Unit *val);
ALISTAR_PARSE_FUNCTION void
ParseMapState(xstate *read, MapState *val);
ALISTAR_PARSE_FUNCTION void
ParseObservationRaw(xstate *read, ObservationRaw *val);
ALISTAR_PARSE_FUNCTION void
ParseObservation(xstate *read, Observation *val);
ALISTAR_PARSE_FUNCTION void
ParsePlayerResult(xstate *read, PlayerResult *val);
ALISTAR_PARSE_FUNCTION void
ParseResponseObservation(xstate *read, ResponseObservation *val);
ALISTAR_PARSE_FUNCTION void
ParseResponse(Response *info, size_t size, const void *data);

static const wchar_t *
AlistarActionResultW(ActionResult result);






static xvalue
CreateParticipantPlayerSetup(const char *name)
{ xvalue req={ali_msg_type};
  AddVarint32Value(&req,/*tag: */1, /*value: */ PARTICIPANT);
  AddStringValue(&req,/*tag: */4, /*value: */ name);
  return req;
}
static xvalue
CreateComputerPlayerSetup(int race, int diff, int build, const char *name)
{ // Computer=2
  xvalue req={ali_msg_type};
  AddVarint32Value(&req, /*tag*/1, COMPUTER );
  AddEnumValue(&req,   /*tag*/2,     race );
  AddEnumValue(&req,   /*tag*/3,     diff );
  AddStringValue(&req, /*tag*/4,     name );
  AddEnumValue(&req,   /*tag*/5,    build );
  return req;
}
static xvalue
CreateDefaultInterfaceOptions()
{ xvalue req={ali_msg_type};
  AddBoolValue(&req, /*tag*/1, TRUE); // raw: 1
  AddBoolValue(&req, /*tag*/8, TRUE); // show_placeholders: 8
  return req;
}
static i32
RequestGameInfo(AlistarContext *ctx)
{ xvalue pay={ali_msg_type};
  return AlistarSendPayload(ctx,9,&pay);
}
static i32
RequestAvailableMaps(AlistarContext *ctx)
{ xvalue pay={ali_msg_type};
  return AlistarSendPayload(ctx,17,&pay);
}
static i32
RequestCreateGame(AlistarContext *ctx)
{ xvalue pay={ali_msg_type};
  AddStringValue(&pay, /*tag*/2, "16-Bit LE"); // Battlenet map name
  AddBoolValue(&pay,   /*tag*/6, true);       // Realtime?
  AddValue(&pay,       /*tag*/3, CreateParticipantPlayerSetup(/*name*/"RoyJacobs"));
  AddValue(&pay,       /*tag*/3, CreateComputerPlayerSetup(ZERG,7,5, /*name*/ "DakotaBrown"));
  return AlistarSendPayload(ctx,1,&pay);
}
static i32
RequestJoinGameAsParticipant(AlistarContext *ctx, int race, const char *name)
{ xvalue pay={ali_msg_type};
  AddEnumValue(&pay,  /* tag */ 1, race);
  AddStringValue(&pay,/* tag */ 7, name);
  AddValue(&pay,      /* tag */ 3, CreateDefaultInterfaceOptions()); // 3:Options
  return AlistarSendPayload(ctx,2,&pay);
}
static int
RequestObservation(AlistarContext *ctx, int disable_fog, int game_loop)
{ xvalue pay={ali_msg_type};
  AddBoolValue(&pay,  /* tag */ 1, disable_fog);
  // AddVarint32Value(&pay,/* tag */ 2, game_loop);
  return AlistarSendPayload(ctx,10,&pay);
}
// Parsing functions
ALISTAR_PARSE_FUNCTION void
ParsePoint2DI(xstate *read, Point2DI *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1: GetVarintValue(read,(unsigned int*)&val->x); break;// TODO(RJ):
    case 2: GetVarintValue(read,(unsigned int*)&val->y); break;// TODO(RJ):
  }
}
ALISTAR_PARSE_FUNCTION void
ParsePoint2D(xstate *read, Point2D *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1: GetFloat32Value(read,&val->x); break;
    case 2: GetFloat32Value(read,&val->y); break;
  }
}
ALISTAR_PARSE_FUNCTION void
ParsePoint3D(xstate *read, Point3D *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1: GetFloat32Value(read,&val->x); break;// TODO(RJ):
    case 2: GetFloat32Value(read,&val->y); break;// TODO(RJ):
    case 3: GetFloat32Value(read,&val->z); break;// TODO(RJ):
  }
}
ALISTAR_PARSE_FUNCTION void
ParseSize2DI(xstate *read, Size2DI *val)
{ return ParsePoint2DI(read,val);
}
ALISTAR_PARSE_FUNCTION void
ParseRectangleI(xstate *read, RectangleI *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1: ParsePoint2DI(read,&val->min); break;
    case 2: ParsePoint2DI(read,&val->max); break;
  }
}
ALISTAR_PARSE_FUNCTION void
ParseImageData(xstate *read, ImageData *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1: GetVarintValue(read,&val->bits_per_pixel); break;
    case 2: ParseSize2DI(read,  &val->size);    break;
    case 3: GetBytesValue(read, &val->bytes);  break;
  }
}
// Mainly for debugging reasons
ALISTAR_PARSE_FUNCTION void
ParseActionChat(xstate *read, ActionChat *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1: GetEnumValue(read,&val->channel);   break;
    case 2: GetStringValue(read,&val->message); break;
  }
}
ALISTAR_PARSE_FUNCTION void
ParseActionRawUnitCommand(xstate *read, ActionRawUnitCommand *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1: GetVarintValue(read, &val->ability_id);             break;
    case 2: ParsePoint2D(read, &val->target_world_space_pos);   break;
    case 3: GetVarint64Value(read, &val->target_unit_tag);      break;
    case 4: GetVarint64Value(read, sb_add(val->unit_tags,1));   break;
    case 5: GetVarintValue(read, &val->queue_command);          break;
  }
}
ALISTAR_PARSE_FUNCTION void
ParseActionRaw(xstate *read, ActionRaw *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1: ParseActionRawUnitCommand(read, &val->unit_command); break;
  }
}
ALISTAR_PARSE_FUNCTION void
ParseAction(xstate *read, Action *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1: ParseActionRaw(read, &val->action_raw); break;
    // case 2:  ParseActionFeatureLayer(read, &val->action_feature_layer); break;
    // case 3:  ParseActionRender(read, &val->action_render); break;
    // case 4:  ParseActionUI(read, &val->action_ui); break;
    case 6: ParseActionChat(read, &val->action_chat); break;
    case 7: GetVarintValue(read, &val->game_loop);    break;
  }
}
ALISTAR_PARSE_FUNCTION void
ParseRequestAction(xstate *read, RequestAction *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1: ParseAction(read,sb_add(val->actions,1)); break;
  }
}
ALISTAR_PARSE_FUNCTION void
ParseRequest(xstate *read, Request *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 11: ParseRequestAction(read,&val->action); break;
  }
}
// Player
ALISTAR_PARSE_FUNCTION void
ParsePlayerInfo(xstate *read, PlayerInfo *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1: GetVarintValue(read,&val->iden);   break;
    case 2: GetEnumValue(read,&val->type);     break;
    case 3: GetEnumValue(read,&val->race_r);   break;
    case 4: GetEnumValue(read,&val->race_a);   break;
    case 5: GetEnumValue(read,&val->diff);     break;
    case 7: GetEnumValue(read,&val->build);    break;
    case 6: GetStringValue(read,&val->name);   break;
  }
}
ALISTAR_PARSE_FUNCTION void
ParseResponseCreateGame(xstate *read, ResponseCreateGame *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1: GetEnumValue(read,&val->errbit);   break;
    case 2: GetStringValue(read,&val->errstr); break;
  }
}
ALISTAR_PARSE_FUNCTION void
ParseResponseJoinGame(xstate *read, ResponseJoinGame *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1: GetVarintValue(read,&val->player); break;
    case 2: GetEnumValue(read,&val->errbit);   break;
    case 3: GetStringValue(read,&val->errstr); break;
  }
}
ALISTAR_PARSE_FUNCTION void
ParsePlayerCommon(xstate *read, PlayerCommon *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1:  GetVarintValue(read,&val->player_id);        break;
    case 2:  GetVarintValue(read,&val->minerals);         break;
    case 3:  GetVarintValue(read,&val->vespene);          break;
    case 4:  GetVarintValue(read,&val->food_cap);         break;
    case 5:  GetVarintValue(read,&val->food_used);        break;
    case 6:  GetVarintValue(read,&val->food_army);        break;
    case 7:  GetVarintValue(read,&val->food_workers);     break;
    case 8:  GetVarintValue(read,&val->idle_worker_count);break;
    case 9:  GetVarintValue(read,&val->army_count);       break;
    case 10: GetVarintValue(read,&val->warp_gate_count);  break;
    case 11: GetVarintValue(read,&val->larva_count);      break;
  }
}
ALISTAR_PARSE_FUNCTION void
ParseStartRaw(xstate *read, StartRaw *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1: ParseSize2DI(read,    &val->map_size);               break;
    case 2: ParseImageData(read,  &val->pathing_grid);           break;
    case 3: ParseImageData(read,  &val->terrain_height);         break;
    case 4: ParseImageData(read,  &val->placement_grid);         break;
    case 5: ParseRectangleI(read, &val->playable_area);          break;
    case 6: ParsePoint2D(read, sb_add(val->start_locations, 1)); break;
  }
}
ALISTAR_PARSE_FUNCTION void
ParseResponseGameInfo(xstate *read, ResponseGameInfo *val)
{ MapName *map;
  ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1:
    { map=sb_add(val->maps,1);
      map->is_local=0;

      GetStringValue(read,&map->string);
    } break;
    break;
    case 2:
    { map=sb_add(val->maps,1);
      map->is_local=1;

      GetStringValue(read,&map->string);
    } break;
    break;
    case 3: ParsePlayerInfo(read,sb_add(val->players,1)); break;
    case 4: ParseStartRaw(read,&val->start_raw);          break;
    case 6: GetStringValue(read,sb_add(val->mods,1));     break;
  }
}
ALISTAR_PARSE_FUNCTION void
ParseResponseAvailableMaps(xstate *read, ResponseAvailableMaps *val)
{ MapName *map;
  ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1:
    { map=sb_add(val->maps,1);
      map->is_local=1;

      GetStringValue(read,&map->string);
    } break;
    case 2:
    { map=sb_add(val->maps,1);
      map->is_local=0;

      GetStringValue(read,&map->string);
    } break;
  }
}
ALISTAR_PARSE_FUNCTION void
ParseErrorAction(xstate *read, ErrorAction *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1: GetVarint64Value(read,&val->unit_tag);   break;
    case 2: GetVarint64Value(read,&val->ability_id); break;
    case 3: GetEnumValue(read,&val->result);         break;
  }
}
ALISTAR_PARSE_FUNCTION void
ParseChatReceived(xstate *read, ChatReceived *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1: GetVarintValue(read,&val->player_id); break;
    case 2: GetStringValue(read,&val->message);   break;
  }
}
ALISTAR_PARSE_FUNCTION void
ParseUnitOrder(xstate *read, UnitOrder *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1: GetVarintValue(read,&val->ability_id); break;
    case 2: ParsePoint3D(read,&val->target_world_space_pos); break;
    case 3: GetVarint64Value(read,&val->target_unit_tag); break;
    case 4: GetFloat32Value(read,&val->progress); break;
  }
}
ALISTAR_PARSE_FUNCTION void
ParseUnit(xstate *read, Unit *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 7:  GetFloat32Value(read,&val->facing); break;
    case 8:  GetFloat32Value(read,&val->radius); break;
    case 9:  GetFloat32Value(read,&val->build_progress); break;
    case 31: GetFloat32Value(read,&val->detect_range); break;
    case 32: GetFloat32Value(read,&val->radar_range); break;
    case 14: GetFloat32Value(read,&val->health); break;
    case 15: GetFloat32Value(read,&val->health_max); break;
    case 16: GetFloat32Value(read,&val->shield); break;
    case 36: GetFloat32Value(read,&val->shield_max); break;
    case 17: GetFloat32Value(read,&val->energy); break;
    case 37: GetFloat32Value(read,&val->energy_max); break;
    case 30: GetFloat32Value(read,&val->weapon_cooldown); break;
    case 3:  GetVarint64Value(read,&val->tag); break;
    case 23: GetVarint64Value(read,&val->add_on_tag); break;
    case 34: GetVarint64Value(read,&val->engaged_target_tag); break;
    case 4:  GetVarintValue(read,&val->unit_type); break;
    case 40: GetVarintValue(read,&val->attack_upgrade_level); break;
    case 41: GetVarintValue(read,&val->armor_upgrade_level); break;
    case 42: GetVarintValue(read,&val->shield_upgrade_level); break;
    case 18: GetVarintValue(read,&val->mineral_contents); break;
    case 19: GetVarintValue(read,&val->vespene_contents); break;
    case 25: GetVarintValue(read,&val->cargo_space_taken); break;
    case 26: GetVarintValue(read,&val->cargo_space_max); break;
    case 28: GetVarintValue(read,&val->assigned_harvesters); break;
    case 29: GetVarintValue(read,&val->ideal_harvesters); break;
    case 43: GetVarintValue(read,&val->buff_duration_remain); break;
    case 44: GetVarintValue(read,&val->buff_duration_max); break;
    case 5:  GetVarintValue(read,&val->owner); break;
    case 20: GetVarintValue(read,&val->is_flying); break;
    case 21: GetVarintValue(read,&val->is_burrowed); break;
    case 38: GetVarintValue(read,&val->is_hallucination); break;
    case 11: GetVarintValue(read,&val->is_selected); break;
    case 12: GetVarintValue(read,&val->is_on_screen); break;
    case 13: GetVarintValue(read,&val->is_blip); break;
    case 35: GetVarintValue(read,&val->is_powered); break;
    case 39: GetVarintValue(read,&val->is_active); break;
    case 10: GetEnumValue(read,&val->cloak); break;
    case 1:  GetEnumValue(read,&val->display_type); break;
    case 2:  GetEnumValue(read,&val->alliance); break;
    case 6:  ParsePoint3D(read,&val->pos); break;
    case 27: GetVarintValue(read,sb_add(val->buff_ids,1)); break;
    case 22: ParseUnitOrder(read,sb_add(val->orders,  1)); break;
    // case 24: PassengerUnit  * passengers;
    // case 45: RallyTarget    * rally_targets;
  }
}
ALISTAR_PARSE_FUNCTION void
ParsePlayerResult(xstate *read, PlayerResult *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1: GetVarintValue(read,&val->player_id); break;
    case 2: GetEnumValue(read,&val->result); break;
  }
}
ALISTAR_PARSE_FUNCTION void
ParseMapState(xstate *read, MapState *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1: ParseImageData(read,&val->visibility); break;
    case 2: ParseImageData(read,&val->creep);      break;
  }
}
ALISTAR_PARSE_FUNCTION void
ParseObservationRaw(xstate *read, ObservationRaw *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 3: ParseMapState(read,&val->map_state);  break;
    case 2: ParseUnit(read,sb_add(val->units,1)); break;
  }
}
ALISTAR_PARSE_FUNCTION void
ParseObservation(xstate *read, Observation *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1: ParsePlayerCommon(read,&val->player_common); break;
    case 9: GetVarintValue(read,&val->game_loop);        break;
    case 5: ParseObservationRaw(read,&val->raw_data);    break;
  }
}
ALISTAR_PARSE_FUNCTION void
ParseResponseObservation(xstate *read, ResponseObservation *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1: ParseAction(read,sb_add(val->actions,1));             break;
    case 2: ParseErrorAction(read,sb_add(val->error_actions,1));  break;
    case 3: ParseObservation(read,&val->observation);             break;
    case 4: ParsePlayerResult(read,sb_add(val->player_results,1));break;
    case 5: ParseChatReceived(read,sb_add(val->chats,1));         break;
    default:
    { TRACE_W("unknown field: %i", GetTag(read));
    } break;
  }
}
ALISTAR_PARSE_FUNCTION void
ParseResponseAction(xstate *read, ResponseAction *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1:
    {
      ActionResult *result;
      result=sb_add(val->results,1);

      GetEnumValue(read,result);

      TRACE_I("Action Result #%s", AlistarActionResultW(*result));

    } break;
  }
}
ALISTAR_PARSE_FUNCTION void
ParseResponse(Response *info, size_t size, const void *data)
{ ZeroMemory(info,sizeof(*info));
  xstate read={};
  Apportion(&read,size,data);

  ForMessageField(&read)
  { case 99: GetEnumValue(&read,&info->Status);  break;
    case 98: GetStringValue(&read,&info->Error); break;
    case 97: GetVarintValue(&read,&info->Id);    break;
    case RESPONSE_TAG_CREATE_GAME:
    { ParseResponseCreateGame(&read,&info->CreateGame);
      info->Type=RESPONSE_TAG_CREATE_GAME;
    } break;
    case RESPONSE_TAG_JOIN_GAME:
    { ParseResponseJoinGame(&read,&info->JoinGame);
      info->Type=RESPONSE_TAG_JOIN_GAME;
    } break;
    case RESPONSE_TAG_GAME_INFO:
    { ParseResponseGameInfo(&read,&info->GameInfo);
      info->Type=RESPONSE_TAG_GAME_INFO;
    } break;
    case RESPONSE_TAG_AVAILABLE_MAPS:
    { ParseResponseAvailableMaps(&read,&info->AvailableMaps);
      info->Type=RESPONSE_TAG_AVAILABLE_MAPS;
    } break;
    case RESPONSE_TAG_OBSERVATION:
    { ParseResponseObservation(&read,&info->Observation);
      info->Type=RESPONSE_TAG_OBSERVATION;
    } break;
    case RESPONSE_TAG_ACTION:
    { ParseResponseAction(&read,&info->Action);
      info->Type=RESPONSE_TAG_ACTION;
    } break;
    default:
    { TRACE_W("unknown response: %i", GetTag(&read));
    } break;
  }
}
static int
AlistarRequestTrainSCV(AlistarContext *ctx, uint64_t unit_tag)
{
  xvalue raw_unit_command={ali_msg_type};
  AddVarint32Value(&raw_unit_command,/* tag */ 1, 524);
  AddVarint64Value(&raw_unit_command,/* tag */ 4, unit_tag);

  xvalue action_raw={ali_msg_type};
  AddValue(&action_raw,/* tag */1,raw_unit_command);

  xvalue action={ali_msg_type};
  AddValue(&action,/* tag */1,action_raw);

  xvalue request_action={ali_msg_type};
  AddValue(&request_action,/* tag */1,action);

  return AlistarSendPayload(ctx,11,&request_action);
}

static int
AlistarSendChat(AlistarContext *ctx, int32_t channel, const char *message)
{
  xvalue action_chat={ali_msg_type};
  AddEnumValue(&action_chat,/* tag */1,channel);
  AddStringValue(&action_chat,/* tag */2,message);

  xvalue action={ali_msg_type};
  AddValue(&action,/* tag */6,action_chat);

  xvalue request_action={ali_msg_type};
  AddValue(&request_action,/* tag */1,action);

  return AlistarSendPayload(ctx,11,&request_action);
}
static const wchar_t *
AlistarActionResultW(ActionResult result)
{
  switch(result)
  {
#define ALISTAR_ACTION_RESULT(CODE,NAME) case CODE: return L""ALISTAR_TO_STRING(NAME);
# include "alistar.h"
#undef ALISTAR_ACTION_RESULT
  }
  return L"internal error";
}
#elif defined(ALISTAR_ACTION_RESULT)
ALISTAR_ACTION_RESULT(1,   Success)
ALISTAR_ACTION_RESULT(2,   NotSupported)
ALISTAR_ACTION_RESULT(3,   Error)
ALISTAR_ACTION_RESULT(4,   CantQueueThatOrder)
ALISTAR_ACTION_RESULT(5,   Retry)
ALISTAR_ACTION_RESULT(6,   Cooldown)
ALISTAR_ACTION_RESULT(7,   QueueIsFull)
ALISTAR_ACTION_RESULT(8,   RallyQueueIsFull)
ALISTAR_ACTION_RESULT(9,   NotEnoughMinerals)
ALISTAR_ACTION_RESULT(10,  NotEnoughVespene)
ALISTAR_ACTION_RESULT(11,  NotEnoughTerrazine)
ALISTAR_ACTION_RESULT(12,  NotEnoughCustom)
ALISTAR_ACTION_RESULT(13,  NotEnoughFood)
ALISTAR_ACTION_RESULT(14,  FoodUsageImpossible)
ALISTAR_ACTION_RESULT(15,  NotEnoughLife)
ALISTAR_ACTION_RESULT(16,  NotEnoughShields)
ALISTAR_ACTION_RESULT(17,  NotEnoughEnergy)
ALISTAR_ACTION_RESULT(18,  LifeSuppressed)
ALISTAR_ACTION_RESULT(19,  ShieldsSuppressed)
ALISTAR_ACTION_RESULT(20,  EnergySuppressed)
ALISTAR_ACTION_RESULT(21,  NotEnoughCharges)
ALISTAR_ACTION_RESULT(22,  CantAddMoreCharges)
ALISTAR_ACTION_RESULT(23,  TooMuchMinerals)
ALISTAR_ACTION_RESULT(24,  TooMuchVespene)
ALISTAR_ACTION_RESULT(25,  TooMuchTerrazine)
ALISTAR_ACTION_RESULT(26,  TooMuchCustom)
ALISTAR_ACTION_RESULT(27,  TooMuchFood)
ALISTAR_ACTION_RESULT(28,  TooMuchLife)
ALISTAR_ACTION_RESULT(29,  TooMuchShields)
ALISTAR_ACTION_RESULT(30,  TooMuchEnergy)
ALISTAR_ACTION_RESULT(31,  MustTargetUnitWithLife)
ALISTAR_ACTION_RESULT(32,  MustTargetUnitWithShields)
ALISTAR_ACTION_RESULT(33,  MustTargetUnitWithEnergy)
ALISTAR_ACTION_RESULT(34,  CantTrade)
ALISTAR_ACTION_RESULT(35,  CantSpend)
ALISTAR_ACTION_RESULT(36,  CantTargetThatUnit)
ALISTAR_ACTION_RESULT(37,  CouldntAllocateUnit)
ALISTAR_ACTION_RESULT(38,  UnitCantMove)
ALISTAR_ACTION_RESULT(39,  TransportIsHoldingPosition)
ALISTAR_ACTION_RESULT(40,  BuildTechRequirementsNotMet)
ALISTAR_ACTION_RESULT(41,  CantFindPlacementLocation)
ALISTAR_ACTION_RESULT(42,  CantBuildOnThat)
ALISTAR_ACTION_RESULT(43,  CantBuildTooCloseToDropOff)
ALISTAR_ACTION_RESULT(44,  CantBuildLocationInvalid)
ALISTAR_ACTION_RESULT(45,  CantSeeBuildLocation)
ALISTAR_ACTION_RESULT(46,  CantBuildTooCloseToCreepSource)
ALISTAR_ACTION_RESULT(47,  CantBuildTooCloseToResources)
ALISTAR_ACTION_RESULT(48,  CantBuildTooFarFromWater)
ALISTAR_ACTION_RESULT(49,  CantBuildTooFarFromCreepSource)
ALISTAR_ACTION_RESULT(50,  CantBuildTooFarFromBuildPowerSource)
ALISTAR_ACTION_RESULT(51,  CantBuildOnDenseTerrain)
ALISTAR_ACTION_RESULT(52,  CantTrainTooFarFromTrainPowerSource)
ALISTAR_ACTION_RESULT(53,  CantLandLocationInvalid)
ALISTAR_ACTION_RESULT(54,  CantSeeLandLocation)
ALISTAR_ACTION_RESULT(55,  CantLandTooCloseToCreepSource)
ALISTAR_ACTION_RESULT(56,  CantLandTooCloseToResources)
ALISTAR_ACTION_RESULT(57,  CantLandTooFarFromWater)
ALISTAR_ACTION_RESULT(58,  CantLandTooFarFromCreepSource)
ALISTAR_ACTION_RESULT(59,  CantLandTooFarFromBuildPowerSource)
ALISTAR_ACTION_RESULT(60,  CantLandTooFarFromTrainPowerSource)
ALISTAR_ACTION_RESULT(61,  CantLandOnDenseTerrain)
ALISTAR_ACTION_RESULT(62,  AddOnTooFarFromBuilding)
ALISTAR_ACTION_RESULT(63,  MustBuildRefineryFirst)
ALISTAR_ACTION_RESULT(64,  BuildingIsUnderConstruction)
ALISTAR_ACTION_RESULT(65,  CantFindDropOff)
ALISTAR_ACTION_RESULT(66,  CantLoadOtherPlayersUnits)
ALISTAR_ACTION_RESULT(67,  NotEnoughRoomToLoadUnit)
ALISTAR_ACTION_RESULT(68,  CantUnloadUnitsThere)
ALISTAR_ACTION_RESULT(69,  CantWarpInUnitsThere)
ALISTAR_ACTION_RESULT(70,  CantLoadImmobileUnits)
ALISTAR_ACTION_RESULT(71,  CantRechargeImmobileUnits)
ALISTAR_ACTION_RESULT(72,  CantRechargeUnderConstructionUnits)
ALISTAR_ACTION_RESULT(73,  CantLoadThatUnit)
ALISTAR_ACTION_RESULT(74,  NoCargoToUnload)
ALISTAR_ACTION_RESULT(75,  LoadAllNoTargetsFound)
ALISTAR_ACTION_RESULT(76,  NotWhileOccupied)
ALISTAR_ACTION_RESULT(77,  CantAttackWithoutAmmo)
ALISTAR_ACTION_RESULT(78,  CantHoldAnyMoreAmmo)
ALISTAR_ACTION_RESULT(79,  TechRequirementsNotMet)
ALISTAR_ACTION_RESULT(80,  MustLockdownUnitFirst)
ALISTAR_ACTION_RESULT(81,  MustTargetUnit)
ALISTAR_ACTION_RESULT(82,  MustTargetInventory)
ALISTAR_ACTION_RESULT(83,  MustTargetVisibleUnit)
ALISTAR_ACTION_RESULT(84,  MustTargetVisibleLocation)
ALISTAR_ACTION_RESULT(85,  MustTargetWalkableLocation)
ALISTAR_ACTION_RESULT(86,  MustTargetPawnableUnit)
ALISTAR_ACTION_RESULT(87,  YouCantControlThatUnit)
ALISTAR_ACTION_RESULT(88,  YouCantIssueCommandsToThatUnit)
ALISTAR_ACTION_RESULT(89,  MustTargetResources)
ALISTAR_ACTION_RESULT(90,  RequiresHealTarget)
ALISTAR_ACTION_RESULT(91,  RequiresRepairTarget)
ALISTAR_ACTION_RESULT(92,  NoItemsToDrop)
ALISTAR_ACTION_RESULT(93,  CantHoldAnyMoreItems)
ALISTAR_ACTION_RESULT(94,  CantHoldThat)
ALISTAR_ACTION_RESULT(95,  TargetHasNoInventory)
ALISTAR_ACTION_RESULT(96,  CantDropThisItem)
ALISTAR_ACTION_RESULT(97,  CantMoveThisItem)
ALISTAR_ACTION_RESULT(98,  CantPawnThisUnit)
ALISTAR_ACTION_RESULT(99,  MustTargetCaster)
ALISTAR_ACTION_RESULT(100,  CantTargetCaster)
ALISTAR_ACTION_RESULT(101,  MustTargetOuter)
ALISTAR_ACTION_RESULT(102,  CantTargetOuter)
ALISTAR_ACTION_RESULT(103,  MustTargetYourOwnUnits)
ALISTAR_ACTION_RESULT(104,  CantTargetYourOwnUnits)
ALISTAR_ACTION_RESULT(105,  MustTargetFriendlyUnits)
ALISTAR_ACTION_RESULT(106,  CantTargetFriendlyUnits)
ALISTAR_ACTION_RESULT(107,  MustTargetNeutralUnits)
ALISTAR_ACTION_RESULT(108,  CantTargetNeutralUnits)
ALISTAR_ACTION_RESULT(109,  MustTargetEnemyUnits)
ALISTAR_ACTION_RESULT(110,  CantTargetEnemyUnits)
ALISTAR_ACTION_RESULT(111,  MustTargetAirUnits)
ALISTAR_ACTION_RESULT(112,  CantTargetAirUnits)
ALISTAR_ACTION_RESULT(113,  MustTargetGroundUnits)
ALISTAR_ACTION_RESULT(114,  CantTargetGroundUnits)
ALISTAR_ACTION_RESULT(115,  MustTargetStructures)
ALISTAR_ACTION_RESULT(116,  CantTargetStructures)
ALISTAR_ACTION_RESULT(117,  MustTargetLightUnits)
ALISTAR_ACTION_RESULT(118,  CantTargetLightUnits)
ALISTAR_ACTION_RESULT(119,  MustTargetArmoredUnits)
ALISTAR_ACTION_RESULT(120,  CantTargetArmoredUnits)
ALISTAR_ACTION_RESULT(121,  MustTargetBiologicalUnits)
ALISTAR_ACTION_RESULT(122,  CantTargetBiologicalUnits)
ALISTAR_ACTION_RESULT(123,  MustTargetHeroicUnits)
ALISTAR_ACTION_RESULT(124,  CantTargetHeroicUnits)
ALISTAR_ACTION_RESULT(125,  MustTargetRoboticUnits)
ALISTAR_ACTION_RESULT(126,  CantTargetRoboticUnits)
ALISTAR_ACTION_RESULT(127,  MustTargetMechanicalUnits)
ALISTAR_ACTION_RESULT(128,  CantTargetMechanicalUnits)
ALISTAR_ACTION_RESULT(129,  MustTargetPsionicUnits)
ALISTAR_ACTION_RESULT(130,  CantTargetPsionicUnits)
ALISTAR_ACTION_RESULT(131,  MustTargetMassiveUnits)
ALISTAR_ACTION_RESULT(132,  CantTargetMassiveUnits)
ALISTAR_ACTION_RESULT(133,  MustTargetMissile)
ALISTAR_ACTION_RESULT(134,  CantTargetMissile)
ALISTAR_ACTION_RESULT(135,  MustTargetWorkerUnits)
ALISTAR_ACTION_RESULT(136,  CantTargetWorkerUnits)
ALISTAR_ACTION_RESULT(137,  MustTargetEnergyCapableUnits)
ALISTAR_ACTION_RESULT(138,  CantTargetEnergyCapableUnits)
ALISTAR_ACTION_RESULT(139,  MustTargetShieldCapableUnits)
ALISTAR_ACTION_RESULT(140,  CantTargetShieldCapableUnits)
ALISTAR_ACTION_RESULT(141,  MustTargetFlyers)
ALISTAR_ACTION_RESULT(142,  CantTargetFlyers)
ALISTAR_ACTION_RESULT(143,  MustTargetBuriedUnits)
ALISTAR_ACTION_RESULT(144,  CantTargetBuriedUnits)
ALISTAR_ACTION_RESULT(145,  MustTargetCloakedUnits)
ALISTAR_ACTION_RESULT(146,  CantTargetCloakedUnits)
ALISTAR_ACTION_RESULT(147,  MustTargetUnitsInAStasisField)
ALISTAR_ACTION_RESULT(148,  CantTargetUnitsInAStasisField)
ALISTAR_ACTION_RESULT(149,  MustTargetUnderConstructionUnits)
ALISTAR_ACTION_RESULT(150,  CantTargetUnderConstructionUnits)
ALISTAR_ACTION_RESULT(151,  MustTargetDeadUnits)
ALISTAR_ACTION_RESULT(152,  CantTargetDeadUnits)
ALISTAR_ACTION_RESULT(153,  MustTargetRevivableUnits)
ALISTAR_ACTION_RESULT(154,  CantTargetRevivableUnits)
ALISTAR_ACTION_RESULT(155,  MustTargetHiddenUnits)
ALISTAR_ACTION_RESULT(156,  CantTargetHiddenUnits)
ALISTAR_ACTION_RESULT(157,  CantRechargeOtherPlayersUnits)
ALISTAR_ACTION_RESULT(158,  MustTargetHallucinations)
ALISTAR_ACTION_RESULT(159,  CantTargetHallucinations)
ALISTAR_ACTION_RESULT(160,  MustTargetInvulnerableUnits)
ALISTAR_ACTION_RESULT(161,  CantTargetInvulnerableUnits)
ALISTAR_ACTION_RESULT(162,  MustTargetDetectedUnits)
ALISTAR_ACTION_RESULT(163,  CantTargetDetectedUnits)
ALISTAR_ACTION_RESULT(164,  CantTargetUnitWithEnergy)
ALISTAR_ACTION_RESULT(165,  CantTargetUnitWithShields)
ALISTAR_ACTION_RESULT(166,  MustTargetUncommandableUnits)
ALISTAR_ACTION_RESULT(167,  CantTargetUncommandableUnits)
ALISTAR_ACTION_RESULT(168,  MustTargetPreventDefeatUnits)
ALISTAR_ACTION_RESULT(169,  CantTargetPreventDefeatUnits)
ALISTAR_ACTION_RESULT(170,  MustTargetPreventRevealUnits)
ALISTAR_ACTION_RESULT(171,  CantTargetPreventRevealUnits)
ALISTAR_ACTION_RESULT(172,  MustTargetPassiveUnits)
ALISTAR_ACTION_RESULT(173,  CantTargetPassiveUnits)
ALISTAR_ACTION_RESULT(174,  MustTargetStunnedUnits)
ALISTAR_ACTION_RESULT(175,  CantTargetStunnedUnits)
ALISTAR_ACTION_RESULT(176,  MustTargetSummonedUnits)
ALISTAR_ACTION_RESULT(177,  CantTargetSummonedUnits)
ALISTAR_ACTION_RESULT(178,  MustTargetUser1)
ALISTAR_ACTION_RESULT(179,  CantTargetUser1)
ALISTAR_ACTION_RESULT(180,  MustTargetUnstoppableUnits)
ALISTAR_ACTION_RESULT(181,  CantTargetUnstoppableUnits)
ALISTAR_ACTION_RESULT(182,  MustTargetResistantUnits)
ALISTAR_ACTION_RESULT(183,  CantTargetResistantUnits)
ALISTAR_ACTION_RESULT(184,  MustTargetDazedUnits)
ALISTAR_ACTION_RESULT(185,  CantTargetDazedUnits)
ALISTAR_ACTION_RESULT(186,  CantLockdown)
ALISTAR_ACTION_RESULT(187,  CantMindControl)
ALISTAR_ACTION_RESULT(188,  MustTargetDestructibles)
ALISTAR_ACTION_RESULT(189,  CantTargetDestructibles)
ALISTAR_ACTION_RESULT(190,  MustTargetItems)
ALISTAR_ACTION_RESULT(191,  CantTargetItems)
ALISTAR_ACTION_RESULT(192,  NoCalldownAvailable)
ALISTAR_ACTION_RESULT(193,  WaypointListFull)
ALISTAR_ACTION_RESULT(194,  MustTargetRace)
ALISTAR_ACTION_RESULT(195,  CantTargetRace)
ALISTAR_ACTION_RESULT(196,  MustTargetSimilarUnits)
ALISTAR_ACTION_RESULT(197,  CantTargetSimilarUnits)
ALISTAR_ACTION_RESULT(198,  CantFindEnoughTargets)
ALISTAR_ACTION_RESULT(199,  AlreadySpawningLarva)
ALISTAR_ACTION_RESULT(200,  CantTargetExhaustedResources)
ALISTAR_ACTION_RESULT(201,  CantUseMinimap)
ALISTAR_ACTION_RESULT(202,  CantUseInfoPanel)
ALISTAR_ACTION_RESULT(203,  OrderQueueIsFull)
ALISTAR_ACTION_RESULT(204,  CantHarvestThatResource)
ALISTAR_ACTION_RESULT(205,  HarvestersNotRequired)
ALISTAR_ACTION_RESULT(206,  AlreadyTargeted)
ALISTAR_ACTION_RESULT(207,  CantAttackWeaponsDisabled)
ALISTAR_ACTION_RESULT(208,  CouldntReachTarget)
ALISTAR_ACTION_RESULT(209,  TargetIsOutOfRange)
ALISTAR_ACTION_RESULT(210,  TargetIsTooClose)
ALISTAR_ACTION_RESULT(211,  TargetIsOutOfArc)
ALISTAR_ACTION_RESULT(212,  CantFindTeleportLocation)
ALISTAR_ACTION_RESULT(213,  InvalidItemClass)
ALISTAR_ACTION_RESULT(214,  CantFindCancelOrder)
#endif
