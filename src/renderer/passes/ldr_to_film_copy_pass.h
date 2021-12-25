#pragma once

#include "render_pass.h"

static const RenderPassType RENDER_PASS_TYPE_LDR_TO_FILM_COPY_PASS = 0x95f25c26;

ENGINE_API bool8 createLDRToFilmCopyPass(RenderPass** outPass);

