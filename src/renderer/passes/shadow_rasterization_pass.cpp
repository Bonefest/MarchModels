#include "program.h"
#include "memory_manager.h"
#include "shader_manager.h"
#include "renderer/renderer.h"
#include "renderer/renderer_utils.h"
#include <../bin/shaders/declarations.h>

#include "passes_common.h"
#include "shadow_rasterization_pass.h"

struct ShadowRasterizationPassData
{
  GLuint raysMapFBO;
  GLuint shadowsMapFBO;
  
  ShaderProgram* preparingProgram;
  ShaderProgram* shadowCalculationProgram;
  ShaderProgram* raysMoverProgram;
};

static void destroyShadowRasterizationPass(RenderPass* pass)
{
  ShadowRasterizationPassData* data = (ShadowRasterizationPassData*)renderPassGetInternalData(pass);
  glDeleteFramebuffers(1, &data->raysMapFBO);
  
  destroyShaderProgram(data->preparingProgram);
  destroyShaderProgram(data->shadowCalculationProgram);
  destroyShaderProgram(data->raysMoverProgram);
  
  engineFreeObject(data, MEMORY_TYPE_GENERAL);
}

static bool8 shadowRasterizationPassPrepareToRasterize(ShadowRasterizationPassData* data, uint32 lightIndex)
{
  shaderProgramUse(data->preparingProgram);
  glBindFramebuffer(GL_FRAMEBUFFER, data->raysMapFBO);
  
  glClearStencil(1);
  glClear(GL_STENCIL_BUFFER_BIT);
  glEnable(GL_STENCIL_TEST);

  glStencilFuncSeparate(GL_FRONT_AND_BACK, GL_ALWAYS, 1, 0xFF);
  glStencilOpSeparate(GL_FRONT_AND_BACK, GL_KEEP, GL_KEEP, GL_REPLACE);

  glUniform1ui(2, lightIndex);
  drawTriangleNoVAO();

  glDisable(GL_STENCIL_TEST);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  shaderProgramUse(nullptr);
  
  return TRUE;
}

static bool8 shadowRasterizationPassRasterize(ShadowRasterizationPassData* data, uint32 lightIndex)
{
  const RenderingParameters& renderingParams = rendererGetPassedRenderingParameters();  
  Scene* sceneToRasterize = rendererGetPassedScene();  

  glEnable(GL_STENCIL_TEST);

  glEnable(GL_BLEND);
  pushBlend(GL_FUNC_ADD, GL_FUNC_ADD, GL_ZERO, GL_ONE, GL_ONE, GL_ONE);

  uint32 culledObjectsCounter = 0;
  for(uint32 i = 0; i < renderingParams.shadowRasterItersMaxCount; i++)
  {
    culledObjectsCounter = 0;

    // ------------------------------------------------------------------------
    // 1. Calculate distances

    // Render only fragments that weren't culled/haven't yet reached limit
    glStencilFuncSeparate(GL_FRONT_AND_BACK, GL_EQUAL, 1, 0xFF);
    glStencilOpSeparate(GL_FRONT_AND_BACK, GL_KEEP, GL_KEEP, GL_KEEP);
    
    // TODO: Generate frustum for light (now we're passing nullptr) for further optimization
    drawGeometryPostorder(nullptr,
                          sceneGetGeometryRoot(sceneToRasterize),
                          0, 0,
                          culledObjectsCounter);

    // ------------------------------------------------------------------------
    // 2. Calculate soft shadow with parameters calculated at this iteration
    // (see: https://www.iquilezles.org/www/articles/rmshadows/rmshadows.htm)
    
    pushBlend(GL_MIN, GL_MIN, GL_ONE, GL_ONE, GL_ONE, GL_ONE);
    glBindFramebuffer(GL_FRAMEBUFFER, data->shadowsMapFBO);
    shaderProgramUse(data->shadowCalculationProgram);
    glUniform1ui(0, lightIndex);

    drawTriangleNoVAO();
    
    popBlend();

    // ------------------------------------------------------------------------
    // 3. Move per-pixel rays based on calculated distances

    // Move through all rays, shader will export ref value itself -->
    // replace stencil's value by exported one.
    glStencilFuncSeparate(GL_FRONT_AND_BACK, GL_ALWAYS, 1, 0xFF);
    glStencilOpSeparate(GL_FRONT_AND_BACK, GL_KEEP, GL_KEEP, GL_REPLACE);

    glBindFramebuffer(GL_FRAMEBUFFER, data->raysMapFBO);
    shaderProgramUse(data->raysMoverProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rendererGetResourceHandle(RR_DISTANCES_MAP_TEXTURE));
    
    glUniform1ui(glGetUniformLocation(shaderProgramGetGLHandle(data->raysMoverProgram), "depthMap"), 0);    
    glUniform1ui(glGetUniformLocation(shaderProgramGetGLHandle(data->raysMoverProgram), "curIterIdx"), i);
    glUniform1ui(glGetUniformLocation(shaderProgramGetGLHandle(data->raysMoverProgram), "lightIndex"), lightIndex); 
    drawTriangleNoVAO();

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);        
  }

  popBlend();  
  glDisable(GL_BLEND);
  glDisable(GL_STENCIL_TEST);
  
  return TRUE;
}

