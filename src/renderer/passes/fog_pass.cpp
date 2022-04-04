#include "shader_program.h"
#include "memory_manager.h"
#include "shader_manager.h"
#include "renderer/renderer.h"
#include "renderer/renderer_utils.h"

#include "fog_pass.h"
#include "passes_common.h"

struct FogPassData
{
  GLuint ldrFBO;
  
  ShaderProgramPtr fogProgram;
  bool8 exponential;
  float2 fogMinMax;
  float2 fogNearFar;
  float3 fogColor;
};

static void destroyFogPass(RenderPass* pass)
{
  FogPassData* data = (FogPassData*)renderPassGetInternalData(pass);
  glDeleteFramebuffers(1, &data->ldrFBO);

  data->fogProgram = ShaderProgramPtr(nullptr);
  
  engineFreeObject(data, MEMORY_TYPE_GENERAL);
}

static bool8 fogPassExecute(RenderPass* pass)
{
  FogPassData* data = (FogPassData*)renderPassGetInternalData(pass);

  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  pushBlend(GL_FUNC_ADD, GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
  
  glBindFramebuffer(GL_FRAMEBUFFER, data->ldrFBO);
  shaderProgramUse(data->fogProgram);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, rendererGetResourceHandle(RR_DISTANCES_MAP_TEXTURE));
  glUniform1i(0, 0);
  glUniform2fv(1, 1, &data->fogMinMax[0]);
  glUniform2fv(2, 1, &data->fogNearFar[0]);
  glUniform3fv(3, 1, &data->fogColor[0]);    
  
  drawTriangleNoVAO();
  
  shaderProgramUse(nullptr);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  popBlend();
  glDisable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
  
  return TRUE;
}

static const char* fogPassGetName(RenderPass* pass)
{
  return "FogPass";
}

bool8 createFogPass(RenderPass** outPass)
{
  RenderPassInterface interface = {};
  interface.destroy = destroyFogPass;
  interface.execute = fogPassExecute;
  interface.getName = fogPassGetName;
  interface.type = RENDER_PASS_TYPE_FOG;

  if(allocateRenderPass(interface, outPass) == FALSE)
  {
    return FALSE;
  }

  FogPassData* data = engineAllocObject<FogPassData>(MEMORY_TYPE_GENERAL);
  data->fogMinMax = float2(0.0f, 0.8f);
  data->fogNearFar = float2(3.0f, 15.0f);
  data->fogColor = float3(1.0f, 1.0f, 1.0f);
  
  data->ldrFBO = createFramebuffer(rendererGetResourceHandle(RR_LDR_MAP_TEXTURE));
  assert(data->ldrFBO != 0);

  data->fogProgram = ShaderProgramPtr(createAndLinkTriangleShadingProgram("shaders/fog.frag"));
  assert(data->fogProgram != nullptr);

  renderPassSetInternalData(*outPass, data);
  
  return TRUE;
}
