#pragma once

#include <../bin/shaders/declarations.h>

#include "asset.h"

static const AssetType ASSET_TYPE_LIGHT_SOURCE = 0x3da6380c;

enum LightSourceType
{
  LIGHT_SOURCE_TYPE_DIRECTIONAL,
  LIGHT_SOURCE_TYPE_SPOT,
  LIGHT_SOURCE_TYPE_POINT
};

ENGINE_API bool8 createLightSource(const std::string& name,
                                   LightSourceType type,
                                   Asset** lsource);

ENGINE_API void lightSourceSetPosition(Asset* lsource, float3 position);
ENGINE_API float3 lightSourceGetPosition(Asset* lsource);

ENGINE_API void lightSourceSetOrientation(Asset* lsource, quat orientation);
ENGINE_API quat lightSourceGetOrientation(Asset* lsource);

ENGINE_API void lightSourceSetAttenuationDistance(Asset* lsource, float2 attDistance);
ENGINE_API float2 lightSourceGetAttenuationDistance(Asset* lsource);

ENGINE_API void lightSourceSetIntensity(Asset* lsource, float4 intensity);
ENGINE_API float4 lightSourceGetIntensity(Asset* lsource);

ENGINE_API void lightSourceSetAttenuationAngle(Asset* lsource, float2 attAngle);
ENGINE_API float2 lightSourceGetAttenuationAngle(Asset* lsource);

ENGINE_API void lightSourceSetShadowFactor(Asset* lsource, float32 factor);
ENGINE_API float32 lightSourceGetShadowFactor(Asset* lsource);

ENGINE_API void lightSourceSetType(Asset* lsource, LightSourceType type);
ENGINE_API LightSourceType lightSourceGetType(Asset* lsource);

ENGINE_API void lightSourceSetEnabled(Asset* lsource, bool8 enabled);
ENGINE_API bool8 lightSourceIsEnabled(Asset* lsource);

ENGINE_API void lightSourceSetShadowEnabled(Asset* lsource, bool8 enabled);
ENGINE_API bool8 lightSourceIsShadowEnabled(Asset* lsource);

ENGINE_API const LightSourceParameters& lightSourceGetParameters(Asset* lsource);
