#pragma once

#include "image.h"
#include "asset.h"

static const AssetType ASSET_TYPE_MATERIAL = 0x89c5e3b6;

enum MaterialTextureType
{
  MATERIAL_TEXTURE_TYPE_DIFFUSE,
  MATERIAL_TEXTURE_TYPE_SPECULAR,
  MATERIAL_TEXTURE_TYPE_BUMP,
  MATERIAL_TEXTURE_TYPE_MRIAO,

  MATERIAL_TEXTURE_TYPE_COUNT
};

enum MaterialTextureProjectionMode
{
  MATERIAL_TEXTURE_PROJECTION_MODE_TRIPLANAR,
  MATERIAL_TEXTURE_PROJECTION_MODE_SPHERICAL,
  MATERIAL_TEXTURE_PROJECTION_MODE_CYLINDRICAL
};
  
ENGINE_API const char* materialTextureTypeLabel(MaterialTextureType texType);
ENGINE_API const char* materialTextureProjectionModeLabel(MaterialTextureProjectionMode mode);

ENGINE_API bool8 createMaterial(const std::string& name, Asset** material);

ENGINE_API void materialSetProjectionMode(Asset* material, MaterialTextureProjectionMode mode);
ENGINE_API MaterialTextureProjectionMode materialGetProjectionMode(Asset* material);

ENGINE_API void materialSetTexture(Asset* material, MaterialTextureType type, ImagePtr texture);
ENGINE_API ImagePtr materialGetTexture(Asset* material, MaterialTextureType type);
ENGINE_API void materialRemoveTexture(Asset* material, MaterialTextureType type);
ENGINE_API bool8 materialHasTexture(Asset* material, MaterialTextureType type);

ENGINE_API void materialSetTextureRegion(Asset* material, MaterialTextureType type, uint4 region);
ENGINE_API uint4 materialGetTextureRegion(Asset* material, MaterialTextureType type);

ENGINE_API void materialSetEnabledTexture(Asset* material, MaterialTextureType type, bool8 enabled);
ENGINE_API bool8 materialIsTextureEnabled(Asset* material, MaterialTextureType type);

ENGINE_API void materialSetAmbientColor(Asset* material, float4 ambientColor);
ENGINE_API float4 materialGetAmbientColor(Asset* material);

ENGINE_API void materialSetDiffuseColor(Asset* material, float4 diffuseColor);
ENGINE_API float4 materialGetDiffuseColor(Asset* material);

ENGINE_API void materialSetSpecularColor(Asset* material, float4 specularColor);
ENGINE_API float4 materialGetSpecularColor(Asset* material);

ENGINE_API void materialSetEmissionColor(Asset* material, float4 emissionColor);
ENGINE_API float4 materialGetEmissionColor(Asset* material);

ENGINE_API void materialSetIOR(Asset* material, float32 ior);
ENGINE_API float32 materialGetIOR(Asset* material);

ENGINE_API void materialSetAO(Asset* material, float32 ao);
ENGINE_API float32 materialGetAO(Asset* material);

ENGINE_API void materialSetMetallic(Asset* material, float32 metallic);
ENGINE_API float32 materialGetMetallic(Asset* material);

ENGINE_API void materialSetRoughness(Asset* material, float32 roughness);
ENGINE_API float32 materialGetRoughness(Asset* material);

// ENGINE_API SMaterial materialToShaderMaterial(Asset* material);
