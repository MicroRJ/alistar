/*
**
** -+- AliStar -+-
**
** Copyright(C) Dayan Rodriguez, 2022, All rights reserved.
*/
#ifndef ALISTAR_H
#define ALISTAR_H

#define MG_EXPERIMENTAL_INTERFACES
#define USE_WEBSOCKET
#define OPENSSL_API_1_0
#include "civetweb.h"
#include "civetweb.c"

typedef struct mg_callbacks mg_callbacks;
typedef struct mg_context mg_context;
typedef struct mg_connection mg_connection;

typedef void *mutex_t;

mutex_t  create_mutex();
   void acquire_mutex(mutex_t);
   void release_mutex(mutex_t);

typedef struct thread_message_t thread_message_t;
typedef struct thread_message_t
{ int    type;
  void * value;
} thread_message_t;

typedef struct thread_t thread_t;
typedef struct thread_t
{ thread_message_t * array;
  int                count;
           mutex_t   mutex;
  void             * memory;
} thread_t;
void  push_thread_message(thread_t *, int, void *);
void *poll_thread_message(thread_t *, int,  int  );

typedef struct pipe_t pipe_t;
typedef struct pipe_t
{ void *read;
  void *write;
} pipe_t;
pipe_t create_system_pipe(int, int);

typedef struct process_t process_t;
typedef struct process_t
{ void         * ProcessHandle;
  void         * ThreadHandle;
  unsigned int   ThreadId;
  unsigned int   ProcessId;
  pipe_t Input;
  pipe_t Output;
  pipe_t Error;
} process_t;
int    launch_system_process(process_t *, const char *, const char *, const char *);
int launch_starcraft_process(process_t *, const char *, int, const char *, int);

typedef struct context_t context_t;
typedef struct context_t
{ mg_connection *conn;
  thread_t       thread;
} context_t;

void *                     send_payload(context_t *, int, ssvalue_t);
void *           request_available_maps(context_t *, int);
void *              request_create_game(context_t *, int);
void * request_join_game_as_participant(context_t *, int, int, const char *);
void *                request_game_info(context_t *, int);
void *              request_observation(context_t *, int, int, int);


#define ALISTAR_EXPAND_(A) A
#define ALISTAR_EXPAND(A) ALISTAR_EXPAND_(A)

#define ALISTAR_TO_STRING(A) #A
#define ALISTAR_STRING_CONCAT_(A,B) A##B

#define ALISTAR_STRING_CONCAT(A,B) ALISTAR_STRING_CONCAT_(A,B)

#define ALISTAR_PARSE_FUNCTION static

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
{ unsigned int    bits_per_pixel;
  ssi32x2_t       size;
  unsigned char * bytes;
} ImageData;


void ssread_i32x2(ssread_t *read, ssi32x2_t *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1: ssread_svar32(read,&val->x); break;
    case 2: ssread_svar32(read,&val->y); break;
  }
}

void ssread_i32x3(ssread_t *read, ssr32x3_t *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1: ssread_real32(read,&val->x); break;
    case 2: ssread_real32(read,&val->y); break;
    case 3: ssread_real32(read,&val->z); break;
  }
}

void ssread_f32x2(ssread_t *read, ssr32x2_t *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1: ssread_real32(read,&val->x); break;
    case 2: ssread_real32(read,&val->y); break;
  }
}

void ssread_i32x4(ssread_t *read, ssi32x4_t *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1: ssread_i32x2(read,&val->min); break;
    case 2: ssread_i32x2(read,&val->max); break;
  }
}

ALISTAR_PARSE_FUNCTION void
ParseImageData(ssread_t *read, ImageData *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1: ssread_uvar32(read,&val->bits_per_pixel); break;
    case 2: ssread_i32x2(read,  &val->size); break;
    case 3: ssread_bytes(read, &val->bytes); break;
  }
}
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

