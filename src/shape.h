#pragma once

#include <string>
#include <vector>

#include "dfunction.h"
#include "maths/common.h"

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
ENGINE_API float32 shapeGetPosition(Shape* shape);

ENGINE_API void shapeSetOrientation(Shape* shape, quat orientation);
ENGINE_API quat shapeGetOrientation(Shape* shape);

ENGINE_API void shapeAddIDF(Shape* shape, IDF* idf);
ENGINE_API std::vector<IDF*>& shapeGetIDFs(Shape* shape);

ENGINE_API void shapeAddODF(Shape* shape, ODF* odf);
ENGINE_API std::vector<ODF*>& shapeGetODFs(Shape* shape);

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
ENGINE_API float3 shapeCalculateNormal(Shape* shape, Ray ray);

// ----------------------------------------------------------------------------
// Branch shape-related interface
// ----------------------------------------------------------------------------

ENGINE_API void shapeAddChild(Shape* shape, Shape* child);
ENGINE_API bool8 shapeRemoveChild(Shape* shape, Shape* child);

ENGINE_API void shapeSetCombinationFunction(Shape* shape, CombinationFunction function);
ENGINE_API CombinationFunction shapeGetCombinationFunction(Shape* shape);

// ----------------------------------------------------------------------------
// Leaft shape-related interface
// ----------------------------------------------------------------------------

ENGINE_API void shapeSetSDF(Shape* shape, SDF* sdf);
ENGINE_API SDF* shapeGetSDF(Shape* shape);
