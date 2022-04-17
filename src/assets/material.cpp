#include "material.h"

struct MaterialTextureData
{
  ImagePtr texture;
  uint4 textureRegion;
  float4 atlasUVRect;
  
  bool8 enabled;
};

struct Material
{
  MaterialTextureProjectionMode projectionMode;
  
  float32 ior;
  float32 ao;
  float32 metallic;
  float32 roughness;
  
  float4 ambientColor;
  float4 diffuseColor;
  float4 specularColor;
  float4 emissionColor;

  MaterialTextureData textures[MATERIAL_TEXTURE_TYPE_COUNT];

  bool8 integratedIntoAtlas = FALSE;
};

using nlohmann::json;

static void materialDestroy(Asset* material);
static bool8 materialSerialize(AssetPtr material, json& jsonData) { }
static bool8 materialDeserialize(AssetPtr material, json& jsonData) { }
static uint32 materialGetSize(Asset* asset) { /** TODO */ }

const char* materialTextureTypeLabel(MaterialTextureType texType)
{
  switch(texType)
  {
    case MATERIAL_TEXTURE_TYPE_DIFFUSE: return "Diffuse";
    case MATERIAL_TEXTURE_TYPE_SPECULAR: return "Specular";
    case MATERIAL_TEXTURE_TYPE_BUMP: return "Bump";
    case MATERIAL_TEXTURE_TYPE_MRIAO: return "MRIAO";

    default: return "Unknown texture";
  }
}

const char* materialTextureProjectionModeLabel(MaterialTextureProjectionMode mode)
{
  switch(mode)
  {
    case MATERIAL_TEXTURE_PROJECTION_MODE_TRIPLANAR: return "Triplanar";
    case MATERIAL_TEXTURE_PROJECTION_MODE_SPHERICAL: return "Spherical";
    case MATERIAL_TEXTURE_PROJECTION_MODE_CYLINDRICAL: return "Cylyndrical";

  default: return "Unknown projection mode";
  }
}

bool8 createMaterial(const std::string& name, Asset** material)
{
  AssetInterface interface = {};
  interface.destroy = materialDestroy;
  interface.serialize = materialSerialize;
  interface.deserialize = materialDeserialize;
  interface.getSize = materialGetSize;
  interface.type = ASSET_TYPE_MATERIAL;

  assert(allocateAsset(interface, name, material));

  Material* materialData = engineAllocObject<Material>(MEMORY_TYPE_GENERAL);
  materialData->projectionMode = MATERIAL_TEXTURE_PROJECTION_MODE_TRIPLANAR;
  
  materialData->ior = 1.0f;
  materialData->ao = 0.0f;
  materialData->metallic = 0.0f;
  materialData->roughness = 1.0f;

  materialData->ambientColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
  materialData->diffuseColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
  materialData->specularColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
  materialData->emissionColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

  for(uint32 i = 0; i < MATERIAL_TEXTURE_TYPE_COUNT; i++)
  {
    materialData->textures[i].texture = ImagePtr(nullptr);
    materialData->textures[i].enabled = FALSE;
  }
  
  assetSetInternalData(*material, materialData);
}

void materialDestroy(Asset* material)
{
  Material* materialData = engineAllocObject<Material>(MEMORY_TYPE_GENERAL);
  for(uint32 i = 0; i < MATERIAL_TEXTURE_TYPE_COUNT; i++)
  {
    materialData->textures[i].texture = ImagePtr(nullptr);
  }
  
  engineFreeObject(materialData, MEMORY_TYPE_GENERAL);
}

void materialSetProjectionMode(Asset* material, MaterialTextureProjectionMode mode)
{
  Material* materialData = (Material*)assetGetInternalData(material);
  materialData->projectionMode = mode;
}

MaterialTextureProjectionMode materialGetProjectionMode(Asset* material)
{
  Material* materialData = (Material*)assetGetInternalData(material);  
  return materialData->projectionMode;
}

void materialSetTexture(Asset* material, MaterialTextureType type, ImagePtr image)
{
  Material* materialData = (Material*)assetGetInternalData(material);
  materialData->textures[type].texture = image;
  materialData->integratedIntoAtlas = FALSE;
}

ImagePtr materialGetTexture(Asset* material, MaterialTextureType type)
{
  Material* materialData = (Material*)assetGetInternalData(material);  
  return materialData->textures[type].texture;
}

void materialRemoveTexture(Asset* material, MaterialTextureType type)
{
  Material* materialData = (Material*)assetGetInternalData(material);  
  materialData->textures[type].texture = ImagePtr(nullptr);
}

