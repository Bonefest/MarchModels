#include "light_source.h"

static void lightSourceDestroy(Asset* asset);
static bool8 lightSourceSerialize(Asset* asset) { /** TODO */ }
static bool8 lightSourceDeserialize(Asset* asset) { /** TODO */ }
static uint32 lightSourceGetSize(Asset* asset) { /** TODO */ }

struct LightSource
{
  LightSourceParameters parameters;
  float2 orientation;
  bool8 selected = FALSE;
};

bool8 createLightSource(const std::string& name,
                        LightSourceType type,
                        Asset** lsource)
{
  AssetInterface interface = {};
  interface.destroy = lightSourceDestroy;
  interface.serialize = lightSourceSerialize;
  interface.deserialize = lightSourceDeserialize;
  interface.getSize = lightSourceGetSize;
  interface.type = ASSET_TYPE_LIGHT_SOURCE;

  assert(allocateAsset(interface, name, lsource));

  LightSource* lsourceData = engineAllocObject<LightSource>(MEMORY_TYPE_GENERAL);
  lsourceData->parameters.type = type;
  lsourceData->parameters.intensity = float4(1.0f, 1.0f, 1.0f, 1.0f);
  lsourceData->parameters.attenuationDistanceFactors.y = 1.0f;
  lsourceData->parameters.forward = float4(0.0f, 0.0f, 1.0f, 0.0f);
  
  assetSetInternalData(*lsource, lsourceData);
  
  return TRUE;
}

void lightSourceDestroy(Asset* lsource)
{
  LightSource* lsourceData = (LightSource*)assetGetInternalData(lsource);

  engineFreeObject(lsourceData, MEMORY_TYPE_GENERAL);
}

void lightSourceSetPosition(Asset* lsource, float3 position)
{
  LightSource* lsourceData = (LightSource*)assetGetInternalData(lsource);

  lsourceData->parameters.position = float4(position, 0.0);
}

float3 lightSourceGetPosition(Asset* lsource)
{
  LightSource* lsourceData = (LightSource*)assetGetInternalData(lsource);

  return lsourceData->parameters.position.xyz();
}

void lightSourceSetOrientation(Asset* lsource, float2 eulerAngles)
{
  LightSource* lsourceData = (LightSource*)assetGetInternalData(lsource);
  lsourceData->orientation = eulerAngles;

  float32 cosPitch = cos(eulerAngles.y);
  
  lsourceData->parameters.forward = float4(cosPitch * sin(eulerAngles.x),
                                           sin(eulerAngles.y),
                                           cosPitch * cos(eulerAngles.x),
                                           0.0f);

}

float2 lightSourceGetOrientation(Asset* lsource)
{
  LightSource* lsourceData = (LightSource*)assetGetInternalData(lsource);
  return lsourceData->orientation;
}

float4 lightSourceGetFwdDirection(Asset* lsource)
{
  LightSource* lsourceData = (LightSource*)assetGetInternalData(lsource);

  return lsourceData->parameters.forward;
}

void lightSourceSetAttenuationDistanceFactors(Asset* lsource, float2 attDistance)
{
  LightSource* lsourceData = (LightSource*)assetGetInternalData(lsource);

  lsourceData->parameters.attenuationDistanceFactors = attDistance;
}

float2 lightSourceGetAttenuationDistanceFactors(Asset* lsource)
{
  LightSource* lsourceData = (LightSource*)assetGetInternalData(lsource);

  return lsourceData->parameters.attenuationDistanceFactors;
}

void lightSourceSetIntensity(Asset* lsource, float4 intensity)
{
  LightSource* lsourceData = (LightSource*)assetGetInternalData(lsource);

  lsourceData->parameters.intensity = intensity;
}

float4 lightSourceGetIntensity(Asset* lsource)
{
  LightSource* lsourceData = (LightSource*)assetGetInternalData(lsource);

  return lsourceData->parameters.intensity;
}

void lightSourceSetAttenuationAngleFactors(Asset* lsource, float2 attAngle)
{
  LightSource* lsourceData = (LightSource*)assetGetInternalData(lsource);

  lsourceData->parameters.attenuationAngleFactors = attAngle;
}

float2 lightSourceGetAttenuationAngleFactors(Asset* lsource)
{
  LightSource* lsourceData = (LightSource*)assetGetInternalData(lsource);

  return lsourceData->parameters.attenuationAngleFactors;
}

void lightSourceSetShadowFactor(Asset* lsource, float32 factor)
{
  LightSource* lsourceData = (LightSource*)assetGetInternalData(lsource);

  lsourceData->parameters.shadowFactor = factor;
}

float32 lightSourceGetShadowFactor(Asset* lsource)
{
  LightSource* lsourceData = (LightSource*)assetGetInternalData(lsource);

  return lsourceData->parameters.shadowFactor;
}

void lightSourceSetType(Asset* lsource, LightSourceType type)
{
  LightSource* lsourceData = (LightSource*)assetGetInternalData(lsource);

  lsourceData->parameters.type = (uint32)type;
}

LightSourceType lightSourceGetType(Asset* lsource)
{
  LightSource* lsourceData = (LightSource*)assetGetInternalData(lsource);

  return (LightSourceType)lsourceData->parameters.type;
}

void lightSourceSetEnabled(Asset* lsource, bool8 enabled)
{
  LightSource* lsourceData = (LightSource*)assetGetInternalData(lsource);

  lsourceData->parameters.enabled = enabled;
}

bool8 lightSourceIsEnabled(Asset* lsource)
{
  LightSource* lsourceData = (LightSource*)assetGetInternalData(lsource);

  return lsourceData->parameters.enabled;
}

void lightSourceSetSelected(Asset* lsource, bool8 selected)
{
  LightSource* lsourceData = (LightSource*)assetGetInternalData(lsource);

  lsourceData->selected = selected;
}

bool8 lightSourceIsSelected(Asset* lsource)
{
  LightSource* lsourceData = (LightSource*)assetGetInternalData(lsource);

  return lsourceData->selected;
}

void lightSourceSetShadowEnabled(Asset* lsource, bool8 enabled)
{
  LightSource* lsourceData = (LightSource*)assetGetInternalData(lsource);

  lsourceData->parameters.shadowEnabled = enabled;
}

bool8 lightSourceShadowIsEnabled(Asset* lsource)
{
  LightSource* lsourceData = (LightSource*)assetGetInternalData(lsource);

  return lsourceData->parameters.shadowEnabled;
}

LightSourceParameters& lightSourceGetParameters(Asset* lsource)
{
  LightSource* lsourceData = (LightSource*)assetGetInternalData(lsource);

  return lsourceData->parameters;
}