typedef enum ActionResult
{
#define ALISTAR_ACTION_RESULT(CODE,NAME) ALISTAR_STRING_CONCAT(ActionResult,NAME)=CODE,
# include "alistar.h"
#undef ALISTAR_ACTION_RESULT
} ActionResult;

typedef struct ActionRawUnitCommand
{ int32_t    ability_id;
  union
  { ssr32x2_t  target_world_space_pos; // worldspace
    uint64_t target_unit_tag;
  };
  uint64_t  *unit_tags;
  uint32_t   is_target_unit_tag;
  int        queue_command;
} ActionRawUnitCommand;
typedef struct ActionRawCameraMove
{  ssr32x2_t loca; // center, worldspace
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
  ssi32x2_t       coord; // screen or minimap space
  unsigned     is_minimap_coord: 1;
  unsigned     is_queue_command: 1;
} ActionSpatialUnitCommand;
typedef struct ActionSpatialCameraMove
{ ssi32x2_t       minimap;
} ActionSpatialCameraMove;
typedef struct ActionSpatialUnitSelectionPoint
{ ssi32x2_t screen;
  int    select;
} ActionSpatialUnitSelectionPoint;
typedef struct ActionSpatialUnitSelectionRect
{ ssi32x4_t   screen[0x10];
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
    ssr32x3_t  target_world_space_pos; // optional:2
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
{ ssr32x2_t   point; // 1   Will always be filled.
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

  ssr32x3_t          pos;            // optional:6

  uint32_t       * buff_ids;       // optional:27
  UnitOrder      * orders;         // repeated:22 (not populated for enemies)
  PassengerUnit  * passengers;     // repeated:24 (not populated for enemies)
  RallyTarget    * rally_targets;  // repeated:45 (not populated for enemies)
} Unit;
typedef struct ResponseAction
{ ActionResult *results; // array
} ResponseAction;
typedef struct StartRaw
{ ssi32x2_t     map_size;
  ImageData   pathing_grid;
  ImageData   terrain_height;
  ImageData   placement_grid;
  ssi32x4_t  playable_area;
  ssr32x2_t    *start_locations;
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
typedef struct RequestAction
{ Action *actions;
} RequestAction;
typedef struct Request
{ union
  { RequestAction action;
  };
} Request;
typedef struct ResponseQueryPathing
{ float distance; // optional:1  Note: 0 if no path exists
} ResponseQueryPathing;
typedef struct ResponseQueryBuildingPlacement
{ ActionResult result; // optional:1
} ResponseQueryBuildingPlacement;
typedef struct ResponseQuery
{ ResponseQueryPathing           *pathing;    // repeated:1;
  ResponseQueryBuildingPlacement *placements; // repeated:3;
} ResponseQuery;


static const wchar_t *
AlistarActionResultW(ActionResult result);


// Parsing functions
ALISTAR_PARSE_FUNCTION void
ParseResponseQueryPathing(ssread_t *read, ResponseQueryPathing *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1: ssread_real32(read,&val->distance); break;
  }
}
// Parsing functions
ALISTAR_PARSE_FUNCTION void
ParseResponseQueryBuildingPlacement(ssread_t *read, ResponseQueryBuildingPlacement *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1: ssread_enumerator(read,&val->result); break;
  }
}
ALISTAR_PARSE_FUNCTION void
ParseResponseQuery(ssread_t *read, ResponseQuery *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1: ParseResponseQueryPathing(read,ccarradd(val->pathing,1)); break;
    case 3: ParseResponseQueryBuildingPlacement(read,ccarradd(val->placements,1)); break;
  }
}
// Mainly for debugging reasons
ALISTAR_PARSE_FUNCTION void
ParseActionChat(ssread_t *read, ActionChat *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1: ssread_enumerator(read,&val->channel);   break;
    case 2: ssread_string(read,&val->message); break;
  }
}
ALISTAR_PARSE_FUNCTION void
ParseActionRawUnitCommand(ssread_t *read, ActionRawUnitCommand *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1: ssread_svar32(read, &val->ability_id);             break;
    case 2: ssread_f32x2(read, &val->target_world_space_pos);   break;
    case 3: ssread_uvar64(read, &val->target_unit_tag);      break;
    case 4: ssread_uvar64(read, ccarradd(val->unit_tags,1));   break;
    case 5: ssread_svar32(read, &val->queue_command);          break;
  }
}
ALISTAR_PARSE_FUNCTION void
ParseActionRaw(ssread_t *read, ActionRaw *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1: ParseActionRawUnitCommand(read, &val->unit_command); break;
  }
}
ALISTAR_PARSE_FUNCTION void
ParseAction(ssread_t *read, Action *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1: ParseActionRaw(read, &val->action_raw); break;
    // case 2:  ParseActionFeatureLayer(read, &val->action_feature_layer); break;
    // case 3:  ParseActionRender(read, &val->action_render); break;
    // case 4:  ParseActionUI(read, &val->action_ui); break;
    case 6: ParseActionChat(read, &val->action_chat); break;
    case 7: ssread_uvar32(read, &val->game_loop);    break;
  }
}
ALISTAR_PARSE_FUNCTION void
ParseRequestAction(ssread_t *read, RequestAction *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1: ParseAction(read,ccarradd(val->actions,1)); break;
  }
}
ALISTAR_PARSE_FUNCTION void
ParseRequest(ssread_t *read, Request *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 11: ParseRequestAction(read,&val->action); break;
  }
}
// Player
ALISTAR_PARSE_FUNCTION void
ParsePlayerInfo(ssread_t *read, PlayerInfo *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1: ssread_uvar32(read,&val->iden);   break;
    case 2: ssread_enumerator(read,&val->type);     break;
    case 3: ssread_enumerator(read,&val->race_r);   break;
    case 4: ssread_enumerator(read,&val->race_a);   break;
    case 5: ssread_enumerator(read,&val->diff);     break;
    case 7: ssread_enumerator(read,&val->build);    break;
    case 6: ssread_string(read,&val->name);   break;
  }
}
ALISTAR_PARSE_FUNCTION void
ParseResponseCreateGame(ssread_t *read, ResponseCreateGame *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1: ssread_enumerator(read,&val->errbit);   break;
    case 2: ssread_string(read,&val->errstr); break;
  }
}
ALISTAR_PARSE_FUNCTION void
ParseResponseJoinGame(ssread_t *read, ResponseJoinGame *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1: ssread_uvar32(read,&val->player); break;
    case 2: ssread_enumerator(read,&val->errbit);   break;
    case 3: ssread_string(read,&val->errstr); break;
  }
}
ALISTAR_PARSE_FUNCTION void
ParsePlayerCommon(ssread_t *read, PlayerCommon *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1:  ssread_uvar32(read,&val->player_id);        break;
    case 2:  ssread_uvar32(read,&val->minerals);         break;
    case 3:  ssread_uvar32(read,&val->vespene);          break;
    case 4:  ssread_uvar32(read,&val->food_cap);         break;
    case 5:  ssread_uvar32(read,&val->food_used);        break;
    case 6:  ssread_uvar32(read,&val->food_army);        break;
    case 7:  ssread_uvar32(read,&val->food_workers);     break;
    case 8:  ssread_uvar32(read,&val->idle_worker_count);break;
    case 9:  ssread_uvar32(read,&val->army_count);       break;
    case 10: ssread_uvar32(read,&val->warp_gate_count);  break;
    case 11: ssread_uvar32(read,&val->larva_count);      break;
  }
}
ALISTAR_PARSE_FUNCTION void
ParseStartRaw(ssread_t *read, StartRaw *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1: ssread_i32x2(read,    &val->map_size);               break;
    case 2: ParseImageData(read,  &val->pathing_grid);           break;
    case 3: ParseImageData(read,  &val->terrain_height);         break;
    case 4: ParseImageData(read,  &val->placement_grid);         break;
    case 5: ssread_i32x4(read, &val->playable_area);          break;
    case 6: ssread_f32x2(read, ccarradd(val->start_locations,1)); break;
  }
}
ALISTAR_PARSE_FUNCTION void
ParseResponseGameInfo(ssread_t *read, ResponseGameInfo *val)
{ MapName *map;
  ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1:
    { map=ccarradd(val->maps,1);
      map->is_local=0;

      ssread_string(read,&map->string);
    } break;
    break;
    case 2:
    { map=ccarradd(val->maps,1);
      map->is_local=1;

      ssread_string(read,&map->string);
    } break;
    break;
    case 3: ParsePlayerInfo(read,ccarradd(val->players,1)); break;
    case 4: ParseStartRaw(read,&val->start_raw);          break;
    case 6: ssread_string(read,ccarradd(val->mods,1));     break;
  }
}

