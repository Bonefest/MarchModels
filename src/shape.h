#pragma once

#include <string>
#include <vector>

#include "maths/common.h"
#include "script_function.h"

struct Material;

enum CombinationFunction
{
  COMBINATION_INTERSECTION,
  COMBINATION_UNION,
  COMBINATION_SUBTRACTION
};

struct Shape;

// ----------------------------------------------------------------------------
// Shape common interface
// ----------------------------------------------------------------------------
ENGINE_API bool8 createShape(const std::string& name, Shape** outShape);
ENGINE_API void destroyShape(Shape* shape);

ENGINE_API void shapeSetScale(Shape* shape, float32 scale);
ENGINE_API float32 shapeGetScale(Shape* shape);

ENGINE_API void shapeSetPosition(Shape* shape, float3 position);
ENGINE_API float3 shapeGetPosition(Shape* shape);

ENGINE_API void shapeSetOrientation(Shape* shape, quat orientation);
ENGINE_API quat shapeGetOrientation(Shape* shape);

ENGINE_API void shapeAddIDF(Shape* shape, ScriptFunction* idf);
ENGINE_API std::vector<ScriptFunction*>& shapeGetIDFs(Shape* shape);

ENGINE_API void shapeAddODF(Shape* shape, ScriptFunction* odf);
ENGINE_API std::vector<ScriptFunction*>& shapeGetODFs(Shape* shape);

ENGINE_API void shapeSetName(Shape* shape, const std::string& name);
ENGINE_API const std::string& shapeGetName(Shape* shape);

ENGINE_API void shapeSetParent(Shape* shape, Shape* parent);
ENGINE_API Shape* shapeGetParent(Shape* shape);

ENGINE_API bool8 shapeIsRoot(Shape* shape);
ENGINE_API bool8 shapeIsBranch(Shape* shape);
ENGINE_API bool8 shapeIsLeaf(Shape* shape);

// ENGINE_API AABB shapeGetAABB(Shape* shape);
// ENGINE_API float32 shapeGetRadius(Shape* shape);

ENGINE_API float32 shapeCalculateDistanceToPoint(Shape* shape, float3 p, Shape** outClosestLeafShape = nullptr);

// WARNING: Highly unoptimized!
ENGINE_API float3 shapeCalculateNormal(Shape* shape, float3 p);

// ----------------------------------------------------------------------------
// Branch shape-related interface
// ----------------------------------------------------------------------------

ENGINE_API void shapeAddChild(Shape* shape, Shape* child);
ENGINE_API bool8 shapeRemoveChild(Shape* shape, Shape* child);
ENGINE_API std::vector<Shape*> shapeGetChildren(Shape* shape);

ENGINE_API void shapeSetCombinationFunction(Shape* shape, CombinationFunction function);
ENGINE_API CombinationFunction shapeGetCombinationFunction(Shape* shape);

// ----------------------------------------------------------------------------
// Leaft shape-related interface
// ----------------------------------------------------------------------------

ENGINE_API void shapeSetSDF(Shape* shape, ScriptFunction* sdf);
ENGINE_API ScriptFunction* shapeGetSDF(Shape* shape);
