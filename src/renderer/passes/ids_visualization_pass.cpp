#include "shader_program.h"
#include "memory_manager.h"
#include "shader_manager.h"
#include "renderer/renderer.h"
#include "renderer/renderer_utils.h"

#include "passes_common.h"
#include "ids_visualization_pass.h"

struct IDsVisualizationPassData
{
  GLuint ldrFBO;
  
  ShaderProgramPtr visualizationProgram;
};

static void destroyIDsVisualizationPass(RenderPass* pass)
{
  IDsVisualizationPassData* data = (IDsVisualizationPassData*)renderPassGetInternalData(pass);
  glDeleteFramebuffers(1, &data->ldrFBO);

  data->visualizationProgram = ShaderProgramPtr(nullptr);
  
  engineFreeObject(data, MEMORY_TYPE_GENERAL);
}

static bool8 idsVisualizationPassExecute(RenderPass* pass)
{
  IDsVisualizationPassData* data = (IDsVisualizationPassData*)renderPassGetInternalData(pass);

  glBindFramebuffer(GL_FRAMEBUFFER, data->ldrFBO);
  shaderProgramUse(data->visualizationProgram);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, rendererGetResourceHandle(RR_GEOIDS_MAP_TEXTURE));
  
  drawTriangleNoVAO();
  shaderProgramUse(nullptr);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return TRUE;
}

static const char* idsVisualizationPassGetName(RenderPass* pass)
{
  return "IDsVisualizationPass";
}

bool8 createIDsVisualizationPass(RenderPass** outPass)
{
  RenderPassInterface interface = {};
  interface.destroy = destroyIDsVisualizationPass;
  interface.execute = idsVisualizationPassExecute;
  interface.getName = idsVisualizationPassGetName;
  interface.type = RENDER_PASS_TYPE_IDS_VISUALIZATION;

  if(allocateRenderPass(interface, outPass) == FALSE)
  {
    return FALSE;
  }

  IDsVisualizationPassData* data = engineAllocObject<IDsVisualizationPassData>(MEMORY_TYPE_GENERAL);
  
  data->ldrFBO = createFramebuffer(rendererGetResourceHandle(RR_LDR_MAP_TEXTURE));
  assert(data->ldrFBO != 0);

  data->visualizationProgram = ShaderProgramPtr(createAndLinkTriangleShadingProgram("shaders/visualize_ids.frag"));
  assert(data->visualizationProgram != nullptr);

  renderPassSetInternalData(*outPass, data);
  
  return TRUE;
}
