#include "program.h"
#include "memory_manager.h"
#include "shader_manager.h"
#include "renderer/renderer.h"
#include "renderer/renderer_utils.h"
#include <../bin/shaders/declarations.h>

#include "rasterization_pass.h"

DECLARE_CVAR(engine_RasterizationStatistics_LastFrameCulledObjects, 0u);

struct RasterizationPassData
{
  GLuint raysMapFBO;
  GLuint geometryAndDistancesFBO;
  
  ShaderProgram* preparingProgram;
  ShaderProgram* raysMoverProgram;
  ShaderProgram* resultsExtractionProgram;
};

static void destroyRasterizationPass(RenderPass* pass)
{
  RasterizationPassData* data = (RasterizationPassData*)renderPassGetInternalData(pass);
  glDeleteFramebuffers(1, &data->raysMapFBO);
  glDeleteFramebuffers(1, &data->geometryAndDistancesFBO);
  
  destroyShaderProgram(data->preparingProgram);
  destroyShaderProgram(data->raysMoverProgram);
  destroyShaderProgram(data->resultsExtractionProgram);
  
  engineFreeObject(data, MEMORY_TYPE_GENERAL);
}

static bool8 rasterizationPassPrepareToRasterize(RasterizationPassData* data)
{
  shaderProgramUse(data->preparingProgram);
  glBindFramebuffer(GL_FRAMEBUFFER, data->raysMapFBO);
  glClearColor(0, 0, 0, 0);
  
  drawTriangleNoVAO();

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  shaderProgramUse(nullptr);

  return TRUE;
}

/**
 * @return boolean value which indicates whether it was rendered or not
 */
static bool8 drawGeometryInorder(Camera* camera,
                                 AssetPtr geometry,
                                 uint32 indexInBranch,
                                 uint32 culledSiblingsCount,
                                 uint32& culledObjCounter)
{
  const AABB& geometryAABB = geometryGetFinalAABB(geometry);
  if(cameraGetFrustum(camera).intersects(geometryAABB) == FALSE)
  {
    // NOTE: We've culled the object + its children
    culledObjCounter += geometryGetTotalChildrenCount(geometry) + 1;
    return FALSE;
  }

  std::vector<AssetPtr>& children = geometryGetChildren(geometry);

  // NOTE: This counter stores number of culled objects on the current level, this number is crucial for
  // some optimizations (e.g knowing that all previous siblings were culled, we know
  // that we don't need to read the stack)
  uint32 culledChildrenCount = 0;
  for(uint32 i = 0; i < children.size(); i++)
  {
    if(drawGeometryInorder(camera, children[i], i, culledChildrenCount, culledObjCounter) == FALSE)
    {
      culledChildrenCount++;
    }
    else
    {
      // NOTE: Read https://gamedev.stackexchange.com/questions/151563/synchronization-between-several-gldispatchcompute-with-same-ssbos;
      // The idea is that we need to tell OpenGL explicitly that we want to synchronize several draw calls, which are
      // reading/writing from the SSBO. Otherwise some strange artifacts may occur.
      glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }
  }

  if(!children.empty() && culledChildrenCount == children.size())
  {
    return FALSE;
  }
  
  // NOTE: If it's a root - omit further actions, because it's treated in a special way
  // (it's not a real geometry object)
  if(geometryIsRoot(geometry) == TRUE)
  {
    return TRUE;
  }
  
  ShaderProgram* geometryProgram = geometryGetDrawProgram(geometry);
  if(geometryProgram == nullptr)
  {
    return FALSE;
  }

  GeometryTransformParameters geoTransforms = {};
  geoTransforms.position = float4(geometryGetPosition(geometry), 1.0);
  geoTransforms.geoWorldMat = geometryGetGeoWorldMat(geometry);
  geoTransforms.worldGeoMat = geometryGetWorldGeoMat(geometry);
  geoTransforms.geoParentMat = geometryGetGeoParentMat(geometry);
  geoTransforms.parentGeoMat = geometryGetParentGeoMat(geometry);
  
  glBindBuffer(GL_UNIFORM_BUFFER, rendererGetResourceHandle(RR_GEOTRANSFORM_PARAMS_UBO));
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GeometryTransformParameters), &geoTransforms);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);                  

  shaderProgramUse(geometryProgram);

  glUniform1ui(glGetUniformLocation(shaderProgramGetGLHandle(geometryProgram), "geometryID"),
               geometryGetID(geometry));
  glUniform1ui(glGetUniformLocation(shaderProgramGetGLHandle(geometryProgram), "indexInBranch"),
               indexInBranch);
  glUniform1ui(glGetUniformLocation(shaderProgramGetGLHandle(geometryProgram), "prevCulledSiblingsCount"),
               culledSiblingsCount);
  
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, rendererGetResourceHandle(RR_RAYS_MAP_TEXTURE));
  
  drawTriangleNoVAO();
  
  shaderProgramUse(nullptr);

  return TRUE;
}

