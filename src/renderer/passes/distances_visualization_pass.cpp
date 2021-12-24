#include "program.h"
#include "memory_manager.h"
#include "shader_manager.h"
#include "renderer/renderer.h"
#include "renderer/renderer_utils.h"

#include "distances_visualization_pass.h"

struct DistancesVisualizationPassData
{
  float2 distancesRange;
  float3 closestColor;
  float3 farthestColor;
  
  GLuint ldrFB;
  
  ShaderProgram* visualizationProgram;
};

static void destroyDistancesVisualizationPass(RenderPass* pass)
{
  DistancesVisualizationPassData* data = (DistancesVisualizationPassData*)renderPassGetInternalData(pass);
  glDeleteFramebuffers(1, &data->ldrFB);
  
  destroyShaderProgram(data->visualizationProgram);
  
  engineFreeObject(data, MEMORY_TYPE_GENERAL);
}

static bool8 distancesVisualizationPassExecute(RenderPass* pass)
{
  DistancesVisualizationPassData* data = (DistancesVisualizationPassData*)renderPassGetInternalData(pass);

  glBindFramebuffer(GL_FRAMEBUFFER, data->ldrFB);
  shaderProgramUse(data->visualizationProgram);

  GLuint programHandle = shaderProgramGetGLHandle(data->visualizationProgram);
  glUniform2fv(glGetUniformLocation(programHandle, "distancesRange"), 1, &data->distancesRange[0]);
  glUniform3fv(glGetUniformLocation(programHandle, "closestColor"), 1, &data->closestColor[0]);
  glUniform3fv(glGetUniformLocation(programHandle, "farthestColor"), 1, &data->farthestColor[0]);
  
  drawTriangleNoVAO();
  shaderProgramUse(nullptr);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
  return TRUE;
}

static const char* distancesVisualizationPassGetName(RenderPass* pass)
{
  return "DistancesVisualizationPass";
}

static ShaderProgram* createVisualizationProgram()
{
  ShaderProgram* program = nullptr;
  
  createShaderProgram(&program);
  shaderProgramAttachShader(program, shaderManagerGetShader("triangle.vert"));
  shaderProgramAttachShader(program, shaderManagerLoadShader(GL_FRAGMENT_SHADER, "shaders/visualize_distances.frag"));

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

bool8 createDistancesVisualizationPass(float2 distancesRange,
                                       float3 closestColor, float3 farthestColor,
                                       RenderPass** outPass)
{
  RenderPassInterface interface = {};
  interface.destroy = destroyDistancesVisualizationPass;
  interface.execute = distancesVisualizationPassExecute;
  interface.getName = distancesVisualizationPassGetName;
  interface.type = RENDER_PASS_TYPE_DISTANCES_VISUALIZATION;

  if(allocateRenderPass(interface, outPass) == FALSE)
  {
    return FALSE;
  }

  DistancesVisualizationPassData* data = engineAllocObject<DistancesVisualizationPassData>(MEMORY_TYPE_GENERAL);

  data->ldrFB = createLDRFramebuffer();
  assert(data->ldrFB != 0);

  data->visualizationProgram = createVisualizationProgram();
  assert(data->visualizationProgram != nullptr);

  renderPassSetInternalData(*outPass, data);
  
  return TRUE;
}