static bool8 shadowRasterizationPassExtractResults(ShadowRasterizationPassData* data)
{
  return TRUE;
}

static bool8 shadowRasterizationPassExecute(RenderPass* pass)
{
  ShadowRasterizationPassData* data = (ShadowRasterizationPassData*)renderPassGetInternalData(pass);
  std::vector<AssetPtr>& lightSources = sceneGetLightSources(rendererGetPassedScene());

  // NOTE: Clear each channel of shadowmap to 1.0f, meaning that initially lights are not blocked
  glBindFramebuffer(GL_FRAMEBUFFER, data->shadowsMapFBO);
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);  
  glClear(GL_COLOR_BUFFER_BIT);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
  for(uint32 lightIndex = 0; lightIndex < lightSources.size(); lightIndex++)
  {
    assert(shadowRasterizationPassPrepareToRasterize(data, lightIndex));
    assert(shadowRasterizationPassRasterize(data, lightIndex));
  }

  return TRUE;
}

static const char* shadowRasterizationPassGetName(RenderPass* pass)
{
  return "ShadowRasterizationPass";
}

static GLuint createRayMapFramebuffer()
{
  GLuint framebuffer = 0;
  
  glGenFramebuffers(1, &framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, rendererGetResourceHandle(RR_RAYS_MAP_TEXTURE), 0);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, rendererGetResourceHandle(RR_COVERAGE_MASK_TEXTURE), 0);
  
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  {
    return 0;
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return framebuffer;
}

static GLuint createShadowMapFramebuffer()
{
  GLuint framebuffer = 0;
  
  glGenFramebuffers(1, &framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, rendererGetResourceHandle(RR_SHADOWS_MAP_TEXTURE), 0);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, rendererGetResourceHandle(RR_COVERAGE_MASK_TEXTURE), 0);
  
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  {
    return 0;
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return framebuffer;  
}

bool8 createShadowRasterizationPass(RenderPass** outPass)
{
  RenderPassInterface interface = {};
  interface.destroy = destroyShadowRasterizationPass;
  interface.execute = shadowRasterizationPassExecute;
  interface.getName = shadowRasterizationPassGetName;
  interface.type = RENDER_PASS_TYPE_SHADOW_RASTERIZATION;

  if(allocateRenderPass(interface, outPass) == FALSE)
  {
    return FALSE;
  }

  ShadowRasterizationPassData* data = engineAllocObject<ShadowRasterizationPassData>(MEMORY_TYPE_GENERAL);

  data->raysMapFBO = createRayMapFramebuffer();
  assert(data->raysMapFBO != 0);

  data->shadowsMapFBO = createShadowMapFramebuffer();
  assert(data->shadowsMapFBO != 0);
  
  data->preparingProgram = createAndLinkTriangleShadingProgram("shaders/prepare_to_shadow_raster.frag");
  assert(data->preparingProgram != nullptr);

  data->shadowCalculationProgram = createAndLinkTriangleShadingProgram("shaders/shadows_estimator.frag");
  assert(data->shadowCalculationProgram != nullptr);
  
  data->raysMoverProgram = createAndLinkTriangleShadingProgram("shaders/rays_mover.frag");
  assert(data->raysMoverProgram != nullptr);
  
  renderPassSetInternalData(*outPass, data);
  
  return TRUE;
}