void ParseResponseAvailableMaps(ssread_t *read, ResponseAvailableMaps *value)
{
  ZeroMemory(value,sizeof(*value));

  ForMessageField(read)
  { case 1:
    { MapName *map=ccarradd(value->maps,1);
      map->is_local=1;
      ssread_string(read,&map->string);
    } break;
    case 2:
    { MapName *map=ccarradd(value->maps,1);
      map->is_local=0;
      ssread_string(read,&map->string);
    } break;
  }
}

ALISTAR_PARSE_FUNCTION void
ParseErrorAction(ssread_t *read, ErrorAction *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1: ssread_uvar64(read,&val->unit_tag);   break;
    case 2: ssread_uvar64(read,&val->ability_id); break;
    case 3: ssread_enumerator(read,&val->result);         break;
  }
}
ALISTAR_PARSE_FUNCTION void
ParseChatReceived(ssread_t *read, ChatReceived *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1: ssread_uvar32(read,&val->player_id); break;
    case 2: ssread_string(read,&val->message);   break;
  }
}
ALISTAR_PARSE_FUNCTION void
ParseUnitOrder(ssread_t *read, UnitOrder *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1: ssread_uvar32(read,&val->ability_id); break;
    case 2: ssread_i32x3(read,&val->target_world_space_pos); break;
    case 3: ssread_uvar64(read,&val->target_unit_tag); break;
    case 4: ssread_real32(read,&val->progress); break;
  }
}
ALISTAR_PARSE_FUNCTION void
ParseUnit(ssread_t *read, Unit *val)
{ ZeroMemory(val,sizeof(*val));

}
ALISTAR_PARSE_FUNCTION void
ParsePlayerResult(ssread_t *read, PlayerResult *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1: ssread_uvar32(read,&val->player_id); break;
    case 2: ssread_enumerator(read,&val->result); break;
  }
}
ALISTAR_PARSE_FUNCTION void
ParseMapState(ssread_t *read, MapState *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1: ParseImageData(read,&val->visibility); break;
    case 2: ParseImageData(read,&val->creep);      break;
  }
}

