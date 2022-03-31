#include <imgui/imgui.h>

#include "memory_manager.h"
#include "pcf_script_function.h"

struct PCFData
{
  PCFNativeType nativeType = PCF_NATIVE_TYPE_UNION;
  float32 multiplier = 1.0f;
};

const char* pcfNativeTypeGetLabel(PCFNativeType type)
{
  switch(type)
  {
    case PCF_NATIVE_TYPE_INTERSECTION: return "intersection";
    case PCF_NATIVE_TYPE_UNION: return "union";
    case PCF_NATIVE_TYPE_SUBTRACTION: return "subtraction";
    default: assert(false);
  }

  return nullptr;
}

const char* pcfNativeTypeGetIcon(PCFNativeType type)
{
  switch(type)
  {
    case PCF_NATIVE_TYPE_INTERSECTION: return ICON_KI_STAR_HALF;
    case PCF_NATIVE_TYPE_UNION: return ICON_KI_STAR;
    case PCF_NATIVE_TYPE_SUBTRACTION: return ICON_KI_STAR_O;
    default: assert(false);
  }

  return nullptr;  
}

static void destroyPCF(Asset* pcf)
{
  PCFData* data = (PCFData*)scriptFunctionGetInternalData(pcf);
  engineFreeObject(data, MEMORY_TYPE_GENERAL);
}

static void copyPCF(Asset* dst, Asset* src)
{
  PCFData* dstData = (PCFData*)scriptFunctionGetInternalData(dst);
  if(dstData == nullptr)
  {
    dstData = engineAllocObject<PCFData>(MEMORY_TYPE_GENERAL);
    scriptFunctionSetInternalData(dst, dstData);
  }
  
  PCFData* srcData = (PCFData*)scriptFunctionGetInternalData(src);
  assert(srcData != nullptr);
  
  *dstData = *srcData;
}

static bool8 serializePCF(AssetPtr pcf, nlohmann::json& jsonData)
{
  PCFData* data = (PCFData*)scriptFunctionGetInternalData(pcf);
  jsonData["pcf_native_type"] = data->nativeType;
  jsonData["multiplier"] = data->multiplier;

  return TRUE;
}

static bool8 deserializePCF(AssetPtr pcf, nlohmann::json& jsonData)
{
  PCFData* data = (PCFData*)scriptFunctionGetInternalData(pcf);
  data->nativeType = jsonData.value("pcf_native_type", PCF_NATIVE_TYPE_INTERSECTION);
  data->multiplier = jsonData.value("multiplier", 0.0f);

  return TRUE;
}

bool8 createPCF(const std::string& name, PCFNativeType nativeType, Asset** outAsset)
{
  ScriptFunctionInterface interface = {};
  interface.destroy = destroyPCF;
  interface.copy = copyPCF;
  interface.serialize = serializePCF;
  interface.deserialize = deserializePCF;

  if(createScriptFunctionExt(SCRIPT_FUNCTION_TYPE_PCF,
                             name,
                             interface,
                             outAsset) == FALSE)
  {
    return FALSE;
  }

  PCFData* data = engineAllocObject<PCFData>(MEMORY_TYPE_GENERAL);
  data->nativeType = nativeType;
  scriptFunctionSetInternalData(*outAsset, data);

  return TRUE;
}

void pcfSetNativeType(Asset* pcf, PCFNativeType type)
{
  PCFData* data = (PCFData*)scriptFunctionGetInternalData(pcf);
  data->nativeType = type;
}

PCFNativeType pcfGetNativeType(Asset* pcf)
{
  PCFData* data = (PCFData*)scriptFunctionGetInternalData(pcf);
  return data->nativeType;
}

void pcfSetBoundingMultiplier(Asset* pcf, float32 multiplier)
{
  PCFData* data = (PCFData*)scriptFunctionGetInternalData(pcf);
  data->multiplier = multiplier;
}

float32 pcfGetBoundingMultiplier(Asset* pcf)
{
  PCFData* data = (PCFData*)scriptFunctionGetInternalData(pcf);
  return data->multiplier;
}
