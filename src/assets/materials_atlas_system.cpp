#include <map>
#include <vector>

using std::map;
using std::vector;

#include "assets/material.h"
#include "assets/assets_manager.h"

#include "materials_atlas_system.h"

struct MASData
{
  ImagePtr atlas;  

  float2 filledSize;
  float2 rowSize;

  std::map<ImagePtr, float4> textureRects;
};

static MASData data;

bool8 initializeMAS()
{
  const uint32 atlasWidth = 4096;
  const uint32 atlasHeight = 4096;  
  
  GLuint atlasHandle;
  glGenTextures(1, &atlasHandle);
  glBindTexture(GL_TEXTURE_2D, atlasHandle);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, atlasWidth, atlasHeight, 0, GL_RGB, GL_FLOAT, NULL);  
  glBindTexture(GL_TEXTURE_2D, 0);

  Image* atlas;
  assert(createImage(atlasHandle, atlasWidth, atlasHeight, &atlas));
  data.atlas = ImagePtr(atlas);
  
  return TRUE;
}

void shutdownMAS()
{

}

static float4 addTextureToAtlas(ImagePtr texture)
{
  auto rectIt = data.textureRects.find(texture);
  if(rectIt != data.textureRects.end())
  {
    return rectIt->second;
  }

  // TODO: Copy given imange into atlas
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

void masUpdate()
{
  std::vector<AssetPtr> materials = assetsManagerGetAssetsByType(ASSET_TYPE_MATERIAL);
  for(AssetPtr material: materials)
  {
    if(materialIsIntegratedIntoAtlas(material) == FALSE)
    {
      for(uint32 itype = 0; itype < MATERIAL_TEXTURE_TYPE_COUNT; itype++)
      {
        MaterialTextureType type = (MaterialTextureType)itype;
        ImagePtr texture = materialGetTexture(material, type);
        
        if(texture != ImagePtr(nullptr))
        {
          materialSetTextureAtlasRect(material, type, addTextureToAtlas(texture));
        }
      }

      materialSetIntegratedIntoAtlas(material, TRUE);
    }
  }
}

ImagePtr masGetAtlas(uint32 index)
{
  return data.atlas;
}