static bool8 rasterizationPassRasterize(RasterizationPassData* data)
{
  const RenderingParameters& renderingParams = rendererGetPassedRenderingParameters();
  static uint32& culledObjectsCounter = CVarSystemGetUint("engine_RasterizationStatistics_LastFrameCulledObjects");
  
  Scene* sceneToRasterize = rendererGetPassedScene();
  
  for(uint32 i = 0; i < renderingParams.rasterItersMaxCount; i++)
  {
    culledObjectsCounter = 0;    
    // Calculate distances
    drawGeometryInorder(rendererGetPassedCamera(),
                        sceneGetGeometryRoot(sceneToRasterize),
                        0, 0,
                        culledObjectsCounter);
    
    // Move per-pixel rays based on calculated distances
    glBindFramebuffer(GL_FRAMEBUFFER, data->raysMapFBO);
    
    glEnable(GL_BLEND);
    pushBlend(GL_FUNC_ADD, GL_FUNC_ADD, GL_ZERO, GL_ONE, GL_ONE, GL_ONE);

    shaderProgramUse(data->raysMoverProgram);

    glUniform1ui(glGetUniformLocation(shaderProgramGetGLHandle(data->raysMoverProgram), "curIterIdx"), i);
    drawTriangleNoVAO();
    shaderProgramUse(nullptr);
    
    assert(popBlend() == TRUE);
    glDisable(GL_BLEND);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
  
  return TRUE;
}

static bool8 rasterizationPassExtractResults(RasterizationPassData* data)
{
  glDepthFunc(GL_ALWAYS);
  glEnable(GL_DEPTH_TEST);
  
  glBindFramebuffer(GL_FRAMEBUFFER, data->geometryAndDistancesFBO);
  glClearDepth(1.0f);
  
  shaderProgramUse(data->resultsExtractionProgram);
  
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, rendererGetResourceHandle(RR_RAYS_MAP_TEXTURE));  
  drawTriangleNoVAO();

  shaderProgramUse(nullptr);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glDisable(GL_DEPTH_TEST);
  
  return TRUE;
}

static bool8 rasterizationPassExecute(RenderPass* pass)
{
  RasterizationPassData* data = (RasterizationPassData*)renderPassGetInternalData(pass);

  assert(rasterizationPassPrepareToRasterize(data));
  assert(rasterizationPassRasterize(data));
  assert(rasterizationPassExtractResults(data));
  
  return TRUE;
}

static const char* rasterizationPassGetName(RenderPass* pass)
{
  return "RasterizationPass";
}

static ShaderProgram* createAndLinkProgram(const char* fragmentShaderPath)
{
  ShaderProgram* program = nullptr;
  
  createShaderProgram(&program);
  shaderProgramAttachShader(program, shaderManagerGetShader("triangle.vert"));
  shaderProgramAttachShader(program, shaderManagerLoadShader(GL_FRAGMENT_SHADER, fragmentShaderPath));

  if(linkShaderProgram(program) == FALSE)
  {
    destroyShaderProgram(program);
    return nullptr;
  }
  
  return program;
}

static GLuint createRayMapFramebuffer()
{
  GLuint framebuffer = 0;
  
  glGenFramebuffers(1, &framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, rendererGetResourceHandle(RR_RAYS_MAP_TEXTURE), 0);
  // TODO: attach stencil too
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  {
    return 0;
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return framebuffer;
}

static GLuint createGeometryAndDistancesFramebuffer()
{
  GLuint framebuffer = 0;
  
  glGenFramebuffers(1, &framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, rendererGetResourceHandle(RR_GEOIDS_MAP_TEXTURE), 0);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, rendererGetResourceHandle(RR_DISTANCES_MAP_TEXTURE), 0);
  //glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, rendererGetResourceHandle(RR_DISTANCES_MAP_TEXTURE), 0);                                                                                      

  GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0};
  glNamedFramebufferDrawBuffers(framebuffer, 1, drawBuffers);
  
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  {
    return 0;
  }
  
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return framebuffer;
}

bool8 createRasterizationPass(RenderPass** outPass)
{
  RenderPassInterface interface = {};
  interface.destroy = destroyRasterizationPass;
  interface.execute = rasterizationPassExecute;
  interface.getName = rasterizationPassGetName;
  interface.type = RENDER_PASS_TYPE_RASTERIZATION;

  if(allocateRenderPass(interface, outPass) == FALSE)
  {
    return FALSE;
  }

  RasterizationPassData* data = engineAllocObject<RasterizationPassData>(MEMORY_TYPE_GENERAL);

  data->raysMapFBO = createRayMapFramebuffer();
  assert(data->raysMapFBO != 0);

  data->geometryAndDistancesFBO = createGeometryAndDistancesFramebuffer();
  assert(data->geometryAndDistancesFBO != 0);

  data->preparingProgram = createAndLinkProgram("shaders/prepare_to_raster.frag");
  assert(data->preparingProgram != nullptr);

  data->raysMoverProgram = createAndLinkProgram("shaders/rays_mover.frag");
  assert(data->raysMoverProgram != nullptr);

  data->resultsExtractionProgram = createAndLinkProgram("shaders/extract_raster_results.frag");
  assert(data->resultsExtractionProgram != nullptr);
  
  renderPassSetInternalData(*outPass, data);
  
  return TRUE;
}
