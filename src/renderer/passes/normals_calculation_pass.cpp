#include "program.h"
#include "memory_manager.h"
#include "shader_manager.h"
#include "renderer/renderer.h"
#include "renderer/renderer_utils.h"

#include "normals_calculation_pass.h"

struct NormalsCalculationPassData
{
  GLuint normalsFBO;
  
  ShaderProgram* normalsCalculationProgram;
};

static void destroyNormalsCalculationPass(RenderPass* pass)
{
  NormalsCalculationPassData* data = (NormalsCalculationPassData*)renderPassGetInternalData(pass);
  glDeleteFramebuffers(1, &data->normalsFBO);
  
  destroyShaderProgram(data->normalsCalculationProgram);
  
  engineFreeObject(data, MEMORY_TYPE_GENERAL);
}

static bool8 normalsCalculationPassExecute(RenderPass* pass)
{
  NormalsCalculationPassData* data = (NormalsCalculationPassData*)renderPassGetInternalData(pass);

  glBindFramebuffer(GL_FRAMEBUFFER, data->normalsFBO);
  shaderProgramUse(data->normalsCalculationProgram);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, rendererGetResourceHandle(RR_DISTANCES_MAP_TEXTURE));

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, rendererGetResourceHandle(RR_GEOIDS_MAP_TEXTURE));
  
  drawTriangleNoVAO();
  
  shaderProgramUse(nullptr);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return TRUE;
}

static const char* normalsCalculationPassGetName(RenderPass* pass)
{
  return "NormalsCalculationPass";
}

static ShaderProgram* createNormalsCalculationProgram()
{
  ShaderProgram* program = nullptr;
  
  createShaderProgram(&program);
  shaderProgramAttachShader(program, shaderManagerGetShader("triangle.vert"));
  shaderProgramAttachShader(program, shaderManagerLoadShader(GL_FRAGMENT_SHADER, "shaders/calculate_normals.frag"));

  if(linkShaderProgram(program) == FALSE)
  {
    destroyShaderProgram(program);
    return nullptr;
  }

  shaderProgramUse(program);
  glUniform1i(glGetUniformLocation(shaderProgramGetGLHandle(program), "distancesMap"), 0);  
  glUniform1i(glGetUniformLocation(shaderProgramGetGLHandle(program), "idsMap"), 1);
  shaderProgramUse(nullptr);
  
  return program;
}

bool8 createNormalsCalculationPass(RenderPass** outPass)
{
  RenderPassInterface interface = {};
  interface.destroy = destroyNormalsCalculationPass;
  interface.execute = normalsCalculationPassExecute;
  interface.getName = normalsCalculationPassGetName;
  interface.type = RENDER_PASS_TYPE_NORMALS_CALCULATION_PASS;

  if(allocateRenderPass(interface, outPass) == FALSE)
  {
    return FALSE;
  }

  NormalsCalculationPassData* data = engineAllocObject<NormalsCalculationPassData>(MEMORY_TYPE_GENERAL);
  
  data->normalsFBO = createFramebuffer(rendererGetResourceHandle(RR_NORMALS_MAP_TEXTURE));
  assert(data->normalsFBO != 0);

  data->normalsCalculationProgram = createNormalsCalculationProgram();
  assert(data->normalsCalculationProgram != nullptr);

  renderPassSetInternalData(*outPass, data);
  
  return TRUE;
}
