#include "assets_factory.h"

#include "geometry.h"
#include "material.h"
#include "light_source.h"
#include "script_function.h"

AssetPtr createAssetFromJson(nlohmann::json& jsonData)
{
  if(jsonData.contains("type_id"))
  {
    AssetType typeId = AssetType(jsonData["type_id"]);
    Asset* asset = nullptr;
    
    switch(typeId)
    {
      case ASSET_TYPE_GEOMETRY: createGeometry("", &asset); break;
      case ASSET_TYPE_LIGHT_SOURCE: createLightSource("", 0, &asset); break;      
      case ASSET_TYPE_SCRIPT_FUNCTION:
      {
        if(ScriptFunctionType(jsonData["sf_type"]) == SCRIPT_FUNCTION_TYPE_PCF)
        {
          createPCF("", PCF_NATIVE_TYPE_INTERSECTION, &asset);
        }
        else
        {
          createScriptFunction(SCRIPT_FUNCTION_TYPE_SDF, "", &asset);
        }
      } break;
      case ASSET_TYPE_MATERIAL: createMaterial("", &asset); break;
      default:
      {
        LOG_ERROR("Attempt to create asset from a json with unknown ID!");
        return AssetPtr(nullptr);
      }
    }

    AssetPtr assetPtr = AssetPtr(asset);

    if(assetDeserialize(assetPtr, jsonData) == FALSE)
    {
      LOG_ERROR("Cannot create asset from a json because given json has wrong data!");
      return AssetPtr(nullptr);
    }

    return assetPtr;
  }

  return AssetPtr(nullptr);
}
