#include "program.h"
#include "memory_manager.h"
#include "shader_manager.h"
#include "renderer/renderer.h"
#include "renderer/renderer_utils.h"

#include "ldr_to_film_copy_pass.h"

struct LDRToFilmCopyPassData
{
  ShaderProgram* copyProgram;
};

static void destroyLDRToFilmCopyPass(RenderPass* pass)
{
  LDRToFilmCopyPassData* data = (LDRToFilmCopyPassData*)renderPassGetInternalData(pass);
  destroyShaderProgram(data->copyProgram);
  
  engineFreeObject(data, MEMORY_TYPE_GENERAL);
}

static bool8 LDRToFilmCopyPassExecute(RenderPass* pass)
{
  LDRToFilmCopyPassData* data = (LDRToFilmCopyPassData*)renderPassGetInternalData(pass);
  Film* film = rendererGetPassedFilm();
  
  glBindFramebuffer(GL_FRAMEBUFFER, filmGetGLFBOHandle(film));
  shaderProgramUse(data->copyProgram);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, rendererGetResourceHandle(RR_LDR_MAP_TEXTURE));
  
  drawTriangleNoVAO();
  
  shaderProgramUse(nullptr);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return TRUE;
}

static const char* LDRToFilmCopyPassGetName(RenderPass* pass)
{
  return "LDRToFilmCopyPass";
}

static ShaderProgram* createCopyProgram()
{
  ShaderProgram* program = nullptr;
  
  createShaderProgram(&program);
  shaderProgramAttachShader(program, shaderManagerGetShader("triangle.vert"));
  shaderProgramAttachShader(program, shaderManagerLoadShader(GL_FRAGMENT_SHADER, "shaders/ldr_passthrough.frag"));

  if(linkShaderProgram(program) == FALSE)
  {
    destroyShaderProgram(program);
    return nullptr;
  }
  
  return program;
}

bool8 createLDRToFilmCopyPass(RenderPass** outPass)
{
  RenderPassInterface interface = {};
  interface.destroy = destroyLDRToFilmCopyPass;
  interface.execute = LDRToFilmCopyPassExecute;
  interface.getName = LDRToFilmCopyPassGetName;
  interface.type = RENDER_PASS_TYPE_LDR_TO_FILM_COPY_PASS;

  if(allocateRenderPass(interface, outPass) == FALSE)
  {
    return FALSE;
  }

  LDRToFilmCopyPassData* data = engineAllocObject<LDRToFilmCopyPassData>(MEMORY_TYPE_GENERAL);
  
  data->copyProgram = createCopyProgram();
  assert(data->copyProgram != nullptr);

  renderPassSetInternalData(*outPass, data);
  
  return TRUE;
}

