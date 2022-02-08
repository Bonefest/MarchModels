#include "light_source.h"

static void lightSourceDestroy(Asset* asset);
static bool8 lightSourceSerialize(Asset* asset) { /** TODO */ }
static bool8 lightSourceDeserialize(Asset* asset) { /** TODO */ }
static uint32 lightSourceGetSize(Asset* asset) { /** TODO */ }

struct LightSource
{
  LightSourceParameters parameters;
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

void lightSourceSetOrientation(Asset* lsource, quat orientation)
{
  LightSource* lsourceData = (LightSource*)assetGetInternalData(lsource);

  lsourceData->parameters.orientation = orientation;
}

quat lightSourceGetOrientation(Asset* lsource)
{
  LightSource* lsourceData = (LightSource*)assetGetInternalData(lsource);

  return lsourceData->parameters.orientation;
}

void lightSourceSetAttenuationDistance(Asset* lsource, float2 attDistance)
{
  LightSource* lsourceData = (LightSource*)assetGetInternalData(lsource);

  lsourceData->parameters.attenuationDistance = attDistance;
}

float2 lightSourceGetAttenuationDistance(Asset* lsource)
{
  LightSource* lsourceData = (LightSource*)assetGetInternalData(lsource);

  return lsourceData->parameters.attenuationDistance;
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

void lightSourceSetAttenuationAngle(Asset* lsource, float2 attAngle)
{
  LightSource* lsourceData = (LightSource*)assetGetInternalData(lsource);

  lsourceData->parameters.attenuationAngle = attAngle;
}

float2 lightSourceGetAttenuationAngle(Asset* lsource)
{
  LightSource* lsourceData = (LightSource*)assetGetInternalData(lsource);

  return lsourceData->parameters.attenuationAngle;
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

bool8 lightSourceIsShadowEnabled(Asset* lsource)
{
  LightSource* lsourceData = (LightSource*)assetGetInternalData(lsource);

  return lsourceData->parameters.shadowEnabled;
}

const LightSourceParameters& lightSourceGetParameters(Asset* lsource)
{
  LightSource* lsourceData = (LightSource*)assetGetInternalData(lsource);

  return lsourceData->parameters;
}
