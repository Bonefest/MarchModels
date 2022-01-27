#pragma once

#include "film.h"
#include "scene.h"
#include "camera.h"
#include "defines.h"
#include "passes/render_pass.h"

enum RendererShadingMode
{
  RS_VISUALIZE_DISTANCES,
  RS_VISUALIZE_IDS,
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
  float32 time;
  float32 tone;
  float32 gamma = 2.2f;
  float32 intersectionThreshold = 0.1f;
  float32 worldSize = 50.0f;
  
  RendererShadingMode shadingMode = RS_VISUALIZE_DISTANCES;

  uint32 rasterItersMaxCount = 8;
  uint32 shadowRasterItersMaxCount = 8;

  uint2 pixelGap = uint2(2, 2);
  
  bool8 enableNormals = TRUE;
  bool8 enableShadows = TRUE;
  bool8 showAABB      = TRUE;

  // TODO: Coverage mask generator (pixel gap is an example of mask generator)
  // TODO: Postprocessing choices
  // TODO: Tonemapper
  // TODO: RaySampler sampler;
  // TODO (sort of): void(*customShadingPathFn)(RenderingParameters params);
};

ENGINE_API bool8 initializeRenderer();
ENGINE_API void shutdownRenderer();

ENGINE_API bool8 rendererRenderScene(Film* film,
                                     Scene* scene,
                                     Camera* camera,
                                     const RenderingParameters& params);



ENGINE_API GLuint rendererGetResourceHandle(RendererResourceType type);
ENGINE_API const std::vector<RenderPass*>& rendererGetPasses();

/**
 * @note Useful for passes, which need additional data (e.g for rasterization pass, which needs to have an access
 * to the scene).
 *
 * @warning returned data is valid only during execution of rendererRenderScene() function.
 *
 * @return pointer to object if getter executed during execution of rendererRenderScene(), otherwise nullptr
 */

/**
 * @return passed film during execution of rendererRenderScene() or nullptr
 */
ENGINE_API Film* rendererGetPassedFilm();

/**
 * @return passed scene during execution of rendererRenderScene() or nullptr
 */
ENGINE_API Scene* rendererGetPassedScene();

/**
 * @return passed camera during execution of rendererRenderScene() or nullptr
 */
ENGINE_API Camera* rendererGetPassedCamera();

/**
 * @return passed rendering parameters during last execution of rendererRenderScene()
 */
ENGINE_API const RenderingParameters& rendererGetPassedRenderingParameters();
