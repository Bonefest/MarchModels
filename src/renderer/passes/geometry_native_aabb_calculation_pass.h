#pragma once

#include <maths/aabb.h>
#include <assets/geometry.h>

ENGINE_API bool8 initializeAABBCalculationPass();
ENGINE_API void destroyAABBCalculationPass();

ENGINE_API AABB AABBCalculationPassCalculateAABB(Asset* geometry);