typedef struct ResponseObservation
{ Action       *actions; // array
  ErrorAction  *error_actions; // array

  uint32_t      /*3.9     */  GameLoop;
  PlayerCommon  /*3.1     */  PlayerCommon;
  Alert         /*3.10    */ *Alerts;
  Unit          /*3.5.2   */ *Units;
  MapState      /*3.5.3   */  MapState;
  ImageData     /*3.6.1. 1*/  HeightMap;
  ImageData     /*3.6.1. 2*/  VisibilityMap;
  ImageData     /*3.6.1. 3*/  Creep;
  ImageData     /*3.6.1. 4*/  Power;
  ImageData     /*3.6.1. 5*/  PlayerId;
  ImageData     /*3.6.1. 6*/  UnitType;
  ImageData     /*3.6.1. 7*/  Selected;
  ImageData     /*3.6.1. 8*/  UnitHitPoints;
  ImageData     /*3.6.1.17*/  UnitHitPointsRatio;
  ImageData     /*3.6.1. 9*/  UnitEnergy;
  ImageData     /*3.6.1.18*/  UnitEnergyRatio;
  ImageData     /*3.6.1.10*/  UnitShields;
  ImageData     /*3.6.1.19*/  UnitShieldsRatio;
  ImageData     /*3.6.1.11*/  PlayerRelative;
  ImageData     /*3.6.1.14*/  UnitDensityAA;
  ImageData     /*3.6.1.15*/  UnitDensity;
  ImageData     /*3.6.1.20*/  Effects;
  ImageData     /*3.6.1.21*/  Hallucinations;
  ImageData     /*3.6.1.22*/  Cloaked;
  ImageData     /*3.6.1.23*/  Blip;
  ImageData     /*3.6.1.24*/  Buffs;
  ImageData     /*3.6.1.26*/  BuffDuration;
  ImageData     /*3.6.1.25*/  Active;
  ImageData     /*3.6.1.27*/  BuildProgress;
  ImageData     /*3.6.1.28*/  Buildable;
  ImageData     /*3.6.1.29*/  Pathable;
  ImageData     /*3.6.1.30*/  Placeholder;
  ImageData     /*3.6.2. 1*/  MiniMapHeightMap;
  ImageData     /*3.6.2. 2*/  MiniMapVisibilityMap;
  ImageData     /*3.6.2. 3*/  MiniMapCreep;
  ImageData     /*3.6.2. 4*/  MiniMapCamera;
  ImageData     /*3.6.2. 5*/  MiniMapPlayerId;
  ImageData     /*3.6.2. 6*/  MiniMapPlayerRelative;
  ImageData     /*3.6.2. 7*/  MiniMapSelected;
  ImageData     /*3.6.2. 9*/  MiniMapAlerts;
  ImageData     /*3.6.2.10*/  MiniMapBuildable;
  ImageData     /*3.6.2.11*/  MiniMapPathable;
  ImageData     /*3.6.2. 8*/  MiniMapUnitType;
  ImageData     /*3.7.1   */  RenderMap;
  ImageData     /*3.7.2   */  RenderMinimap;

  // Observation   observation;
  PlayerResult *player_results; // array
  ChatReceived *chats; // array
} ResponseObservation;

