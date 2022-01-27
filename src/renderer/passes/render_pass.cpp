#include "memory_manager.h"

#include "render_pass.h"

struct RenderPass
{
  RenderPassInterface interface;

  void* internalData;
};

bool8 allocateRenderPass(RenderPassInterface interface, RenderPass** outPass)
{
  *outPass = engineAllocObject<RenderPass>(MEMORY_TYPE_GENERAL);
  RenderPass* pass = *outPass;
  pass->interface = interface;
  pass->internalData = nullptr;

  return TRUE;
}

void destroyRenderPass(RenderPass* pass)
{
  pass->interface.destroy(pass);
  
  engineFreeObject(pass, MEMORY_TYPE_GENERAL);
}

bool8 renderPassExecute(RenderPass* pass)
{
  return pass->interface.execute(pass);
}

void renderPassDrawInputView(RenderPass* pass)
{
  if(renderPassHasInputView(pass) == TRUE)
  {
    pass->interface.drawInputView(pass);
  }
}

bool8 renderPassHasInputView(RenderPass* pass)
{
  return pass->interface.drawInputView != nullptr ? TRUE : FALSE;
}

const char* renderPassGetName(RenderPass* pass)
{
  return pass->interface.getName(pass);
}

RenderPassType renderPassGetType(RenderPass* pass)
{
  return pass->interface.type;
}

void renderPassSetInternalData(RenderPass* pass, void* internalData)
{
  pass->internalData = internalData;
}

void* renderPassGetInternalData(RenderPass* pass)
{
  return pass->internalData;
}

