#include <imgui/imgui.h>

#include <utils.h>
#include <../bin/shaders/declarations.h>

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
  FogType fogType;
  float2 fogMinMax;
  float2 fogNearFar;
  float3 fogColor;
  float32 fogDensity;
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

  float2 fogData = data->fogType == FOG_TYPE_LINEAR ? data->fogNearFar : float2(data->fogDensity, 0.0f);
  
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, rendererGetResourceHandle(RR_DISTANCES_MAP_TEXTURE));
  glUniform1i(0, 0);

  glActiveTexture(GL_TEXTURE1);  
  glBindTexture(GL_TEXTURE_2D, rendererGetResourceHandle(RR_GEOIDS_MAP_TEXTURE));
  glUniform1i(1, 1);
  
  glUniform1ui(2, data->fogType);
  glUniform2fv(3, 1, &data->fogMinMax[0]);
  glUniform2fv(4, 1, &fogData[0]);
  glUniform3fv(5, 1, &data->fogColor[0]);
  
  drawTriangleNoVAO();
  
  shaderProgramUse(nullptr);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  popBlend();
  glDisable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
  
  return TRUE;
}

static void fogPassDrawInputView(RenderPass* pass)
{
  FogPassData* data = (FogPassData*)renderPassGetInternalData(pass);

  const static char* fogTypeLables[] =
  {
    "Linear",
    "Exponential",
    "Exponential2"
  };

  ImGui::Combo("Fog type",
               (int32*)&data->fogType,
               fogTypeLables,
               ARRAY_SIZE(fogTypeLables));
               
  ImGui::SliderFloat2("Fog min/max", &data->fogMinMax[0], 0.0f, 1.0f);

  if(FOG_TYPE_LINEAR == data->fogType)
  {
    ImGui::SliderFloat2("Fog near/far", &data->fogNearFar[0], 0.0f, 100.0f);
  }
  else if(FOG_TYPE_EXPONENTIAL == data->fogType || FOG_TYPE_EXPONENTIAL2 == data->fogType)
  {
    ImGui::SliderFloat("Fog density", &data->fogDensity, 0.0f, 1.0f);
  }

  ImGui::ColorEdit3("Fog color", &data->fogColor[0]);

  data->fogMinMax.y = std::max(data->fogMinMax.x, data->fogMinMax.y);
  data->fogNearFar.y = std::max(data->fogNearFar.x, data->fogNearFar.y);
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
  interface.drawInputView = fogPassDrawInputView;  
  interface.getName = fogPassGetName;
  interface.type = RENDER_PASS_TYPE_FOG;

  if(allocateRenderPass(interface, outPass) == FALSE)
  {
    return FALSE;
  }

  FogPassData* data = engineAllocObject<FogPassData>(MEMORY_TYPE_GENERAL);
  data->fogType = FOG_TYPE_LINEAR;
  data->fogMinMax = float2(0.0f, 0.8f);
  data->fogNearFar = float2(3.0f, 15.0f);
  data->fogDensity = 0.5f;
  data->fogColor = float3(1.0f, 1.0f, 1.0f);
  
  data->ldrFBO = createFramebuffer(rendererGetResourceHandle(RR_LDR_MAP_TEXTURE));
  assert(data->ldrFBO != 0);

  data->fogProgram = ShaderProgramPtr(createAndLinkTriangleShadingProgram("shaders/fog.frag"));
  assert(data->fogProgram != nullptr);

  renderPassSetInternalData(*outPass, data);
  
  return TRUE;
}
