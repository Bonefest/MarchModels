#include <unordered_map>

using std::string;
using std::unordered_map;

#include "cvar_system.h"

template <typename T>
struct CVar
{
  T value;
  CVarFlags flags;
};

struct CVarMeta
{
  CVarType type;
  CVarFlags flags;
};

using CVarInt = CVar<int32>;
using CVarUint = CVar<uint32>;
using CVarFloat = CVar<float32>;
using CVarBool = CVar<bool8>;

struct CVarSystemData
{
  unordered_map<string, CVarInt> intCVars;
  unordered_map<string, CVarUint> uintCVars;
  unordered_map<string, CVarFloat> floatCVars;
  unordered_map<string, CVarBool> boolCVars;
  unordered_map<string, CVarMeta> cvarsMeta;
};

static CVarSystemData data;

bool8 CVarSystemRegisterIntVar(const string& name, int32 initValue, CVarFlags flags)
{
  if(CVarSystemHasVar(name) == TRUE)
  {
    return FALSE;
  }

  data.cvarsMeta[name] = CVarMeta{ CVAR_TYPE_INT, flags};
  data.intCVars[name] = CVarInt { initValue, flags };
  
  return TRUE;
}

bool8 CVarSystemRegisterUintVar(const string& name, uint32 initValue, CVarFlags flags)
{
  if(CVarSystemHasVar(name) == TRUE)
  {
    return FALSE;
  }

  data.cvarsMeta[name] = CVarMeta{ CVAR_TYPE_UINT, flags};  
  data.uintCVars[name] = CVarUint { initValue, flags };
  
  return TRUE;
}

bool8 CVarSystemRegisterFloatVar(const string& name, float32 initValue, CVarFlags flags)
{
  if(CVarSystemHasVar(name) == TRUE)
  {
    return FALSE;
  }

  data.cvarsMeta[name] = CVarMeta{ CVAR_TYPE_FLOAT, flags};    
  data.floatCVars[name] = CVarFloat { initValue, flags };
  
  return TRUE;
}

bool8 CVarSystemRegisterBoolVar(const string& name, bool8 initValue, CVarFlags flags)
{
  if(CVarSystemHasVar(name) == TRUE)
  {
    return FALSE;
  }

  data.cvarsMeta[name] = CVarMeta{ CVAR_TYPE_BOOL, flags};      
  data.boolCVars[name] = CVarBool { initValue, flags };
  
  return TRUE;
}

#define ASSERT_VAR_EXISTS(name) \
  assert(CVarSystemHasVar(name) == TRUE && "Requested variable doesn't exist!")

const int32& CVarSystemReadInt(const string& name)
{
  ASSERT_VAR_EXISTS(name);
  return data.intCVars[name].value;
}

int32& CVarSystemGetInt(const string& name)
{
  ASSERT_VAR_EXISTS(name);
  return data.intCVars[name].value;
}

const uint32& CVarSystemReadUint(const string& name)
{
  ASSERT_VAR_EXISTS(name);
  return data.uintCVars[name].value;
}

uint32& CVarSystemGetUint(const string& name)
{
  ASSERT_VAR_EXISTS(name);
  return data.uintCVars[name].value;
}

const float32& CVarSystemReadFloat(const string& name)
{
  ASSERT_VAR_EXISTS(name);
  return data.floatCVars[name].value;
}

float32& CVarSystemGetFloat(const string& name)
{
  ASSERT_VAR_EXISTS(name);
  return data.floatCVars[name].value;
}

const bool8& CVarSystemReadBool(const string& name)
{
  ASSERT_VAR_EXISTS(name);
  return data.boolCVars[name].value;
}

bool8& CVarSystemGetBool(const string& name)
{
  ASSERT_VAR_EXISTS(name);
  return data.boolCVars[name].value;
}

CVarType CVarSystemGetVarType(const string& name)
{
  if(CVarSystemHasVar(name) == FALSE)
  {
    return CVAR_TYPE_UNKNOWN;
  }

  return data.cvarsMeta[name].type;
}

CVarFlags CVarSystemGetVarFlags(const std::string& name)
{
  if(CVarSystemHasVar(name) == FALSE)
  {
    return CVAR_FLAG_NONE;
  }

  return data.cvarsMeta[name].flags;
}

bool8 CVarSystemHasVar(const string& name)
{
  return data.cvarsMeta.find(name) != data.cvarsMeta.end();
}

