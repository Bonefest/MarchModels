#pragma once

#include "defines.h"

using RenderPassType = uint32;

static const RenderPassType RENDER_PASS_TYPE_UNKNOWN = (RenderPassType)-1;

struct RenderPass;
struct RenderPassInterface
{
  void (*destroy)(RenderPass*);
  
  bool8 (*execute)(RenderPass* pass);  
  const char*(*getName)(RenderPass* pass);
  
  RenderPassType type = RENDER_PASS_TYPE_UNKNOWN;
};

ENGINE_API bool8 allocateRenderPass(RenderPassInterface interface, RenderPass** outPass);
ENGINE_API void destroyRenderPass(RenderPass* pass);

ENGINE_API bool8 renderPassExecute(RenderPass* pass);

ENGINE_API const char* renderPassGetName(RenderPass* pass);
ENGINE_API RenderPassType renderPassGetType(RenderPass* pass);

void renderPassSetInternalData(RenderPass* pass, void* internalData);
void* renderPassGetInternalData(RenderPass* pass);
