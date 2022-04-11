#include <imgui/imgui.h>

#include "shader_program.h"
#include "memory_manager.h"
#include "shader_manager.h"
#include "renderer/renderer.h"
#include "renderer/renderer_utils.h"

#include "passes_common.h"
#include "sky_rendering_pass.h"

struct SkyRenderingPassData
{
  GLuint ldrFBO;
  float3 bottomColor;
  float3 topColor;
  
  ShaderProgramPtr skyRenderingProgram;
};

static void destroySkyRenderingPass(RenderPass* pass)
{
  SkyRenderingPassData* data = (SkyRenderingPassData*)renderPassGetInternalData(pass);
  glDeleteFramebuffers(1, &data->ldrFBO);

  data->skyRenderingProgram = ShaderProgramPtr(nullptr);
  
  engineFreeObject(data, MEMORY_TYPE_GENERAL);
}

static bool8 skyRenderingPassExecute(RenderPass* pass)
{
  SkyRenderingPassData* data = (SkyRenderingPassData*)renderPassGetInternalData(pass);

  glBindFramebuffer(GL_FRAMEBUFFER, data->ldrFBO);
  shaderProgramUse(data->skyRenderingProgram);

  glUniform3fv(0, 1, &data->bottomColor[0]);
  glUniform3fv(1, 1, &data->topColor[0]);  
  
  drawTriangleNoVAO();
  
  shaderProgramUse(nullptr);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return TRUE;
}

static void skyRenderingPassDrawInputView(RenderPass* pass)
{
  SkyRenderingPassData* data = (SkyRenderingPassData*)renderPassGetInternalData(pass);

  ImGui::ColorEdit3("Top background color", &data->topColor[0]);  
  ImGui::ColorEdit3("Bottom background color", &data->bottomColor[0]);
}

static const char* skyRenderingPassGetName(RenderPass* pass)
{
  return "SkyRenderingPass";
}

bool8 createSkyRenderingPass(RenderPass** outPass)
{
  RenderPassInterface interface = {};
  interface.destroy       = destroySkyRenderingPass;
  interface.execute       = skyRenderingPassExecute;
  interface.drawInputView = skyRenderingPassDrawInputView;
  interface.getName       = skyRenderingPassGetName;
  interface.type          = RENDER_PASS_TYPE_SKY_RENDERING;

  if(allocateRenderPass(interface, outPass) == FALSE)
  {
    return FALSE;
  }

  SkyRenderingPassData* data = engineAllocObject<SkyRenderingPassData>(MEMORY_TYPE_GENERAL);
  data->topColor = float3(0.09f, 0.7f, 1.0f);    
  data->bottomColor = float3(0.63f, 0.93f, 0.9f);
  
  data->ldrFBO = createFramebuffer(rendererGetResourceHandle(RR_LDR1_MAP_TEXTURE));
  assert(data->ldrFBO != 0);

  data->skyRenderingProgram = ShaderProgramPtr(createAndLinkTriangleShadingProgram("shaders/sky_rendering.frag"));
  assert(data->skyRenderingProgram != nullptr);

  renderPassSetInternalData(*outPass, data);
  
  return TRUE;
}
