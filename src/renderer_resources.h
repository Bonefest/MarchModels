#pragma once

#include "defines.h"

enum RendererResourceType
{
  RR_EMPTY_VAO,
  
  RR_DISTANCES_STACK_SSBO,
  
  RR_GLOBAL_PARAMS_UBO,
  RR_GEOTRANSFORM_PARAMS_UBO,
  
  RR_COVERAGE_MASK_TEXTURE,
  RR_GEOIDS_MAP_TEXTURE,
  RR_RAYS_MAP_TEXTURE,
  
  RR_DISTANCES_MAP_TEXTURE,
  RR_NORMALS_MAP_TEXTURE,
  RR_SHADOWS_MAP_TEXTURE,
  RR_TEXCOORDS_MAP_TEXTURE,
  
  RR_RADIANCE_MAP_TEXTURE,
  RR_LDR_MAP_TEXTURE,

  RR_MAX
};

bool8 initializeRendererResources();
void shutdownRendererResources();

void rendererResourcesUpdate(float32 delta);

ENGINE_API GLuint getRendererResourceHandle(RendererResourceType type);


