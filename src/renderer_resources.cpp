#include <../bin/shaders/declarations.h>

#include "logging.h"
#include "renderer_resources.h"

struct RendererResources
{
  GLuint handles[RR_MAX];
  
  bool8 initialized = FALSE;
};

static RendererResources data;

#define INIT(funcName)                                                  \
  if(funcName() == FALSE)                                               \
  {                                                                     \
    LOG_ERROR("RendererResources initialization failed: '%s' returned FALSE", #funcName); \
    return FALSE;                                                       \
  }                                                                     \

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

bool8 initializeRendererResources()
{
  if(data.initialized == TRUE)
  {
    LOG_ERROR("Renderer resources are already initialized!");
    return FALSE;
  }
  
  glCreateVertexArrays(1, &data.handles[RR_EMPTY_VAO]);

  INIT(initStacksSSBO);
  INIT(initGlobalParamsUBO);
  INIT(initGeometryTransformParamsUBO);
  INIT(initRaysMapTexture);

  data.initialized = TRUE;
  
  return TRUE;
}

GLuint getRendererResourceHandle(RendererResourceType type)
{
  assert(data.initialized == TRUE);
  
  return data.handles[type];
}
