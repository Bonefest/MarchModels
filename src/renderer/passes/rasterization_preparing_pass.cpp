#include "program.h"
#include "shader_manager.h"
#include "renderer/renderer.h"

#include "rasterization_preparing_pass.h"

struct RasterizationPreparingPassData
{
  GLuint raysMapFramebuffer;
  ShaderProgram* program;
};

static void destroyRasterizationPrepararingPass(RenderPass* pass)
{
  RasterizationPreparingPassData* data = (RasterizationPreparingPassData*)renderPassGetInternalData(pass);
  glDeleteFramebuffers(1, &data->raysMapFramebuffer);  
  destroyShaderProgram(data->program);
  
  engineFreeObject(data, MEMORY_TYPE_GENERAL);
}

static bool8 rasterizationPreparingPassExecute(RenderPass* pass)
{
  RasterizationPreparingPassData* data = (RasterizationPreparingPassData*)renderPassGetInternalData(pass);

  shaderProgramUse(data->program);
  glBindFramebuffer(GL_FRAMEBUFFER, data->raysMapFramebuffer);
  glBindVertexArray(rendererGetResourceHandle(RR_EMPTY_VAO));

  glDrawArrays(GL_TRIANGLES, 0, 3);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  shaderProgramUse(nullptr);
  
  return TRUE;
}

static const char* rasterizationPreparingPassGetName(RenderPass* pass)
{
  return "RasterizationPreparingPass";
}

static ShaderProgram* createShaderProgram()
{
  ShaderProgram* program = nullptr;
  
  createShaderProgram(&program);
  shaderProgramAttachShader(program, shaderManagerGetShader("triangle.vert"));
  shaderProgramAttachShader(program, shaderManagerLoadShader(GL_FRAGMENT_SHADER, "shaders/frame_preparer.frag"));

  if(linkShaderProgram(program) == FALSE)
  {
    destroyShaderProgram(program);
    return nullptr;
  }
  
  return program;
}

static GLuint createRayMapFramebuffer()
{
  GLuint framebuffer = 0;
  
  glGenFramebuffers(1, &framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, rendererGetResourceHandle(RR_RAYS_MAP_TEXTURE), 0);
  // TODO: attach stencil too
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  {
    return 0;
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return framebuffer;
}

bool8 createRasterizationPreparingPass(RenderPass** outPass)
{
  RenderPassInterface interface = {};
  interface.destroy = destroyRasterizationPrepararingPass;
  interface.execute = rasterizationPreparingPassExecute;
  interface.getName = rasterizationPreparingPassGetName;
  interface.type = RENDER_PASS_TYPE_RASTERIZATION_PREPARING;

  if(allocateRenderPass(interface, outPass) == FALSE)
  {
    return FALSE;
  }

  RasterizationPreparingPassData* data = engineAllocObject<RasterizationPreparingPassData>(MEMORY_TYPE_GENERAL);

  data->raysMapFramebuffer = createRayMapFramebuffer();
  assert(data->raysMapFramebuffer != 0);
  
  data->program = createShaderProgram();
  assert(data->program != nullptr);
  
  renderPassSetInternalData(*outPass, data);
  
  return TRUE;
}
