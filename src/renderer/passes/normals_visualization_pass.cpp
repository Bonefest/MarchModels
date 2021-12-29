#include "program.h"
#include "memory_manager.h"
#include "shader_manager.h"
#include "renderer/renderer.h"
#include "renderer/renderer_utils.h"

#include "normals_visualization_pass.h"

struct NormalsVisualizationPassData
{
  GLuint ldrFBO;
  
  ShaderProgram* visualizationProgram;
};

static void destroyNormalsVisualizationPass(RenderPass* pass)
{
  NormalsVisualizationPassData* data = (NormalsVisualizationPassData*)renderPassGetInternalData(pass);
  glDeleteFramebuffers(1, &data->ldrFBO);
  
  destroyShaderProgram(data->visualizationProgram);
  
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

static ShaderProgram* createVisualizationProgram()
{
  ShaderProgram* program = nullptr;
  
  createShaderProgram(&program);
  shaderProgramAttachShader(program, shaderManagerGetShader("triangle.vert"));
  shaderProgramAttachShader(program, shaderManagerLoadShader(GL_FRAGMENT_SHADER, "shaders/visualize_normals.frag"));

  if(linkShaderProgram(program) == FALSE)
  {
    destroyShaderProgram(program);
    return nullptr;
  }
  
  return program;
}

static GLuint createLDRFramebuffer()
{
  GLuint framebuffer = 0;
  
  glGenFramebuffers(1, &framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, rendererGetResourceHandle(RR_LDR_MAP_TEXTURE), 0);
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  {
    return 0;
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return framebuffer;
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
  
  data->ldrFBO = createLDRFramebuffer();
  assert(data->ldrFBO != 0);

  data->visualizationProgram = createVisualizationProgram();
  assert(data->visualizationProgram != nullptr);

  renderPassSetInternalData(*outPass, data);
  
  return TRUE;
}