ALISTAR_PARSE_FUNCTION void
ParseResponseObservation(ssread_t *read, ResponseObservation *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1: ParseAction(read,ccarradd(val->actions,1));             break;
    case 2: ParseErrorAction(read,ccarradd(val->error_actions,1));  break;
    case 4: ParsePlayerResult(read,ccarradd(val->player_results,1));break;
    case 5: ParseChatReceived(read,ccarradd(val->chats,1));         break;
    case 3:
    { ForMessageField(read)
      { case 9:  ssread_uvar32(read,&val->GameLoop); break;
        case 1:  ParsePlayerCommon(read,&val->PlayerCommon); break;
        case 10: ssread_enumerator(read,ccarradd(val->Alerts,1)); break;
        case 5:
        { ForMessageField(read)
          { case 3: ParseMapState(read,&val->MapState); break;
            case 2:
            { Unit *unit=ccarradd(val->Units,1);
              ZeroMemory(unit,sizeof(*unit));

              ForMessageField(read)
              { case 7:  ssread_real32(read,&unit->facing); break;
                case 8:  ssread_real32(read,&unit->radius); break;
                case 9:  ssread_real32(read,&unit->build_progress); break;
                case 31: ssread_real32(read,&unit->detect_range); break;
                case 32: ssread_real32(read,&unit->radar_range); break;
                case 14: ssread_real32(read,&unit->health); break;
                case 15: ssread_real32(read,&unit->health_max); break;
                case 16: ssread_real32(read,&unit->shield); break;
                case 36: ssread_real32(read,&unit->shield_max); break;
                case 17: ssread_real32(read,&unit->energy); break;
                case 37: ssread_real32(read,&unit->energy_max); break;
                case 30: ssread_real32(read,&unit->weapon_cooldown); break;
                case 3:  ssread_uvar64(read,&unit->tag); break;
                case 23: ssread_uvar64(read,&unit->add_on_tag); break;
                case 34: ssread_uvar64(read,&unit->engaged_target_tag); break;
                case 4:  ssread_uvar32(read,&unit->unit_type); break;
                case 40: ssread_svar32(read,&unit->attack_upgrade_level); break;
                case 41: ssread_svar32(read,&unit->armor_upgrade_level); break;
                case 42: ssread_svar32(read,&unit->shield_upgrade_level); break;
                case 18: ssread_svar32(read,&unit->mineral_contents); break;
                case 19: ssread_svar32(read,&unit->vespene_contents); break;
                case 25: ssread_svar32(read,&unit->cargo_space_taken); break;
                case 26: ssread_svar32(read,&unit->cargo_space_max); break;
                case 28: ssread_svar32(read,&unit->assigned_harvesters); break;
                case 29: ssread_svar32(read,&unit->ideal_harvesters); break;
                case 43: ssread_svar32(read,&unit->buff_duration_remain); break;
                case 44: ssread_svar32(read,&unit->buff_duration_max); break;
                case 5:  ssread_svar32(read,&unit->owner); break;
                case 20: ssread_svar32(read,&unit->is_flying); break;
                case 21: ssread_svar32(read,&unit->is_burrowed); break;
                case 38: ssread_svar32(read,&unit->is_hallucination); break;
                case 11: ssread_svar32(read,&unit->is_selected); break;
                case 12: ssread_svar32(read,&unit->is_on_screen); break;
                case 13: ssread_svar32(read,&unit->is_blip); break;
                case 35: ssread_svar32(read,&unit->is_powered); break;
                case 39: ssread_svar32(read,&unit->is_active); break;
                case 10: ssread_enumerator(read,&unit->cloak); break;
                case 1:  ssread_enumerator(read,&unit->display_type); break;
                case 2:  ssread_enumerator(read,&unit->alliance); break;
                case 6:  ssread_i32x3(read,&unit->pos); break;
                case 27: ssread_uvar32(read,ccarradd(unit->buff_ids,1)); break;
                case 22: ParseUnitOrder(read,ccarradd(unit->orders,1)); break;
                // case 24: PassengerUnit  * passengers;
                // case 45: RallyTarget    * rally_targets;
              }
            } break;
          }
        } break;
        case 6:
        { ForMessageField(read)
          { case 1:
            { ForMessageField(read)
              { case 1:  ParseImageData(read,&val->HeightMap); break;
                case 2:  ParseImageData(read,&val->VisibilityMap); break;
                case 3:  ParseImageData(read,&val->Creep); break;
                case 4:  ParseImageData(read,&val->Power); break;
                case 5:  ParseImageData(read,&val->PlayerId); break;
                case 6:  ParseImageData(read,&val->UnitType); break;
                case 7:  ParseImageData(read,&val->Selected); break;
                case 8:  ParseImageData(read,&val->UnitHitPoints); break;
                case 17: ParseImageData(read,&val->UnitHitPointsRatio); break;
                case 9:  ParseImageData(read,&val->UnitEnergy); break;
                case 18: ParseImageData(read,&val->UnitEnergyRatio); break;
                case 10: ParseImageData(read,&val->UnitShields); break;
                case 19: ParseImageData(read,&val->UnitShieldsRatio); break;
                case 11: ParseImageData(read,&val->PlayerRelative); break;
                case 14: ParseImageData(read,&val->UnitDensityAA); break;
                case 15: ParseImageData(read,&val->UnitDensity); break;
                case 20: ParseImageData(read,&val->Effects); break;
                case 21: ParseImageData(read,&val->Hallucinations); break;
                case 22: ParseImageData(read,&val->Cloaked); break;
                case 23: ParseImageData(read,&val->Blip); break;
                case 24: ParseImageData(read,&val->Buffs); break;
                case 26: ParseImageData(read,&val->BuffDuration); break;
                case 25: ParseImageData(read,&val->Active); break;
                case 27: ParseImageData(read,&val->BuildProgress); break;
                case 28: ParseImageData(read,&val->Buildable); break;
                case 29: ParseImageData(read,&val->Pathable); break;
                case 30: ParseImageData(read,&val->Placeholder); break;
              }
            } break;
            case 2:
            { ForMessageField(read)
              { case  1: ParseImageData(read,&val->MiniMapHeightMap); break;
                case  2: ParseImageData(read,&val->MiniMapVisibilityMap); break;
                case  3: ParseImageData(read,&val->MiniMapCreep); break;
                case  4: ParseImageData(read,&val->MiniMapCamera); break;
                case  5: ParseImageData(read,&val->MiniMapPlayerId); break;
                case  6: ParseImageData(read,&val->MiniMapPlayerRelative); break;
                case  7: ParseImageData(read,&val->MiniMapSelected); break;
                case  9: ParseImageData(read,&val->MiniMapAlerts); break;
                case 10: ParseImageData(read,&val->MiniMapBuildable); break;
                case 11: ParseImageData(read,&val->MiniMapPathable); break;
                case  8: ParseImageData(read,&val->MiniMapUnitType); break;
              }
            } break;
          }
        } break;
        case 7:
        { ForMessageField(read)
          { case 1: ParseImageData(read,&val->RenderMap); break;
            case 2: ParseImageData(read,&val->RenderMinimap); break;
          }
        } break;
      }

    } break;
    default:
    { cctracewar("unknown field: %i", ssread_fieldname(read));
    } break;
  }
}

