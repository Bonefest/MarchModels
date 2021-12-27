#include <../bin/shaders/declarations.h>

#include "logging.h"
#include "renderer_utils.h"
#include "passes/render_pass.h"
#include "passes/rasterization_pass.h"
#include "passes/ldr_to_film_copy_pass.h"
#include "passes/ids_visualization_pass.h"
#include "passes/distances_visualization_pass.h"

#include "renderer.h"

#define MAX_WIDTH 1280
#define MAX_HEIGHT 720

struct Renderer
{
  GLuint handles[RR_MAX];
  
  RenderPass* rasterizationPass;
  RenderPass* normalsCalculationPass;

  RenderPass* distancesVisualizationPass;
  RenderPass* idsVisualizationPass;
  RenderPass* normalsVisualizationPass;
  RenderPass* shadowsVisualizationPass;

  RenderPass* simpleShadingPass;
  RenderPass* pbrShadingPass;

  // std::vector<RenderPass*> tonemapperPasses;
  // std::vector<RenderPass*> postprocessPasses;

  RenderPass* ldrToFilmPass;
  
  GlobalParameters globalParameters;

  Film*               passedFilm;
  Camera*             passedCamera;
  Scene*              passedScene;
  RenderingParameters passedRenderingParams;
  
  bool8 initialized = FALSE;
};

static Renderer data;

// ----------------------------------------------------------------------------
// Helpers
// ----------------------------------------------------------------------------

#define INIT(funcName, ...)                                             \
  if(funcName(__VA_ARGS__) == FALSE)                                    \
  {                                                                     \
    LOG_ERROR("Initialization of '%s' has failed!", #funcName);         \
    return FALSE;                                                       \
  }                                                                     \

static void rendererSetupGlobalParameters(Film* film,
                                          Scene* scene,
                                          Camera* camera,
                                          const RenderingParameters& params)
{
  GlobalParameters& parameters = data.globalParameters;
  
  parameters.time = params.time;
  parameters.tone = params.tone;
  parameters.gamma = params.gamma;
  parameters.invGamma = 1.0 / params.gamma;

  parameters.intersectionThreshold = params.intersectionThreshold;
  
  parameters.resolution = filmGetSize(film);
  parameters.invResolution = float2(1.0f / parameters.resolution.x, 1.0f / parameters.resolution.y);

  parameters.pixelGapX = 0;
  parameters.pixelGapY = 0;      
  parameters.rasterItersMaxCount = params.rasterItersMaxCount;
  
  parameters.camPosition = float4(cameraGetPosition(camera), 1.0);
  parameters.camOrientation = cameraGetOrientation(camera);
  parameters.camNDCCameraMat = cameraGetNDCCameraMat(camera);
  parameters.camCameraNDCMat = cameraGetCameraNDCMat(camera);
  parameters.camNDCWorldMat = cameraGetNDCWorldMat(camera);
  parameters.camWorldNDCMat = cameraGetWorldNDCMat(camera);
  parameters.camCameraWorldMat = cameraGetCameraWorldMat(camera);
  parameters.camWorldCameraMat = cameraGetWorldCameraMat(camera);

  glBindBuffer(GL_UNIFORM_BUFFER, rendererGetResourceHandle(RR_GLOBAL_PARAMS_UBO));
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GlobalParameters), &parameters);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);                    
}

// ----------------------------------------------------------------------------
// Resources initialization / destroying
// ----------------------------------------------------------------------------

static bool8 initStacksSSBO()
{
  glGenBuffers(1, &data.handles[RR_DISTANCES_STACK_SSBO]);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, data.handles[RR_DISTANCES_STACK_SSBO]);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GeometriesStack) * MAX_WIDTH * MAX_HEIGHT, NULL, GL_DYNAMIC_COPY);
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
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, MAX_WIDTH, MAX_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
  glBindTexture(GL_TEXTURE_2D, 0);

  return TRUE;
}

