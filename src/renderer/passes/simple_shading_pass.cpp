#include <imgui/imgui.h>

#include "shader_program.h"
#include "memory_manager.h"
#include "shader_manager.h"
#include "renderer/renderer.h"
#include "renderer/renderer_utils.h"
#include "assets/materials_atlas_system.h"

#include "passes_common.h"
#include "simple_shading_pass.h"

struct SimpleShadingPassData
{
  GLuint ldrFBO;
  float3 ambientColor;
  
  ShaderProgramPtr shadingProgram;
};

static void destroySimpleShadingPass(RenderPass* pass)
{
  SimpleShadingPassData* data = (SimpleShadingPassData*)renderPassGetInternalData(pass);
  glDeleteFramebuffers(1, &data->ldrFBO);

  data->shadingProgram = ShaderProgramPtr(nullptr);
  
  engineFreeObject(data, MEMORY_TYPE_GENERAL);
}

static bool8 simpleShadingPassExecute(RenderPass* pass)
{
  SimpleShadingPassData* data = (SimpleShadingPassData*)renderPassGetInternalData(pass);
  const std::vector<AssetPtr>& lightSources = sceneGetEnabledLightSources(rendererGetPassedScene());

  glEnable(GL_BLEND);
  pushBlend(GL_FUNC_ADD, GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);  
  
  glBindFramebuffer(GL_FRAMEBUFFER, data->ldrFBO);
  shaderProgramUse(data->shadingProgram);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, imageGetGLHandle(masGetAtlas()));
  
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, rendererGetResourceHandle(RR_GEOIDS_MAP_TEXTURE));
  
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, rendererGetResourceHandle(RR_DEPTH1_MAP_TEXTURE));
  
  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D, rendererGetResourceHandle(RR_NORMALS_MAP_TEXTURE));

  glActiveTexture(GL_TEXTURE4);
  glBindTexture(GL_TEXTURE_2D, rendererGetResourceHandle(RR_SHADOWS_MAP_TEXTURE));

  glUniform1ui(0, lightSources.size());
  glUniform1i(1, 0);
  glUniform1i(2, 1);
  glUniform1i(3, 2);
  glUniform1i(4, 3);
  glUniform1i(5, 4);  
  
  drawTriangleNoVAO();

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  popBlend();
  glDisable(GL_BLEND);
  
  return TRUE;
}

static void simpleShadingPassDrawInputView(RenderPass* pass)
{
  SimpleShadingPassData* data = (SimpleShadingPassData*)renderPassGetInternalData(pass);

  ImGui::ColorEdit3("Ambient color", &data->ambientColor[0]);
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
  
  data->ldrFBO = createFramebuffer(rendererGetResourceHandle(RR_LDR1_MAP_TEXTURE));
  assert(data->ldrFBO != 0);

  data->shadingProgram = ShaderProgramPtr(createAndLinkTriangleShadingProgram("shaders/simple_shading.frag"));
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
