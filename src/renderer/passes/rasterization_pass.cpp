#include "program.h"
#include "memory_manager.h"
#include "shader_manager.h"
#include "renderer/renderer.h"
#include "renderer/renderer_utils.h"
#include <../bin/shaders/declarations.h>

#include "passes_common.h"
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

static bool8 rasterizationPassRasterize(RasterizationPassData* data)
{
  const RenderingParameters& renderingParams = rendererGetPassedRenderingParameters();
  static uint32& culledObjectsCounter = CVarSystemGetUint("engine_RasterizationStatistics_LastFrameCulledObjects");
  
  Scene* sceneToRasterize = rendererGetPassedScene();

  glBindFramebuffer(GL_FRAMEBUFFER, data->raysMapFBO);

  glClearStencil(1);
  glClear(GL_STENCIL_BUFFER_BIT);  
  glEnable(GL_STENCIL_TEST);
  
  glEnable(GL_BLEND);
  pushBlend(GL_FUNC_ADD, GL_FUNC_ADD, GL_ZERO, GL_ONE, GL_ONE, GL_ONE);
  
  for(uint32 i = 0; i < renderingParams.rasterItersMaxCount; i++)
  {
    culledObjectsCounter = 0;

    // ------------------------------------------------------------------------
    // 1. Calculate distances

    // Render only fragments that did not intersected something
    // nor moved too far
    glStencilFuncSeparate(GL_FRONT_AND_BACK, GL_EQUAL, 1, 0xFF);
    glStencilOpSeparate(GL_FRONT_AND_BACK, GL_KEEP, GL_KEEP, GL_KEEP);

    
    drawGeometryPostorder(rendererGetPassedCamera(),
                          sceneGetGeometryRoot(sceneToRasterize),
                          0, 0,
                          culledObjectsCounter);

    // ------------------------------------------------------------------------
    // 2. Move per-pixel rays based on calculated distances
    
    // Move through all rays, shader will export ref value itself -->
    // replace stencil's value by exported one.
    glStencilFuncSeparate(GL_FRONT_AND_BACK, GL_ALWAYS, 1, 0xFF);
    glStencilOpSeparate(GL_FRONT_AND_BACK, GL_KEEP, GL_KEEP, GL_REPLACE);
    
    shaderProgramUse(data->raysMoverProgram);
    glUniform1ui(glGetUniformLocation(shaderProgramGetGLHandle(data->raysMoverProgram), "curIterIdx"), i);
    drawTriangleNoVAO();

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);    
  }

  assert(popBlend() == TRUE);
  glDisable(GL_BLEND);
  glDisable(GL_STENCIL_TEST);
  
  glBindFramebuffer(GL_FRAMEBUFFER, 0);  
  
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

  data->raysMapFBO = createFramebufferDS(rendererGetResourceHandle(RR_RAYS_MAP_TEXTURE),
                                         rendererGetResourceHandle(RR_COVERAGE_MASK_TEXTURE));
  assert(data->raysMapFBO != 0);

  data->geometryAndDistancesFBO = createFramebufferD(rendererGetResourceHandle(RR_GEOIDS_MAP_TEXTURE),
                                                     rendererGetResourceHandle(RR_DISTANCES_MAP_TEXTURE));
  assert(data->geometryAndDistancesFBO != 0);

  data->preparingProgram = createAndLinkTriangleShadingProgram("shaders/prepare_to_raster.frag");
  assert(data->preparingProgram != nullptr);

  data->raysMoverProgram = createAndLinkTriangleShadingProgram("shaders/rays_mover.frag");
  assert(data->raysMoverProgram != nullptr);

  data->resultsExtractionProgram = createAndLinkTriangleShadingProgram("shaders/extract_raster_results.frag");
  assert(data->resultsExtractionProgram != nullptr);
  
  renderPassSetInternalData(*outPass, data);
  
  return TRUE;
}
