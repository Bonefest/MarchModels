#pragma once

#include "render_pass.h"

static const RenderPassType RENDER_PASS_TYPE_RASTERIZATION_PREPARING = 0x44cc3cda;

ENGINE_API bool8 createRasterizationPreparingPass(RenderPass** outPass);
