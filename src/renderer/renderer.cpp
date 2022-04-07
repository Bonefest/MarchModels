#include <../bin/shaders/declarations.h>

#include "logging.h"
#include "renderer_utils.h"
#include "billboard_system.h"

#include "passes/fog_pass.h"
#include "passes/render_pass.h"
#include "passes/rasterization_pass.h"
#include "passes/simple_shading_pass.h"
#include "passes/ldr_to_film_copy_pass.h"
#include "passes/ids_visualization_pass.h"
#include "passes/normals_calculation_pass.h"
#include "passes/shadow_rasterization_pass.h"
#include "passes/lights_visualization_pass.h"
#include "passes/normals_visualization_pass.h"
#include "passes/distances_visualization_pass.h"
#include "passes/ui_widgets_visualization_pass.h"
#include "passes/geometry_native_aabb_calculation_pass.h"

#include "renderer.h"

#define MAX_WIDTH 1280
#define MAX_HEIGHT 720

struct Renderer
{
  GLuint handles[RR_MAX];
  
  RenderPass* rasterizationPass;
  RenderPass* normalsCalculationPass;
  RenderPass* shadowRasterizationPass;

  RenderPass* distancesVisualizationPass;
  RenderPass* idsVisualizationPass;
  RenderPass* normalsVisualizationPass;
  RenderPass* shadowsVisualizationPass;
  RenderPass* uiWidgetsVisualizationPass;
  RenderPass* lightsVisualizationPass;

  RenderPass* simpleShadingPass;
  RenderPass* pbrShadingPass;

  RenderPass* fogPass;
  
  std::vector<RenderPass*> passes;
  
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
  parameters.worldSize = params.worldSize;
  
  parameters.resolution = filmGetSize(film);
  parameters.invResolution = float2(1.0f / parameters.resolution.x, 1.0f / parameters.resolution.y);

  parameters.gapResolution = uint2(parameters.resolution.x / (params.pixelGap.x + 1),
                                   parameters.resolution.y / (params.pixelGap.y + 1));
  parameters.invGapResolution = float2(1.0f / parameters.gapResolution.x, 1.0f / parameters.gapResolution.y);
  
  parameters.pixelGapX = params.pixelGap.x;
  parameters.pixelGapY = params.pixelGap.y;
  
  parameters.rasterItersMaxCount = params.rasterItersMaxCount;
  
  parameters.camPosition = float4(cameraGetPosition(camera), 1.0);
  parameters.camOrientation = cameraGetOrientation(camera);
  parameters.camNDCCameraMat = cameraGetNDCCameraMat(camera);
  parameters.camCameraNDCMat = cameraGetCameraNDCMat(camera);
  parameters.camNDCWorldMat = cameraGetNDCWorldMat(camera);
  parameters.camWorldNDCMat = cameraGetWorldNDCMat(camera);
  parameters.camCameraWorldMat = cameraGetCameraWorldMat(camera);
  parameters.camWorldCameraMat = cameraGetWorldCameraMat(camera);
  parameters.camFwdAxis = parameters.camCameraWorldMat[2];
  parameters.camSideAxis = parameters.camCameraWorldMat[0];
  parameters.camUpAxis = parameters.camCameraWorldMat[1];
  parameters.camMisc.x = cameraGetNear(camera);
  parameters.camMisc.y = cameraGetFar(camera);
  parameters.camMisc.z = cameraGetFovX(camera);
  parameters.camMisc.w = cameraGetFovY(camera);  
  
  glBindBuffer(GL_UNIFORM_BUFFER, rendererGetResourceHandle(RR_GLOBAL_PARAMS_UBO));
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GlobalParameters), &parameters);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);                    
}

static void rendererSetupGlobalLightParameters(Scene* scene)
{
  std::vector<AssetPtr> lightSources = sceneGetEnabledLightSources(scene);  
  std::vector<LightSourceParameters> parameters(MAX_LIGHT_SOURCES_COUNT);

  for(uint32 i = 0; i < std::min<uint32>(MAX_LIGHT_SOURCES_COUNT, lightSources.size()); i++)
  {
    parameters[i] = lightSourceGetParameters(lightSources[i]);
  }

  uint32 offset = sizeof(GlobalParameters);

  glBindBuffer(GL_UNIFORM_BUFFER, rendererGetResourceHandle(RR_GLOBAL_PARAMS_UBO));
  glBufferSubData(GL_UNIFORM_BUFFER, offset,
                  sizeof(LightSourceParameters) * parameters.size(),
                  &parameters[0]);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

}

// ----------------------------------------------------------------------------
// Resources initialization / destroying
// ----------------------------------------------------------------------------

