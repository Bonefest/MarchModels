#include "shader_program.h"
#include "memory_manager.h"
#include "shader_manager.h"
#include "renderer/renderer.h"
#include "renderer/renderer_utils.h"

#include "passes_common.h"
#include "normals_visualization_pass.h"

struct NormalsVisualizationPassData
{
  GLuint ldrFBO;
  
  ShaderProgramPtr visualizationProgram;
};

static void destroyNormalsVisualizationPass(RenderPass* pass)
{
  NormalsVisualizationPassData* data = (NormalsVisualizationPassData*)renderPassGetInternalData(pass);
  glDeleteFramebuffers(1, &data->ldrFBO);

  data->visualizationProgram = ShaderProgramPtr(nullptr);
  
  engineFreeObject(data, MEMORY_TYPE_GENERAL);
}

static bool8 normalsVisualizationPassExecute(RenderPass* pass)
{
  NormalsVisualizationPassData* data = (NormalsVisualizationPassData*)renderPassGetInternalData(pass);

  glBindFramebuffer(GL_FRAMEBUFFER, data->ldrFBO);
  shaderProgramUse(data->visualizationProgram);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, rendererGetResourceHandle(RR_NORMALS_MAP_TEXTURE));
  
  drawTriangleNoVAO();
  
  shaderProgramUse(nullptr);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return TRUE;
}

static const char* normalsVisualizationPassGetName(RenderPass* pass)
{
  return "NormalsVisualizationPass";
}

bool8 createNormalsVisualizationPass(RenderPass** outPass)
{
  RenderPassInterface interface = {};
  interface.destroy = destroyNormalsVisualizationPass;
  interface.execute = normalsVisualizationPassExecute;
  interface.getName = normalsVisualizationPassGetName;
  interface.type = RENDER_PASS_TYPE_NORMALS_VISUALIZATION;

  if(allocateRenderPass(interface, outPass) == FALSE)
  {
    return FALSE;
  }

  NormalsVisualizationPassData* data = engineAllocObject<NormalsVisualizationPassData>(MEMORY_TYPE_GENERAL);
  
  data->ldrFBO = createFramebuffer(rendererGetResourceHandle(RR_LDR1_MAP_TEXTURE));
  assert(data->ldrFBO != 0);

  data->visualizationProgram = ShaderProgramPtr(createAndLinkTriangleShadingProgram("shaders/visualize_normals.frag"));
  assert(data->visualizationProgram != nullptr);

  renderPassSetInternalData(*outPass, data);
  
  return TRUE;
}
