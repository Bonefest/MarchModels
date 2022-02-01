/**
 * PCF Script function contains additional functions for working with
 * primitive combination script functions. It simply extends API of the
 * original script function, hence can be used in all places where script
 * function is used.
 */

#include "script_function.h"

enum PCFNativeType
{
  PCF_NATIVE_TYPE_INTERSECTION,
  PCF_NATIVE_TYPE_UNION,
  PCF_NATIVE_TYPE_SUBTRACTION
};

ENGINE_API bool8 createPCF(const std::string& name, PCFNativeType nativeType, Asset** outAsset);

ENGINE_API void pcfSetNativeType(Asset* pcf, PCFNativeType type);
ENGINE_API PCFNativeType pcfGetnativeType(Asset* pcf);

ENGINE_API void pcfSetBoundingMultiplier(Asset* pcf, float32 multiplier);
ENGINE_API float32 pcfGetBoundingMultiplier(Asset* pcf);