static bool8 initGeometryIDMapTexture()
{
  glGenTextures(1, &data.handles[RR_GEOIDS_MAP_TEXTURE]);
  glBindTexture(GL_TEXTURE_2D, data.handles[RR_GEOIDS_MAP_TEXTURE]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R16UI, MAX_WIDTH, MAX_HEIGHT, 0, GL_RED_INTEGER, GL_UNSIGNED_SHORT, NULL);
  glBindTexture(GL_TEXTURE_2D, 0);

  return TRUE;  
}

static bool8 initDistancesMapTexture()
{
  glGenTextures(1, &data.handles[RR_DISTANCES_MAP_TEXTURE]);
  glBindTexture(GL_TEXTURE_2D, data.handles[RR_DISTANCES_MAP_TEXTURE]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, MAX_WIDTH, MAX_HEIGHT, 0, GL_RED, GL_FLOAT, NULL);
  glBindTexture(GL_TEXTURE_2D, 0);

  return TRUE;  
}

static bool8 initLDRMapTexture()
{
  glGenTextures(1, &data.handles[RR_LDR_MAP_TEXTURE]);
  glBindTexture(GL_TEXTURE_2D, data.handles[RR_LDR_MAP_TEXTURE]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, MAX_WIDTH, MAX_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
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
  INIT(initGeometryIDMapTexture);
  INIT(initDistancesMapTexture);
  INIT(initLDRMapTexture);
  
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
  INIT(createRasterizationPass, &data.rasterizationPass);
  INIT(createDistancesVisualizationPass,
       float2(0.0f, 20.0f), float3(0.156f, 0.7, 0.06), float3(0.0f, 0.0f, 0.0f),
       &data.distancesVisualizationPass);
  INIT(createIDsVisualizationPass, &data.idsVisualizationPass);
  INIT(createLDRToFilmCopyPass, &data.ldrToFilmPass);
  
  return TRUE;
}

static void destroyRenderPasses()
{
  destroyRenderPass(data.rasterizationPass);
  destroyRenderPass(data.distancesVisualizationPass);
  destroyRenderPass(data.idsVisualizationPass);
  destroyRenderPass(data.ldrToFilmPass);  
}

bool8 initializeRenderer()
{
  if(data.initialized == TRUE)
  {
    LOG_ERROR("Renderer is already initialized!");
    return FALSE;
  }

  INIT(initializeRendererResources);
  INIT(initializeRenderPasses);
  
  data.initialized = TRUE;
  
  return TRUE;
}

void shutdownRenderer()
{
  assert(data.initialized == TRUE);
  
  destroyRendererResources();
  destroyRenderPasses();
  
  data = Renderer{};
}

bool8 rendererRenderScene(Film* film,
                          Scene* scene,
                          Camera* camera,
                          const RenderingParameters& params)
{
  data.passedFilm = film;
  data.passedScene = scene;
  data.passedCamera = camera;
  data.passedRenderingParams = params;
  
  rendererSetupGlobalParameters(film, scene, camera, params);

  pushViewport(0, 0, data.globalParameters.resolution.x, data.globalParameters.resolution.y);

  assert(renderPassExecute(data.rasterizationPass));
  //assert(renderPassExecute(data.distancesVisualizationPass));
  assert(renderPassExecute(data.idsVisualizationPass));

  assert(renderPassExecute(data.ldrToFilmPass));
  
  assert(popViewport() == TRUE);
  
  return TRUE;
}

GLuint rendererGetResourceHandle(RendererResourceType type)
{
  return data.handles[type];
}

Film* rendererGetPassedFilm()
{
  return data.passedFilm;
}

Scene* rendererGetPassedScene()
{
  return data.passedScene;
}

Camera* rendererGetPassedCamera()
{
  return data.passedCamera;
}

const RenderingParameters& rendererGetPassedRenderingParameters()
{
  return data.passedRenderingParams;
}

