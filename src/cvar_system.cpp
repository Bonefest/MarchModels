#include <algorithm>
#include <unordered_map>

using std::string;
using std::vector;
using std::transform;
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

string CVarSystemReadStr(const std::string& name)
{
  char result[256] = {};
  
  auto metaInfo = data.cvarsMeta.find(name);
  
  if(metaInfo == data.cvarsMeta.end())
  {
    return result;
  }

  switch(metaInfo->second.type)
  {
    case CVAR_TYPE_INT: sprintf(result, "%d", CVarSystemReadInt(name)); break;
    case CVAR_TYPE_UINT: sprintf(result, "%u", CVarSystemReadUint(name)); break;
    case CVAR_TYPE_FLOAT: sprintf(result, "%f", CVarSystemReadFloat(name)); break;
    case CVAR_TYPE_BOOL: sprintf(result, "%s", CVarSystemReadBool(name) == TRUE ? "true" : "false"); break;

    default: assert(false);
  }
  
  return result;
}

CVarParseCode CVarSystemParseStr(const std::string& name, const std::string& val)
{
  auto metaInfo = data.cvarsMeta.find(name);
  
  if(metaInfo == data.cvarsMeta.end())
  {
    return CVAR_PARSE_CODE_VAR_NOT_FOUND;
  }

  if((metaInfo->second.flags & CVAR_FLAG_READ_ONLY) == CVAR_FLAG_READ_ONLY)
  {
    return CVAR_PARSE_CODE_VAR_READ_ONLY;
  }

  switch(metaInfo->second.type)
  {
    case CVAR_TYPE_INT:
    {
      int32 ival = 0;
      if(sscanf(val.c_str(), "%d", &ival) == 0)
      {
        return CVAR_PARSE_CODE_CANNOT_PARSE;
      }

      CVarSystemGetInt(name) = ival;
      return CVAR_PARSE_CODE_SUCCESS;
    }
    case CVAR_TYPE_UINT:
    {
      uint32 uval = 0;
      if(sscanf(val.c_str(), "%u", &uval) == 0)
      {
        return CVAR_PARSE_CODE_CANNOT_PARSE;
      }

      CVarSystemGetUint(name) = uval;
      return CVAR_PARSE_CODE_SUCCESS;
    }
    case CVAR_TYPE_FLOAT:
    {
      float32 fval = 0;
      if(sscanf(val.c_str(), "%f", &fval) == 0)
      {
        return CVAR_PARSE_CODE_CANNOT_PARSE;
      }

      CVarSystemGetFloat(name) = fval;
      return CVAR_PARSE_CODE_SUCCESS;
    }
    case CVAR_TYPE_BOOL:
    {
      int32 ival = 0;
      if(sscanf(val.c_str(), "%d", &ival) == 0)
      {
        string sval = val;
        transform(sval.begin(), sval.end(), sval.begin(), [](auto c){ return tolower(c); });

        if(sval == "true")
        {
          ival = 1;
        }
        else if(sval == "false")
        {
          ival = 0;
        }        
        else
        {
          return CVAR_PARSE_CODE_CANNOT_PARSE;          
        }
      }

      bool8 bval = ival > 0 ? TRUE : FALSE;
      
      CVarSystemGetBool(name) = bval;
      return CVAR_PARSE_CODE_SUCCESS;
    }

    default: assert(false);
  }

  return CVAR_PARSE_CODE_OTHER;
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

vector<string> CVarSystemGetRegisteredVars()
{
  vector<string> names;

  for(auto cvarMeta: data.cvarsMeta)
  {
    names.push_back(cvarMeta.first);
  }

  return names;
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

