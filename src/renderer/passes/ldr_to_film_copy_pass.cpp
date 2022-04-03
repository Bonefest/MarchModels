#include "shader_program.h"
#include "memory_manager.h"
#include "shader_manager.h"
#include "renderer/renderer.h"
#include "renderer/renderer_utils.h"

#include "passes_common.h"
#include "ldr_to_film_copy_pass.h"

struct LDRToFilmCopyPassData
{
  ShaderProgramPtr copyProgram;
};

static void destroyLDRToFilmCopyPass(RenderPass* pass)
{
  LDRToFilmCopyPassData* data = (LDRToFilmCopyPassData*)renderPassGetInternalData(pass);
  data->copyProgram = ShaderProgramPtr(nullptr);
  
  engineFreeObject(data, MEMORY_TYPE_GENERAL);
}

static bool8 LDRToFilmCopyPassExecute(RenderPass* pass)
{
  LDRToFilmCopyPassData* data = (LDRToFilmCopyPassData*)renderPassGetInternalData(pass);
  Film* film = rendererGetPassedFilm();
  
  glBindFramebuffer(GL_FRAMEBUFFER, filmGetGLFBOHandle(film));
  float4 blackColor = float4(0.0, 0.0, 0.0, 1.0);
  glClearBufferfv(GL_COLOR, 0, &blackColor[0]);
  
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
  
  data->copyProgram = ShaderProgramPtr(createAndLinkTriangleShadingProgram("shaders/ldr_passthrough.frag"));
  assert(data->copyProgram != nullptr);

  renderPassSetInternalData(*outPass, data);
  
  return TRUE;
}

