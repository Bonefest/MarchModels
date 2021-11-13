#pragma once

#include <string>
#include <vector>

#include "maths/common.h"
#include "assets/script_function.h"

struct Material;

enum CombinationFunction
{
  COMBINATION_INTERSECTION,
  COMBINATION_UNION,
  COMBINATION_SUBTRACTION
};

struct Geometry;

// ----------------------------------------------------------------------------
// Geometry common interface
// ----------------------------------------------------------------------------
ENGINE_API bool8 createGeometry(const std::string& name, Geometry** outGeometry);
ENGINE_API void destroyGeometry(Geometry* geometry);

ENGINE_API void geometrySetScale(Geometry* geometry, float32 scale);
ENGINE_API float32 geometryGetScale(Geometry* geometry);

ENGINE_API void geometrySetPosition(Geometry* geometry, float3 position);
ENGINE_API float3 geometryGetPosition(Geometry* geometry);

ENGINE_API void geometrySetOrientation(Geometry* geometry, quat orientation);
ENGINE_API quat geometryGetOrientation(Geometry* geometry);

ENGINE_API bool8 geometryRemoveFunction(Geometry* geometry, AssetPtr function);

ENGINE_API void geometryAddIDF(Geometry* geometry, AssetPtr idf);
ENGINE_API std::vector<AssetPtr>& geometryGetIDFs(Geometry* geometry);

ENGINE_API void geometryAddODF(Geometry* geometry, AssetPtr odf);
ENGINE_API std::vector<AssetPtr>& geometryGetODFs(Geometry* geometry);

// Get SDF, IDFs and ODFs as a single array
ENGINE_API std::vector<AssetPtr> geometryGetScriptFunctions(Geometry* geometry);

ENGINE_API void geometrySetName(Geometry* geometry, const std::string& name);
ENGINE_API const std::string& geometryGetName(Geometry* geometry);

ENGINE_API void geometrySetParent(Geometry* geometry, Geometry* parent);
ENGINE_API Geometry* geometryGetParent(Geometry* geometry);
ENGINE_API Geometry* geometryGetRoot(Geometry* geometry);

ENGINE_API bool8 geometryIsRoot(Geometry* geometry);
ENGINE_API bool8 geometryIsBranch(Geometry* geometry);
ENGINE_API bool8 geometryIsLeaf(Geometry* geometry);

// ENGINE_API AABB geometryGetAABB(Geometry* geometry);
// ENGINE_API float32 geometryGetRadius(Geometry* geometry);

ENGINE_API float3 geometryTransformToParent(Geometry* geometry, float3 p);
ENGINE_API float3 geometryTransformFromParent(Geometry* geometry, float3 p);
ENGINE_API float3 geometryTransformToLocal(Geometry* geometry, float3 p);
ENGINE_API float3 geometryTransformToWorld(Geometry* geometry, float3 p);

ENGINE_API float32 geometryCalculateDistanceToPoint(Geometry* geometry,
                                                    float3 p,
                                                    Geometry** outClosestLeafGeometry = nullptr);

// WARNING: Highly unoptimized!
ENGINE_API float3 geometryCalculateNormal(Geometry* geometry, float3 p);

// ----------------------------------------------------------------------------
// Branch geometry-related interface
// ----------------------------------------------------------------------------

ENGINE_API void geometryAddChild(Geometry* geometry, Geometry* child);
ENGINE_API bool8 geometryRemoveChild(Geometry* geometry, Geometry* child);
ENGINE_API std::vector<Geometry*>& geometryGetChildren(Geometry* geometry);

ENGINE_API void geometrySetCombinationFunction(Geometry* geometry, CombinationFunction function);
ENGINE_API CombinationFunction geometryGetCombinationFunction(Geometry* geometry);

// ----------------------------------------------------------------------------
// Leaf geometry-related interface
// ----------------------------------------------------------------------------

ENGINE_API void geometrySetSDF(Geometry* geometry, AssetPtr sdf);
ENGINE_API AssetPtr geometryGetSDF(Geometry* geometry);
ENGINE_API bool8 geometryHasSDF(Geometry* geometry);
