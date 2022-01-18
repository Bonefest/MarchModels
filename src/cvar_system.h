#pragma once

#include <string>

#include "defines.h"

enum CVarFlags
{
  CVAR_FLAG_NONE      = 0x0,
  CVAR_FLAG_READ_ONLY = 0x01,
  CVAR_FLAG_STATIC    = 0x10
};

enum CVarType
{
  CVAR_TYPE_INT,
  CVAR_TYPE_UINT,
  CVAR_TYPE_FLOAT,
  CVAR_TYPE_BOOL,
  CVAR_TYPE_UNKNOWN
};

template <typename T>
struct CVarTypeTrait;

template <>
struct CVarTypeTrait<int32>
{
  static const CVarType type = CVAR_TYPE_INT;
};

template <>
struct CVarTypeTrait<uint32>
{
  static const CVarType type = CVAR_TYPE_UINT;
};

template <>
struct CVarTypeTrait<float32>
{
  static const CVarType type = CVAR_TYPE_FLOAT;
};

template <>
struct CVarTypeTrait<bool8>
{
  static const CVarType type = CVAR_TYPE_BOOL;
};

template <typename T>
ENGINE_API bool8 CVarSystemRegisterVar(const std::string& name, T initValue, CVarFlags flags)
{
  CVarType type = CVarTypeTrait<T>::type;
  switch(type)
  {
    case CVAR_TYPE_INT: return CVarSystemRegisterIntVar(name, initValue, flags);
    case CVAR_TYPE_UINT: return CVarSystemRegisterUintVar(name, initValue, flags);
    case CVAR_TYPE_FLOAT: return CVarSystemRegisterFloatVar(name, initValue, flags);
    case CVAR_TYPE_BOOL: return CVarSystemRegisterBoolVar(name, initValue, flags);
      
    default: assert(false);
  }

  return FALSE;
}

ENGINE_API CVarType CVarSystemGetVarType(const std::string& name);
ENGINE_API CVarFlags CVarSystemGetVarFlags(const std::string& name);
ENGINE_API bool8 CVarSystemHasVar(const std::string& name);

ENGINE_API bool8 CVarSystemRegisterIntVar(const std::string& name, int32 initValue, CVarFlags flags);
ENGINE_API bool8 CVarSystemRegisterUintVar(const std::string& name, uint32 initValue, CVarFlags flags);
ENGINE_API bool8 CVarSystemRegisterFloatVar(const std::string& name, float32 initValue, CVarFlags flags);
ENGINE_API bool8 CVarSystemRegisterBoolVar(const std::string& name, bool8 initValue, CVarFlags flags);

ENGINE_API const int32& CVarSystemReadInt(const std::string& name);
ENGINE_API int32& CVarSystemGetInt(const std::string& name);

ENGINE_API const uint32& CVarSystemReadUint(const std::string& name);
ENGINE_API uint32& CVarSystemGetUint(const std::string& name);

ENGINE_API const float32& CVarSystemReadFloat(const std::string& name);
ENGINE_API float32& CVarSystemGetFloat(const std::string& name);

ENGINE_API const bool8& CVarSystemReadBool(const std::string& name);
ENGINE_API bool8& CVarSystemGetBool(const std::string& name);

template <typename T>
ENGINE_API const T& CVarSystemReadVar(const std::string& name)
{
  CVarType type           = CVarTypeTrait<T>::type;
  CVarType registeredType = CVarSystemGetVarType(name);
  assert(type == registeredType && "Requested variable has another type!");

  switch(type)
  {
    case CVAR_TYPE_INT: return CVarSystemReadInt(name);
    case CVAR_TYPE_UINT: return CVarSystemReadUint(name);
    case CVAR_TYPE_INT: return CVarSystemReadFloat(name);
    case CVAR_TYPE_BOOL: return CVarSystemReadBool(name);

    default: assert(false);
  }
}

template <typename T>
ENGINE_API T& CVarSystemGetVar(const std::string& name)
{
  CVarType type           = CVarTypeTrait<T>::type;
  CVarType registeredType = CVarSystemGetVarType(name);
  assert(type == registeredType && "Requested variable has another type!");

  switch(type)
  {
    case CVAR_TYPE_INT: return CVarSystemGetInt(name);
    case CVAR_TYPE_UINT: return CVarSystemGetUint(name);
    case CVAR_TYPE_INT: return CVarSystemGetFloat(name);
    case CVAR_TYPE_BOOL: return CVarSystemGetBool(name);

    default: assert(false);
  }
}

#define DECLARE_CVAR(name, initValue) \
  DECLARE_CVAR_FLAGS(name, initValue, CVAR_FLAG_NONE)

#define DECLARE_CVAR_FLAGS(name, initValue, flags) \
  static CVarStaticInitializator StaticCVar_##name((#name), (initValue), (flags))

template <typename T>
class CVarStaticInitializator
{
public:  
  CVarStaticInitializator(const std::string& name, T initValue, CVarFlags flags)
  {
    assert(CVarSystemRegisterVar(name, initValue, (CVarFlags)(flags | CVAR_FLAG_STATIC)) == TRUE &&
           "Cannot register console variable!");
  }
};
