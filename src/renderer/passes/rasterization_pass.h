#pragma once

#include "render_pass.h"

static const RenderPassType RENDER_PASS_TYPE_RASTERIZATION = 0x233e2b58;

ENGINE_API bool8 createRasterizationPass(RenderPass** outPass);