static bool8 initStacksSSBO()
{
  glGenBuffers(1, &data.handles[RR_DISTANCES_STACK_SSBO]);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, data.handles[RR_DISTANCES_STACK_SSBO]);
  glBufferData(GL_SHADER_STORAGE_BUFFER, GEOMETRY_STACK_MEMBERS_COUNT * MAX_WIDTH * MAX_HEIGHT * sizeof(float32), NULL, GL_DYNAMIC_COPY);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, STACKS_SSBO_BINDING, data.handles[RR_DISTANCES_STACK_SSBO]);

  return TRUE;
}

static bool8 initGlobalParamsUBO()
{
  glGenBuffers(1, &data.handles[RR_GLOBAL_PARAMS_UBO]);
  glBindBuffer(GL_UNIFORM_BUFFER, data.handles[RR_GLOBAL_PARAMS_UBO]);
  glBufferData(GL_UNIFORM_BUFFER,
               sizeof(GlobalParameters) + MAX_LIGHT_SOURCES_COUNT * sizeof(LightSourceParameters),
               NULL, GL_DYNAMIC_DRAW);
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

static bool8 initCoverageMaskTexture()
{
  glGenTextures(1, &data.handles[RR_COVERAGE_MASK_TEXTURE]);
  glBindTexture(GL_TEXTURE_2D, data.handles[RR_COVERAGE_MASK_TEXTURE]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8,
               MAX_WIDTH, MAX_HEIGHT, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
  glBindTexture(GL_TEXTURE_2D, 0);

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

static bool8 initDepthMapTexture()
{
  glGenTextures(2, &data.handles[RR_DEPTH1_MAP_TEXTURE]);
  
  glBindTexture(GL_TEXTURE_2D, data.handles[RR_DEPTH1_MAP_TEXTURE]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, MAX_WIDTH, MAX_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

  glBindTexture(GL_TEXTURE_2D, data.handles[RR_DEPTH2_MAP_TEXTURE]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, MAX_WIDTH, MAX_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  
  glBindTexture(GL_TEXTURE_2D, 0);

  return TRUE;  
}

static bool8 initLDRMapTexture()
{
  glGenTextures(2, &data.handles[RR_LDR1_MAP_TEXTURE]);
  
  glBindTexture(GL_TEXTURE_2D, data.handles[RR_LDR1_MAP_TEXTURE]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, MAX_WIDTH, MAX_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);

  glBindTexture(GL_TEXTURE_2D, data.handles[RR_LDR2_MAP_TEXTURE]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, MAX_WIDTH, MAX_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
  
  glBindTexture(GL_TEXTURE_2D, 0);

  return TRUE;
}

static bool8 initNormalsMapTexture()
{
  glGenTextures(1, &data.handles[RR_NORMALS_MAP_TEXTURE]);
  glBindTexture(GL_TEXTURE_2D, data.handles[RR_NORMALS_MAP_TEXTURE]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, MAX_WIDTH, MAX_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
  glBindTexture(GL_TEXTURE_2D, 0);

  return TRUE;
}

static bool8 initShadowsMapTexture()
{
  glGenTextures(1, &data.handles[RR_SHADOWS_MAP_TEXTURE]);
  glBindTexture(GL_TEXTURE_2D, data.handles[RR_SHADOWS_MAP_TEXTURE]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, MAX_WIDTH, MAX_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
  glBindTexture(GL_TEXTURE_2D, 0);

  return TRUE;
}

static bool8 initializeRendererResources()
{
  glCreateVertexArrays(1, &data.handles[RR_EMPTY_VAO]);

  INIT(initStacksSSBO);
  INIT(initGlobalParamsUBO);
  INIT(initGeometryTransformParamsUBO);
  INIT(initCoverageMaskTexture);
  INIT(initRaysMapTexture);
  INIT(initGeometryIDMapTexture);
  INIT(initDepthMapTexture);
  INIT(initLDRMapTexture);
  INIT(initNormalsMapTexture);
  INIT(initShadowsMapTexture);
  
  return TRUE;
}

static void destroyRendererResources()
{
  glDeleteVertexArrays(1, &data.handles[RR_EMPTY_VAO]);
  glDeleteBuffers(1, &data.handles[RR_DISTANCES_STACK_SSBO]);
  glDeleteBuffers(1, &data.handles[RR_GLOBAL_PARAMS_UBO]);
  glDeleteBuffers(1, &data.handles[RR_GEOTRANSFORM_PARAMS_UBO]);
  glDeleteTextures(1, &data.handles[RR_COVERAGE_MASK_TEXTURE]);
  glDeleteTextures(1, &data.handles[RR_RAYS_MAP_TEXTURE]);
  glDeleteTextures(1, &data.handles[RR_GEOIDS_MAP_TEXTURE]);  
  glDeleteTextures(1, &data.handles[RR_DEPTH1_MAP_TEXTURE]);
  glDeleteTextures(1, &data.handles[RR_DEPTH2_MAP_TEXTURE]);    
  glDeleteTextures(1, &data.handles[RR_LDR1_MAP_TEXTURE]);
  glDeleteTextures(1, &data.handles[RR_LDR2_MAP_TEXTURE]);  
  glDeleteTextures(1, &data.handles[RR_SHADOWS_MAP_TEXTURE]);
}

// ----------------------------------------------------------------------------
// Passes initiailization
// ----------------------------------------------------------------------------

static bool8 initializeRenderPasses()
{
  INIT(createRasterizationPass, &data.rasterizationPass);
  INIT(createShadowRasterizationPass, &data.shadowRasterizationPass);
  INIT(createNormalsCalculationPass, &data.normalsCalculationPass);
  INIT(createDistancesVisualizationPass,
       float2(0.0f, 20.0f), float3(0.156f, 0.7, 0.06), float3(0.0f, 0.0f, 0.0f),
       &data.distancesVisualizationPass);
  INIT(createIDsVisualizationPass, &data.idsVisualizationPass);
  INIT(createNormalsVisualizationPass, &data.normalsVisualizationPass);
  INIT(createUIWidgetsVisualizationPass, &data.uiWidgetsVisualizationPass);
  INIT(createLightsVisualizationPass, &data.lightsVisualizationPass);
  INIT(createLDRToFilmCopyPass, &data.ldrToFilmPass);
  INIT(initializeAABBCalculationPass);
  INIT(createSimpleShadingPass, &data.simpleShadingPass);
  INIT(createFogPass, &data.fogPass);

  data.passes.push_back(data.rasterizationPass);
  data.passes.push_back(data.shadowRasterizationPass);
  data.passes.push_back(data.normalsCalculationPass);
  data.passes.push_back(data.distancesVisualizationPass);
  data.passes.push_back(data.idsVisualizationPass);
  data.passes.push_back(data.normalsVisualizationPass);
  data.passes.push_back(data.uiWidgetsVisualizationPass);
  data.passes.push_back(data.lightsVisualizationPass);
  data.passes.push_back(data.simpleShadingPass);
  data.passes.push_back(data.fogPass);
  
  return TRUE;
}

static void destroyRenderPasses()
{
  destroyRenderPass(data.rasterizationPass);
  destroyRenderPass(data.normalsCalculationPass);
  destroyRenderPass(data.distancesVisualizationPass);
  destroyRenderPass(data.idsVisualizationPass);
  destroyRenderPass(data.normalsVisualizationPass);
  destroyRenderPass(data.uiWidgetsVisualizationPass);
  destroyRenderPass(data.lightsVisualizationPass);
  destroyRenderPass(data.ldrToFilmPass);
  destroyRenderPass(data.simpleShadingPass);
  destroyRenderPass(data.fogPass);
  destroyAABBCalculationPass();
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
  INIT(initializeBillboardSystem);
  
  data.initialized = TRUE;
  
  return TRUE;
}

void shutdownRenderer()
{
  assert(data.initialized == TRUE);

  shutdownBillboardSystem();
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
  rendererSetupGlobalLightParameters(scene);

  pushViewport(0, 0, data.globalParameters.gapResolution.x, data.globalParameters.gapResolution.y);

  assert(renderPassExecute(data.rasterizationPass));

  if(params.enableNormals == TRUE)
  {
    assert(renderPassExecute(data.normalsCalculationPass));
  }

  if(params.enableShadows == TRUE)
  {
    assert(renderPassExecute(data.shadowRasterizationPass));
  }
  
  if(params.shadingMode == RS_VISUALIZE_DISTANCES)
  {
    assert(renderPassExecute(data.distancesVisualizationPass));
  }
  else if(params.shadingMode == RS_VISUALIZE_IDS)
  {
    assert(renderPassExecute(data.idsVisualizationPass));
  }
  else if(params.shadingMode == RS_VISUALIZE_NORMALS)
  {
    assert(renderPassExecute(data.normalsVisualizationPass));
  }
  else if(params.shadingMode == RS_SIMPLE_SHADING)
  {
    assert(renderPassExecute(data.simpleShadingPass));
    assert(renderPassExecute(data.fogPass));
  }

  if(params.showUIWidgets == TRUE)
  {
    assert(renderPassExecute(data.uiWidgetsVisualizationPass));
  }

  if(params.showLights == TRUE)
  {
    assert(renderPassExecute(data.lightsVisualizationPass));
  }
  
  if(params.showBillboards == TRUE)
  {
    billboardSystemPresent();
  }
  
  assert(popViewport() == TRUE);
  
  pushViewport(0, 0, data.globalParameters.resolution.x, data.globalParameters.resolution.y);
  
  assert(renderPassExecute(data.ldrToFilmPass));
  
  assert(popViewport() == TRUE);  
  
  return TRUE;
}

GLuint rendererGetResourceHandle(RendererResourceType type)
{
  return data.handles[type];
}

const std::vector<RenderPass*>& rendererGetPasses()
{
  return data.passes;
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

