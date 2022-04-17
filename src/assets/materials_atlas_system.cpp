#include <map>
#include <vector>

using std::map;
using std::vector;

#include "utils.h"
#include "assets/material.h"
#include "assets/assets_manager.h"

#include "materials_atlas_system.h"

struct MASData
{
  ImagePtr atlas;  
  GLuint copyingFBO;
  
  uint32 rowHeight = 0;
  uint4 atlasInsertRegion = uint4(0, 0, 0, 0);

  std::map<ImagePtr, float4> textureRects;

  bool8 initialized = FALSE;
};

static MASData data;

bool8 initializeMAS()
{
  if(data.initialized == TRUE)
  {
    return FALSE;
  }

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

  glGenFramebuffers(1, &data.copyingFBO);

  data.initialized = TRUE;
  
  return TRUE;
}

void shutdownMAS()
{
  if(data.initialized == TRUE)
  {
    data.atlas = ImagePtr(nullptr);
    glDeleteFramebuffers(1, &data.copyingFBO);
  }
}

static float4 addTextureToAtlas(ImagePtr texture)
{
  auto rectIt = data.textureRects.find(texture);
  if(rectIt != data.textureRects.end())
  {
    return rectIt->second;
  }

  uint2 atlasSize = imageGetSize(data.atlas);
  uint2 textureSize = imageGetSize(texture);

  // If we cannot place this image at the end of the current row - move insert pointer at
  // the beginning of the next row.
  if(data.atlasInsertRegion.x + textureSize.x > atlasSize.x)
  {
    data.atlasInsertRegion.x = 0;
    data.atlasInsertRegion.y += data.rowHeight;
    assert(data.atlasInsertRegion.y < atlasSize.y);

    data.rowHeight = 0;
  }

  // Bind FBO, change color attachment to a given texture that's going to be copied, say
  // that glCopyTex... command should read from the color attachment #0 (which in turn
  // stores handle of our texture), finally perform copying.
  glBindFramebuffer(GL_FRAMEBUFFER, data.copyingFBO);
  
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, imageGetGLHandle(texture), 0);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
  
    glCopyTextureSubImage2D(imageGetGLHandle(data.atlas),
                            0,
                            data.atlasInsertRegion.x,
                            data.atlasInsertRegion.y,
                            0, 0,
                            textureSize.x, textureSize.y);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
  float4 uvRect = calculateUVRect(atlasSize,
                                  uint2(data.atlasInsertRegion.x, data.atlasInsertRegion.y),
                                  textureSize);
  data.textureRects[texture] = uvRect;
  
  data.rowHeight = max(data.rowHeight, textureSize.y);
  data.atlasInsertRegion.x += textureSize.x;
  
  return uvRect;
}

void masUpdate()
{
  std::vector<AssetPtr> materials = assetsManagerGetAssetsByType(ASSET_TYPE_MATERIAL);

  uint2 atlasSize = imageGetSize(data.atlas);
  
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
          uint4 textureRegion = materialGetTextureRegion(material, type);

          // Get offset in the atlas, calculate uv rect relatively to the origin, apply given offset
          float2 offsetUVRect = addTextureToAtlas(texture).xy();
          float4 originUVRect = calculateUVRect(atlasSize,
                                                uint2(textureRegion.x, textureRegion.y),
                                                uint2(textureRegion.z, textureRegion.w));
          
          float2 uvMin = float2(originUVRect.x, originUVRect.y) + offsetUVRect;
          float2 uvMax = float2(originUVRect.z, originUVRect.w) + offsetUVRect;

          materialSetTextureAtlasRect(material, type, float4(uvMin.x, uvMin.y, uvMax.x, uvMax.y));
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

