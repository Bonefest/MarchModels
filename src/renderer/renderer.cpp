#include <../bin/shaders/declarations.h>

#include "logging.h"
#include "passes/render_pass.h"

#include "renderer.h"

struct Renderer
{
  GLuint handles[RR_MAX];
  
  RenderPass* rasterizationPreparingPass;
  RenderPass* shadowRasterizationPreparingPass;
  RenderPass* rasterizationRaysUpdatePass;
  
  RenderPass* rasterizationPass;
  RenderPass* normalsCalculationPass;
  RenderPass* rasterizationDataExtractionPass;

  RenderPass* distancesVisualizationPass;
  RenderPass* normalsVisualizationPass;
  RenderPass* shadowsVisualizationPass;

  RenderPass* simpleShadingPass;
  RenderPass* pbrShadingPass;

  // std::vector<RenderPass*> tonemapperPasses;
  // std::vector<RenderPass*> postprocessPasses;
  
  bool8 initialized = FALSE;
};

static Renderer data;

// ----------------------------------------------------------------------------
// Helpers
// ----------------------------------------------------------------------------

#define INIT(funcName)                                                  \
  if(funcName() == FALSE)                                               \
  {                                                                     \
    LOG_ERROR("Initialization of '%s' has failed!", #funcName);  \
    return FALSE;                                                       \
  }                                                                     \

// ----------------------------------------------------------------------------
// Resources initialization / destroying
// ----------------------------------------------------------------------------

static bool8 initStacksSSBO()
{
  glGenBuffers(1, &data.handles[RR_DISTANCES_STACK_SSBO]);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, data.handles[RR_DISTANCES_STACK_SSBO]);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(DistancesStack) * 1280 * 720, NULL, GL_DYNAMIC_COPY);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, STACKS_SSBO_BINDING, data.handles[RR_DISTANCES_STACK_SSBO]);

  return TRUE;
}

static bool8 initGlobalParamsUBO()
{
  glGenBuffers(1, &data.handles[RR_GLOBAL_PARAMS_UBO]);
  glBindBuffer(GL_UNIFORM_BUFFER, data.handles[RR_GLOBAL_PARAMS_UBO]);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(GlobalParameters), NULL, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
  glBindBufferBase(GL_UNIFORM_BUFFER, GLOBAL_PARAMS_UBO_BINDING, data.handles[RR_GLOBAL_PARAMS_UBO]);

  return TRUE;
}

static bool8 initGeometryTransformParamsUBO()
{
  glGenBuffers(1, &data.handles[RR_GEOTRANSFORM_PARAMS_UBO]);
  glBindBuffer(GL_UNIFORM_BUFFER, data.handles[RR_GEOTRANSFORM_PARAMS_UBO]);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(GeometryTransformParameters), NULL, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
  glBindBufferBase(GL_UNIFORM_BUFFER, GEOMETRY_TRANSFORMS_UBO_BINDING, data.handles[RR_GEOTRANSFORM_PARAMS_UBO]);

  return TRUE;
}

static bool8 initRaysMapTexture()
{
  glGenTextures(1, &data.handles[RR_RAYS_MAP_TEXTURE]);
  glBindTexture(GL_TEXTURE_2D, data.handles[RR_RAYS_MAP_TEXTURE]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1280, 720, 0, GL_RGBA, GL_FLOAT, NULL);
  glBindTexture(GL_TEXTURE_2D, 0);

  return TRUE;
}

static bool8 initializeRendererResources()
{
  glCreateVertexArrays(1, &data.handles[RR_EMPTY_VAO]);

  INIT(initStacksSSBO);
  INIT(initGlobalParamsUBO);
  INIT(initGeometryTransformParamsUBO);
  INIT(initRaysMapTexture);

  return TRUE;
}

static void destroyRendererResources()
{
  glDeleteVertexArrays(1, &data.handles[RR_EMPTY_VAO]);
  glDeleteBuffers(1, &data.handles[RR_DISTANCES_STACK_SSBO]);
  glDeleteBuffers(1, &data.handles[RR_GLOBAL_PARAMS_UBO]);
  glDeleteBuffers(1, &data.handles[RR_GEOTRANSFORM_PARAMS_UBO]);
  glDeleteTextures(1, &data.handles[RR_RAYS_MAP_TEXTURE]);
}

// ----------------------------------------------------------------------------
// Passes initiailization
// ----------------------------------------------------------------------------

static bool8 initializeRenderPasses()
{
  return TRUE;
}

bool8 intializeRenderer()
{
  if(data.initialized == TRUE)
  {
    LOG_ERROR("Renderer is already initialized!");
    return FALSE;
  }

  INIT(initializeRendererResources);

  data.initialized = TRUE;
  
  return TRUE;
}

void shutdownRenderer()
{
  assert(data.initialized == TRUE);
  
  destroyRendererResources();
  
  data = Renderer{};
}

GLuint getRendererResourceHandle(RendererResourceType type)
{
  assert(data.initialized == TRUE);
  
  return data.handles[type];
}

