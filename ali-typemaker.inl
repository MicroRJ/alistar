// Copyright(C) 2022-2023 Dayan Rodriguez, All rights reserved.
#include "ali-types.inl"
typedef struct ALISTAR_STRING_CONCAT(Ali,ALI_TYPENAME)
{
#define ALI_TYPEFIELD(TYPE,METHOD,TAG,NAME) TYPE NAME;
#include "ali-types.inl"
} ALISTAR_STRING_CONCAT(Ali,ALI_TYPENAME);

static const char *ALISTAR_STRING_CONCAT(AliFieldStrings_,ALI_TYPENAME)[]=
{
#define ALI_TYPEFIELD(TYPE,METHOD,TAG,NAME) ALISTAR_TO_STRING(NAME),
#include "ali-types.inl"
};

ALISTAR_PARSE_FUNCTION void
ALISTAR_STRING_CONCAT(AliParse,ALI_TYPENAME)
(xstate *read, ALISTAR_STRING_CONCAT(Ali,ALI_TYPENAME) *val)
{ ZeroMemory(val,sizeof(*val));
  ForMessageField(read)
  {
#define ALI_TYPEFIELD(TYPE,METHOD,TAG,NAME)\
  case TAG: ALISTAR_STRING_CONCAT(Parse,TYPE)(read,&val->NAME); break;
#include "ali-types.inl"
  }
}