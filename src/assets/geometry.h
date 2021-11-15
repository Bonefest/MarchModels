#pragma once

#include "asset.h"



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

static const AssetType ASSET_TYPE_GEOMETRY = 0xe6593c2d;

// ----------------------------------------------------------------------------
// Geometry common interface
// ----------------------------------------------------------------------------
ENGINE_API bool8 createGeometry(const std::string& name, Asset** outGeometry);

ENGINE_API void geometrySetScale(Asset* geometry, float32 scale);
ENGINE_API float32 geometryGetScale(Asset* geometry);

ENGINE_API void geometrySetPosition(Asset* geometry, float3 position);
ENGINE_API float3 geometryGetPosition(Asset* geometry);

ENGINE_API void geometrySetOrientation(Asset* geometry, quat orientation);
ENGINE_API quat geometryGetOrientation(Asset* geometry);

ENGINE_API void geometryAddFunction(Asset* geometry, AssetPtr function);
ENGINE_API bool8 geometryRemoveFunction(Asset* geometry, Asset* function);

ENGINE_API void geometryAddIDF(Asset* geometry, AssetPtr idf);
ENGINE_API std::vector<AssetPtr>& geometryGetIDFs(Asset* geometry);

ENGINE_API void geometryAddODF(Asset* geometry, AssetPtr odf);
ENGINE_API std::vector<AssetPtr>& geometryGetODFs(Asset* geometry);

// Get SDF, IDFs and ODFs as a single array
ENGINE_API std::vector<AssetPtr> geometryGetScriptFunctions(Asset* geometry);

ENGINE_API void geometrySetParent(Asset* geometry, AssetPtr parent);
ENGINE_API AssetPtr geometryGetParent(Asset* geometry);
ENGINE_API Asset* geometryGetRoot(Asset* geometry);

ENGINE_API bool8 geometryIsRoot(Asset* geometry);
ENGINE_API bool8 geometryIsBranch(Asset* geometry);
ENGINE_API bool8 geometryIsLeaf(Asset* geometry);

// ENGINE_API AABB geometryGetAABB(Asset* geometry);
// ENGINE_API float32 geometryGetRadius(Asset* geometry);

ENGINE_API float3 geometryTransformToParent(Asset* geometry, float3 p);
ENGINE_API float3 geometryTransformFromParent(Asset* geometry, float3 p);
ENGINE_API float3 geometryTransformToLocal(Asset* geometry, float3 p);
ENGINE_API float3 geometryTransformToWorld(Asset* geometry, float3 p);

ENGINE_API float32 geometryCalculateDistanceToPoint(Asset* geometry,
                                                    float3 p,
                                                    Asset** outClosestLeafGeometry = nullptr);

// WARNING: Highly unoptimized!
ENGINE_API float3 geometryCalculateNormal(Asset* geometry, float3 p);

// ----------------------------------------------------------------------------
// Branch geometry-related interface
// ----------------------------------------------------------------------------

ENGINE_API void geometryAddChild(Asset* geometry, AssetPtr child);
ENGINE_API bool8 geometryRemoveChild(Asset* geometry, Asset* child);
ENGINE_API std::vector<AssetPtr>& geometryGetChildren(Asset* geometry);

ENGINE_API void geometrySetCombinationFunction(Asset* geometry, CombinationFunction function);
ENGINE_API CombinationFunction geometryGetCombinationFunction(Asset* geometry);

// ----------------------------------------------------------------------------
// Leaf geometry-related interface
// ----------------------------------------------------------------------------

ENGINE_API void geometrySetSDF(Asset* geometry, AssetPtr sdf);
ENGINE_API AssetPtr geometryGetSDF(Asset* geometry);
ENGINE_API bool8 geometryHasSDF(Asset* geometry);


