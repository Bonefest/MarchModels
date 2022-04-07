#include <imgui/imgui.h>

#include "shader_program.h"
#include "memory_manager.h"
#include "shader_manager.h"
#include "renderer/renderer.h"
#include "renderer/renderer_utils.h"

#include "passes_common.h"
#include "distances_visualization_pass.h"

struct DistancesVisualizationPassData
{
  float2 distancesRange;
  float3 closestColor;
  float3 farthestColor;
  
  GLuint ldrFBO;
  
  ShaderProgramPtr visualizationProgram;
};

static void destroyDistancesVisualizationPass(RenderPass* pass)
{
  DistancesVisualizationPassData* data = (DistancesVisualizationPassData*)renderPassGetInternalData(pass);
  glDeleteFramebuffers(1, &data->ldrFBO);
  
  data->visualizationProgram = ShaderProgramPtr(nullptr);
  
  engineFreeObject(data, MEMORY_TYPE_GENERAL);
}

static bool8 distancesVisualizationPassExecute(RenderPass* pass)
{
  DistancesVisualizationPassData* data = (DistancesVisualizationPassData*)renderPassGetInternalData(pass);

  glBindFramebuffer(GL_FRAMEBUFFER, data->ldrFBO);
  shaderProgramUse(data->visualizationProgram);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, rendererGetResourceHandle(RR_DEPTH1_MAP_TEXTURE));
  
  GLuint programHandle = shaderProgramGetGLHandle(data->visualizationProgram);
  glUniform2fv(glGetUniformLocation(programHandle, "distancesRange"), 1, &data->distancesRange[0]);
  glUniform3fv(glGetUniformLocation(programHandle, "closestColor"), 1, &data->closestColor[0]);
  glUniform3fv(glGetUniformLocation(programHandle, "farthestColor"), 1, &data->farthestColor[0]);
  
  drawTriangleNoVAO();
  shaderProgramUse(nullptr);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return TRUE;
}

static void distancesVisualizationPassDrawInputView(RenderPass* pass)
{
  DistancesVisualizationPassData* data = (DistancesVisualizationPassData*)renderPassGetInternalData(pass);

  ImGui::SliderFloat("Max distance", &data->distancesRange.y, 0.1, 100.0);
  ImGui::ColorEdit3("Closest color", &data->closestColor[0]);
  ImGui::ColorEdit3("Farthest color", &data->farthestColor[0]);  
}

static const char* distancesVisualizationPassGetName(RenderPass* pass)
{
  return "DistancesVisualizationPass";
}

bool8 createDistancesVisualizationPass(float2 distancesRange,
                                       float3 closestColor, float3 farthestColor,
                                       RenderPass** outPass)
{
  RenderPassInterface interface = {};
  interface.destroy = destroyDistancesVisualizationPass;
  interface.execute = distancesVisualizationPassExecute;
  interface.drawInputView = distancesVisualizationPassDrawInputView;
  interface.getName = distancesVisualizationPassGetName;
  interface.type = RENDER_PASS_TYPE_DISTANCES_VISUALIZATION;

  if(allocateRenderPass(interface, outPass) == FALSE)
  {
    return FALSE;
  }

  DistancesVisualizationPassData* data = engineAllocObject<DistancesVisualizationPassData>(MEMORY_TYPE_GENERAL);
  data->distancesRange = distancesRange;
  data->closestColor = closestColor;
  data->farthestColor = farthestColor;
  
  data->ldrFBO = createFramebuffer(rendererGetResourceHandle(RR_LDR1_MAP_TEXTURE));
  assert(data->ldrFBO != 0);

  data->visualizationProgram = ShaderProgramPtr(createAndLinkTriangleShadingProgram("shaders/visualize_distances.frag"));
  assert(data->visualizationProgram != nullptr);

  renderPassSetInternalData(*outPass, data);
  
  return TRUE;
}