bool8 materialHasTexture(Asset* material, MaterialTextureType type)
{
  Material* materialData = (Material*)assetGetInternalData(material);  
  return materialData->textures[type].texture != ImagePtr(nullptr) ? TRUE : FALSE;
}

void materialSetTextureRegion(Asset* material, MaterialTextureType type, uint4 region)
{
  Material* materialData = (Material*)assetGetInternalData(material);
  materialData->textures[type].textureRegion = region;
}

uint4 materialGetTextureRegion(Asset* material, MaterialTextureType type)
{
  Material* materialData = (Material*)assetGetInternalData(material);
  return materialData->textures[type].textureRegion;
}

void materialSetEnabledTexture(Asset* material, MaterialTextureType type, bool8 enabled)
{
  Material* materialData = (Material*)assetGetInternalData(material);    
  materialData->textures[type].enabled = enabled;
}

bool8 materialIsTextureEnabled(Asset* material, MaterialTextureType type)
{
  Material* materialData = (Material*)assetGetInternalData(material);    
  return materialData->textures[type].enabled;
}

void materialSetAmbientColor(Asset* material, float4 ambientColor)
{
  Material* materialData = (Material*)assetGetInternalData(material);
  materialData->ambientColor = ambientColor;
}

float4 materialGetAmbientColor(Asset* material)
{
  Material* materialData = (Material*)assetGetInternalData(material);
  return materialData->ambientColor;
}

void materialSetDiffuseColor(Asset* material, float4 diffuseColor)
{
  Material* materialData = (Material*)assetGetInternalData(material);
  materialData->diffuseColor = diffuseColor;
}

float4 materialGetDiffuseColor(Asset* material)
{
  Material* materialData = (Material*)assetGetInternalData(material);
  return materialData->diffuseColor;
}

void materialSetSpecularColor(Asset* material, float4 specularColor)
{
  Material* materialData = (Material*)assetGetInternalData(material);
  materialData->specularColor = specularColor;  
}

float4 materialGetSpecularColor(Asset* material)
{
  Material* materialData = (Material*)assetGetInternalData(material);
  return materialData->specularColor;
}

void materialSetEmissionColor(Asset* material, float4 emissionColor)
{
  Material* materialData = (Material*)assetGetInternalData(material);
  materialData->emissionColor = emissionColor;
}

float4 materialGetEmissionColor(Asset* material)
{
  Material* materialData = (Material*)assetGetInternalData(material);
  return materialData->emissionColor;
}

void materialSetIOR(Asset* material, float32 ior)
{
  Material* materialData = (Material*)assetGetInternalData(material);
  materialData->ior = ior;
}

float32 materialGetIOR(Asset* material)
{
  Material* materialData = (Material*)assetGetInternalData(material);
  return materialData->ior;
}

void materialSetAO(Asset* material, float32 ao)
{
  Material* materialData = (Material*)assetGetInternalData(material);
  materialData->ao = ao;
}

float32 materialGetAO(Asset* material)
{
  Material* materialData = (Material*)assetGetInternalData(material);
  return materialData->ao;
}

void materialSetMetallic(Asset* material, float32 metallic)
{
  Material* materialData = (Material*)assetGetInternalData(material);
  materialData->metallic = metallic;
}

float32 materialGetMetallic(Asset* material)
{
  Material* materialData = (Material*)assetGetInternalData(material);
  return materialData->metallic;
}

void materialSetRoughness(Asset* material, float32 roughness)
{
  Material* materialData = (Material*)assetGetInternalData(material);
  materialData->roughness = roughness;
}

float32 materialGetRoughness(Asset* material)
{
  Material* materialData = (Material*)assetGetInternalData(material);
  return materialData->roughness;
}

void materialSetTextureAtlasRect(Asset* material, MaterialTextureType type, float4 rect)
{
  Material* materialData = (Material*)assetGetInternalData(material);
  materialData->textures[type].atlasUVRect = rect;
}

float4 materialGetTextureAtlasRect(Asset* material, MaterialTextureType type)
{
  Material* materialData = (Material*)assetGetInternalData(material);
  return materialData->textures[type].atlasUVRect;
}

void materialSetIntegratedIntoAtlas(Asset* material, bool8 integrated)
{
  Material* materialData = (Material*)assetGetInternalData(material);
  materialData->integratedIntoAtlas = integrated;
}

bool8 materialIsIntegratedIntoAtlas(Asset* material)
{
  Material* materialData = (Material*)assetGetInternalData(material);
  return materialData->integratedIntoAtlas;
}
