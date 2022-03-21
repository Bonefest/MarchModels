#pragma once

#include <../bin/shaders/declarations.h>

#include "asset.h"

static const AssetType ASSET_TYPE_LIGHT_SOURCE = 0x3da6380c;

ENGINE_API bool8 createLightSource(const std::string& name,
                                   LightSourceType type,
                                   Asset** lsource);

ENGINE_API void lightSourceSetPosition(Asset* lsource, float3 position);
ENGINE_API float3 lightSourceGetPosition(Asset* lsource);

ENGINE_API void lightSourceSetOrientation(Asset* lsource, float2 eulerAngles);
ENGINE_API float2 lightSourceGetOrientation(Asset* lsource);
ENGINE_API float4 lightSourceGetFwdDirection(Asset* lsrouce, float4 direction);

ENGINE_API void lightSourceSetAttenuationDistanceFactors(Asset* lsource, float2 attDistance);
ENGINE_API float2 lightSourceGetAttenuationDistanceFactors(Asset* lsource);

ENGINE_API void lightSourceSetIntensity(Asset* lsource, float4 intensity);
ENGINE_API float4 lightSourceGetIntensity(Asset* lsource);

ENGINE_API void lightSourceSetAttenuationAngleFactors(Asset* lsource, float2 attAngle);
ENGINE_API float2 lightSourceGetAttenuationAngleFactors(Asset* lsource);

ENGINE_API void lightSourceSetShadowFactor(Asset* lsource, float32 factor);
ENGINE_API float32 lightSourceGetShadowFactor(Asset* lsource);

ENGINE_API void lightSourceSetType(Asset* lsource, LightSourceType type);
ENGINE_API LightSourceType lightSourceGetType(Asset* lsource);

ENGINE_API void lightSourceSetEnabled(Asset* lsource, bool8 enabled);
ENGINE_API bool8 lightSourceIsEnabled(Asset* lsource);

ENGINE_API void lightSourceSetSelected(Asset* lsource, bool8 selected);
ENGINE_API bool8 lightSourceIsSelected(Asset* lsource);

ENGINE_API void lightSourceSetShadowEnabled(Asset* lsource, bool8 enabled);
ENGINE_API bool8 lightSourceShadowIsEnabled(Asset* lsource);

ENGINE_API LightSourceParameters& lightSourceGetParameters(Asset* lsource);
