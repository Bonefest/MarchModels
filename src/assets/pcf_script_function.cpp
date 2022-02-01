#include "memory_manager.h"
#include "pcf_script_function.h"

struct PCFData
{
  PCFNativeType nativeType = PCF_NATIVE_TYPE_UNION;
  float32 multiplier = 1.0f;
};

static void destroyPCF(Asset* pcf)
{
  PCFData* data = (PCFData*)scriptFunctionGetInternalData(pcf);
  engineFreeObject(data, MEMORY_TYPE_GENERAL);
}

static void copyPCF(Asset* dst, Asset* src)
{
  PCFData* dstData = (PCFData*)scriptFunctionGetInternalData(dst);
  PCFData* srcData = (PCFData*)scriptFunctionGetInternalData(src);
  *dstData = *srcData;
}

bool8 createPCF(const std::string& name, PCFNativeType nativeType, Asset** outAsset)
{
  ScriptFunctionInterface interface = {};
  interface.destroy = destroyPCF;
  interface.copy = copyPCF;

  if(createScriptFunctionExt(SCRIPT_FUNCTION_TYPE_PCF,
                             name,
                             interface,
                             outAsset) == FALSE)
  {
    return FALSE;
  }

  PCFData* data = engineAllocObject<PCFData>(MEMORY_TYPE_GENERAL);
  scriptFunctionSetInternalData(*outAsset, data);

  return TRUE;
}

void pcfSetNativeType(Asset* pcf, PCFNativeType type)
{
  PCFData* data = (PCFData*)scriptFunctionGetInternalData(pcf);
  data->nativeType = type;
}

PCFNativeType pcfGetnativeType(Asset* pcf)
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