void ParseResponseAction(ssread_t *read, ResponseAction *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  { case 1:
    {
      ActionResult *result;
      result=ccarradd(val->results,1);

      ssread_enumerator(read,result);

      cctracelog("Action Result #%s", AlistarActionResultW(*result));
    } break;
  }
}

typedef struct ssresponse_t ssresponse_t;
typedef struct ssresponse_t
{
union
{ ResponseGameInfo      GameInfo;
  ResponseCreateGame    CreateGame;
  ResponseJoinGame      JoinGame;
  ResponseAvailableMaps AvailableMaps;
  ResponseObservation   Observation;
  ResponseAction        Action;
  ResponseQuery         Query;
};
  int               Status;
  int               Type;
  unsigned int      Id;
  char           *  Error;
} ssresponse_t, ali_response;

ALISTAR_PARSE_FUNCTION void
ParseResponse(ssread_t *read, ssresponse_t *info)
{ ZeroMemory(info,sizeof(*info));
  ForMessageField(read)
  { case 99: ssread_enumerator(read,&info->Status);  break;
    case 98: ssread_string(read,&info->Error); break;
    case 97: ssread_uvar32(read,&info->Id);    break;
    case RESPONSE_TAG_CREATE_GAME:
    { ParseResponseCreateGame(read,&info->CreateGame);
      info->Type=RESPONSE_TAG_CREATE_GAME;
    } break;
    case RESPONSE_TAG_JOIN_GAME:
    { ParseResponseJoinGame(read,&info->JoinGame);
      info->Type=RESPONSE_TAG_JOIN_GAME;
    } break;
    case RESPONSE_TAG_GAME_INFO:
    { ParseResponseGameInfo(read,&info->GameInfo);
      info->Type=RESPONSE_TAG_GAME_INFO;
    } break;
    case RESPONSE_TAG_AVAILABLE_MAPS:
    { ParseResponseAvailableMaps(read,&info->AvailableMaps);
      info->Type=RESPONSE_TAG_AVAILABLE_MAPS;
    } break;
    case RESPONSE_TAG_OBSERVATION:
    { ParseResponseObservation(read,&info->Observation);
      info->Type=RESPONSE_TAG_OBSERVATION;
    } break;
    case RESPONSE_TAG_ACTION:
    { ParseResponseAction(read,&info->Action);
      info->Type=RESPONSE_TAG_ACTION;
    } break;
    case RESPONSE_TAG_QUERY:
    { ParseResponseQuery(read,&info->Query);
      info->Type=RESPONSE_TAG_QUERY;
    } break;
    default:
    { cctracewar("unknown response: %i", ssread_fieldname(read));
    } break;
  }
}


