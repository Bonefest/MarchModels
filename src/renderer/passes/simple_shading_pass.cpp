#include <imgui/imgui.h>

#include "program.h"
#include "memory_manager.h"
#include "shader_manager.h"
#include "renderer/renderer.h"
#include "renderer/renderer_utils.h"

#include "passes_common.h"
#include "simple_shading_pass.h"

struct SimpleShadingPassData
{
  GLuint ldrFBO;
  float3 ambientColor;
  float3 bottomColor;
  float3 topColor;
  
  ShaderProgram* shadingProgram;
};

static void destroySimpleShadingPass(RenderPass* pass)
{
  SimpleShadingPassData* data = (SimpleShadingPassData*)renderPassGetInternalData(pass);
  glDeleteFramebuffers(1, &data->ldrFBO);
  
  destroyShaderProgram(data->shadingProgram);
  
  engineFreeObject(data, MEMORY_TYPE_GENERAL);
}

static bool8 simpleShadingPassExecute(RenderPass* pass)
{
  SimpleShadingPassData* data = (SimpleShadingPassData*)renderPassGetInternalData(pass);
  const std::vector<AssetPtr>& lightSources = sceneGetEnabledLightSources(rendererGetPassedScene());
  
  glBindFramebuffer(GL_FRAMEBUFFER, data->ldrFBO);
  shaderProgramUse(data->shadingProgram);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, rendererGetResourceHandle(RR_DISTANCES_MAP_TEXTURE));
  
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, rendererGetResourceHandle(RR_NORMALS_MAP_TEXTURE));

  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, rendererGetResourceHandle(RR_SHADOWS_MAP_TEXTURE));

  glUniform1ui(0, lightSources.size());
  glUniform3fv(1, 1, &data->ambientColor[0]);
  glUniform3fv(2, 1, &data->topColor[0]);
  glUniform3fv(3, 1, &data->bottomColor[0]);    
  glUniform1i(4, 0);
  glUniform1i(5, 1);
  glUniform1i(6, 2);  
  
  drawTriangleNoVAO();

  shaderProgramUse(nullptr);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
  return TRUE;
}

static void simpleShadingPassDrawInputView(RenderPass* pass)
{
  SimpleShadingPassData* data = (SimpleShadingPassData*)renderPassGetInternalData(pass);

  ImGui::ColorEdit3("Ambient color", &data->ambientColor[0]);
  ImGui::ColorEdit3("Top background color", &data->topColor[0]);
  ImGui::ColorEdit3("Bottom background color", &data->bottomColor[0]);  
}

static const char* simpleShadingPassGetName(RenderPass* pass)
{
  return "SimpleShadingPass";
}

bool8 createSimpleShadingPass(RenderPass** outPass)
{
  RenderPassInterface interface = {};
  interface.destroy       = destroySimpleShadingPass;
  interface.execute       = simpleShadingPassExecute;
  interface.drawInputView = simpleShadingPassDrawInputView;
  interface.getName       = simpleShadingPassGetName;
  interface.type          = RENDER_PASS_TYPE_SIMPLE_SHADING_PASS;

  if(allocateRenderPass(interface, outPass) == FALSE)
  {
    return FALSE;
  }

  SimpleShadingPassData* data = engineAllocObject<SimpleShadingPassData>(MEMORY_TYPE_GENERAL);
  data->ambientColor = float3(0.09f, 0.13f, 0.16f);
  data->bottomColor = float3(0.5f, 0.5f, 1.0f);
  data->topColor = float3(0.45f, 0.45f, 0.45f);  
  
  data->ldrFBO = createFramebuffer(rendererGetResourceHandle(RR_LDR_MAP_TEXTURE));
  assert(data->ldrFBO != 0);

  data->shadingProgram = createAndLinkTriangleShadingProgram("shaders/simple_shading.frag");
  assert(data->shadingProgram != nullptr);

  renderPassSetInternalData(*outPass, data);
  
  return TRUE;
}

void simpleShadingPassSetAmbientColor(RenderPass* pass, float3 ambientColor)
{
  SimpleShadingPassData* data = (SimpleShadingPassData*)renderPassGetInternalData(pass);
  data->ambientColor = ambientColor;
}

float3 simpleShadingPassGetAmbinetColor(RenderPass* pass)
{
  SimpleShadingPassData* data = (SimpleShadingPassData*)renderPassGetInternalData(pass);
  return data->ambientColor;
}
