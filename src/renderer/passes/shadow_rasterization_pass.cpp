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
  
  ShaderProgram* preparingProgram;
  ShaderProgram* raysMoverProgram;
};

static void destroyShadowRasterizationPass(RenderPass* pass)
{
  ShadowRasterizationPassData* data = (ShadowRasterizationPassData*)renderPassGetInternalData(pass);
  glDeleteFramebuffers(1, &data->raysMapFBO);
  
  destroyShaderProgram(data->preparingProgram);
  destroyShaderProgram(data->raysMoverProgram);
  
  engineFreeObject(data, MEMORY_TYPE_GENERAL);
}

static bool8 shadowRasterizationPassPrepareToRasterize(ShadowRasterizationPassData* data)
{
  
  
  return TRUE;
}

static bool8 shadowRasterizationPassRasterize(ShadowRasterizationPassData* data)
{
  return TRUE;
}

static bool8 shadowRasterizationPassExtractResults(ShadowRasterizationPassData* data)
{
  return TRUE;
}

static bool8 shadowRasterizationPassExecute(RenderPass* pass)
{
  ShadowRasterizationPassData* data = (ShadowRasterizationPassData*)renderPassGetInternalData(pass);

  assert(shadowRasterizationPassPrepareToRasterize(data));
  assert(shadowRasterizationPassRasterize(data));
  assert(shadowRasterizationPassExtractResults(data));
  
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

  data->preparingProgram = createAndLinkTriangleShadingProgram("shaders/prepare_to_raster.frag");
  assert(data->preparingProgram != nullptr);

  data->raysMoverProgram = createAndLinkTriangleShadingProgram("shaders/rays_mover.frag");
  assert(data->raysMoverProgram != nullptr);
  
  renderPassSetInternalData(*outPass, data);
  
  return TRUE;
}
