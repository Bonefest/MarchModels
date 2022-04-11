#pragma once

#include "render_pass.h"

static const RenderPassType RENDER_PASS_TYPE_SKY_RENDERING = 0x542ef00d;

ENGINE_API bool8 createSkyRenderingPass(RenderPass** outPass);
