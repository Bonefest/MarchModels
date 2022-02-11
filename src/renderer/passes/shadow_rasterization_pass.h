#pragma once

#include "render_pass.h"

static const RenderPassType RENDER_PASS_TYPE_SHADOW_RASTERIZATION = 0x233e2b58;

ENGINE_API bool8 createShadowRasterizationPass(RenderPass** outPass);