#if 0
#if 0
static int
AlistarRequestTrainSCV(context_t *ctx, uint64_t unit_tag)
{
  ssvalue_t raw_unit_command={sstype_kRECORD};
  AddVarint32Value(&raw_unit_command,/* tag */ 1, 524);
  AddVarint64Value(&raw_unit_command,/* tag */ 4, unit_tag);

  ssvalue_t action_raw={sstype_kRECORD};
  ssvalue_member(&action_raw,/* tag */1,raw_unit_command);

  ssvalue_t action={sstype_kRECORD};
  ssvalue_member(&action,/* tag */1,action_raw);

  ssvalue_t request_action={sstype_kRECORD};
  ssvalue_member(&request_action,/* tag */1,action);

  return AlistarSendPayload(ctx,11,&request_action);
}
static int
AlistarRequestBuildDepot(context_t *ctx, uint64_t unit_tag, ssr32x2_t location)
{ ssvalue_t raw_unit_command={sstype_kRECORD};
  AddVarint32Value(&raw_unit_command,1,319);
  AddVarint64Value(&raw_unit_command,4,unit_tag);
  ssvalue_member(&raw_unit_command,2,ValuePoint2D(location));

  ssvalue_t action_raw={sstype_kRECORD};
  ssvalue_member(&action_raw,1,raw_unit_command);

  ssvalue_t action={sstype_kRECORD};
  ssvalue_member(&action,1,action_raw);

  ssvalue_t request_action={sstype_kRECORD};
  ssvalue_member(&request_action,1,action);

  return AlistarSendPayload(ctx,11,&request_action);
}
static int
AlistarQueryDistanceToFromUnit(context_t *ctx, Unit *unit, ssr32x2_t pos)
{
  ssvalue_t req_query_pathing={sstype_kRECORD};
  AddVarint64Value(&req_query_pathing,2,unit->tag);
  ssvalue_member(&req_query_pathing,3,ValuePoint2D(pos));

  ssvalue_t req_query={sstype_kRECORD};
  ssvalue_member(&req_query,1,req_query_pathing);
  return AlistarSendPayload(ctx,14,&req_query);
}
static int
AlistarQueryBuildingPlacement(context_t *ctx, int32_t ability_id, ssr32x2_t location)
{
  ssvalue_t req_query_building_placement={sstype_kRECORD};
  AddVarint32Value(&req_query_building_placement,1,ability_id);
  ssvalue_member(&req_query_building_placement,2,ValuePoint2D(location));

  ssvalue_t req_query={sstype_kRECORD};
  ssvalue_member(&req_query,3,req_query_building_placement);
  return AlistarSendPayload(ctx,14,&req_query);
}
static int
AlistarSendChat(context_t *ctx, int32_t channel, const char *message)
{
  ssvalue_t action_chat={sstype_kRECORD};
  AddEnumValue(&action_chat,/* tag */1,channel);
  AddStringValue(&action_chat,/* tag */2,message);

  ssvalue_t action={sstype_kRECORD};
  ssvalue_member(&action,/* tag */6,action_chat);

  ssvalue_t request_action={sstype_kRECORD};
  ssvalue_member(&request_action,/* tag */1,action);

  return AlistarSendPayload(ctx,11,&request_action);
}
#endif
#endif

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
