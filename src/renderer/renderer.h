#pragma once

#include "scene.h"
#include "camera.h"
#include "defines.h"

enum RendererShadingMode
{
  RS_VISUALIZE_DISTANCES,
  RS_VISUALIZE_NORMALS,
  RS_VISUALIZE_SHADOWS,

  RS_SIMPLE_SHADING,
  RS_PBR_SHADING,

  RS_CUSTOM_SHADING,

  RS_MAX
};

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

struct RenderingParameters
{
  RendererShadingMode shadingMode;

  uint32 rasterItersMaxCount;
  uint32 shadowRasterItersMaxCount;
  
  bool8 enableNormals;
  bool8 enableShadows;

  // TODO: Postprocessing choices
  // TODO: Tonemapper
  // TODO: RaySampler sampler;
  // TODO (sort of): void(*customShadingPathFn)(RenderingParameters params);
};

ENGINE_API bool8 intializeRenderer();
ENGINE_API void shutdownRenderer();

ENGINE_API bool8 rendererRenderScene(Scene* scene, Camera* camera, const RenderingParameters& params);
ENGINE_API GLuint rendererGetResourceHandle(RendererResourceType type);
