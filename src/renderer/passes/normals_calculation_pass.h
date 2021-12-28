#pragma once

#include "render_pass.h"

static const RenderPassType RENDER_PASS_TYPE_NORMALS_CALCULATION_PASS = 0xc12cb3a6;

ENGINE_API bool8 createNormalsCalculationPass(RenderPass** outPass);

