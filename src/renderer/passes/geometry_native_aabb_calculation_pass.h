#pragma once

#include <assets/geometry.h>
#include <maths/primitives.h>

ENGINE_API bool8 initializeAABBCalculationPass();
ENGINE_API void destroyAABBCalculationPass();

ENGINE_API AABB AABBCalculationPassCalculateAABB(Asset* geometry);
