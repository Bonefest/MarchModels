#pragma once

#include "asset.h"

#include <set>
#include <string>
#include <vector>

#include "cvar_system.h"
#include "maths/common.h"
#include "shader_program.h"
#include "maths/primitives.h"
#include "assets/script_function.h"
#include "assets/pcf_script_function.h"

struct Material;

static const AssetType ASSET_TYPE_GEOMETRY = 0xe6593c2d;

// ----------------------------------------------------------------------------
// Geometry common interface
// ----------------------------------------------------------------------------
ENGINE_API bool8 createGeometry(const std::string& name, Asset** outGeometry);

// WARNING: Assumes that given geometry is a root
ENGINE_API void geometryUpdate(Asset* geometry, float64 delta);

ENGINE_API void geometrySetScale(Asset* geometry, float3 scale);
ENGINE_API float3 geometryGetScale(Asset* geometry);
ENGINE_API float3 geometryGetFullScale(Asset* geometry);

ENGINE_API void geometrySetPosition(Asset* geometry, float3 position);
ENGINE_API float3 geometryGetPosition(Asset* geometry);

ENGINE_API void geometrySetOrientation(Asset* geometry, quat orientation);
ENGINE_API quat geometryGetOrientation(Asset* geometry);

ENGINE_API void geometryAddFunction(Asset* geometry, AssetPtr function);
ENGINE_API bool8 geometryRemoveFunction(Asset* geometry, Asset* function);
ENGINE_API void geometryNotifyFunctionHasChanged(Asset* geometry, Asset* function);

ENGINE_API uint32 geometryGetID(Asset* geometry);

ENGINE_API std::vector<AssetPtr>& geometryGetIDFs(Asset* geometry);
ENGINE_API std::vector<AssetPtr>& geometryGetODFs(Asset* geometry);
ENGINE_API AssetPtr geometryGetPCF(Asset* geometry);
// Get SDF, IDFs and ODFs as a single array
ENGINE_API std::vector<AssetPtr> geometryGetScriptFunctions(Asset* geometry);
ENGINE_API bool8 geometryHasFunction(Asset* geometry, Asset* function);

ENGINE_API void geometrySetParent(Asset* geometry, AssetPtr parent);
ENGINE_API bool8 geometryHasParent(Asset* geometry);
ENGINE_API AssetPtr geometryGetParent(Asset* geometry);
ENGINE_API Asset* geometryGetRoot(Asset* geometry);

ENGINE_API bool8 geometryIsRoot(Asset* geometry);
ENGINE_API bool8 geometryIsBranch(Asset* geometry);
ENGINE_API bool8 geometryIsLeaf(Asset* geometry);

ENGINE_API void geometrySetSelected(Asset* geometry, bool8 selected);
ENGINE_API bool8 geometryIsSelected(Asset* geometry);

ENGINE_API float3 geometryTransformToParent(Asset* geometry, float3 localPos);
ENGINE_API float3 geometryTransformFromParent(Asset* geometry, float3 parentPos);
ENGINE_API float3 geometryTransformToLocal(Asset* geometry, float3 worldPos);
ENGINE_API float3 geometryTransformToWorld(Asset* geometry, float3 localPos);

ENGINE_API const float4x4& geometryGetWorldGeoMat(Asset* geometry);
ENGINE_API const float4x4& geometryGetGeoWorldMat(Asset* geometry);
ENGINE_API const float4x4& geometryGetParentGeoMat(Asset* geometry);
ENGINE_API const float4x4& geometryGetGeoParentMat(Asset* geometry);

ENGINE_API void geometrySetBounded(Asset* geometry, bool8 bounded);
ENGINE_API bool8 geometryIsBounded(Asset* geometry);

ENGINE_API void geometrySetEnabled(Asset* geometry, bool8 enabled);
ENGINE_API bool8 geometryIsEnabled(Asset* geometry);

ENGINE_API void geometrySetAABBAutomaticallyCalculated(Asset* geometry, bool8 automatically);
ENGINE_API bool8 geometryAABBIsAutomaticallyCalculated(Asset* geometry);

ENGINE_API void geometrySetNativeAABB(Asset* geometry, const AABB& nativeAABB);
ENGINE_API const AABB& geometryGetNativeAABB(Asset* geometry);
ENGINE_API const AABB& geometryGetDynamicAABB(Asset* geometry);
ENGINE_API const AABB& geometryGetFinalAABB(Asset* geometry);

ENGINE_API void geometryMarkNeedAABBRecalculation(Asset* geometry, bool8 markChildren = FALSE);
ENGINE_API bool8 geometryNeedAABBRecalculation(Asset* geometry);
ENGINE_API bool8 geometryNeedRebuild(Asset* geometry);

ENGINE_API ShaderProgramPtr geometryGetDrawProgram(Asset* geometry);
ENGINE_API ShaderProgramPtr geometryGetShadowProgram(Asset* geometry);
ENGINE_API ShaderProgramPtr geometryGetAABBProgram(Asset* geometry);

ENGINE_API PCFNativeType geometryGetPCFNativeType(Asset* geometry);

ENGINE_API AssetPtr geometryClone(Asset* geometry);
ENGINE_API void geometryCopy(AssetPtr geometryDst, Asset* geometrySrc);

typedef bool8(*fpTraverseFunction)(Asset* geometry, void* userData);
ENGINE_API bool8 geometryTraversePostorder(Asset* geometry,
                                           fpTraverseFunction traverseFunction,
                                           void* userData = nullptr);

const std::set<AssetPtr>& geometryRootGetAllChildren(Asset* root);

// ----------------------------------------------------------------------------
// Branch geometry-related interface
// ----------------------------------------------------------------------------

// NOTE: We need an AssetPtr because we need to set a parent for a child, hence,
// we cannot use a raw pointer as a parent ptr.
ENGINE_API void geometryAddChild(AssetPtr geometry, AssetPtr child);
ENGINE_API bool8 geometryRemoveChild(Asset* geometry, Asset* child);
ENGINE_API void geometryClearChildren(Asset* geometry);
ENGINE_API std::vector<AssetPtr>& geometryGetChildren(Asset* geometry);
ENGINE_API uint32 geometryGetTotalChildrenCount(Asset* geometry);

// ----------------------------------------------------------------------------
// Leaf geometry-related interface
// ----------------------------------------------------------------------------

ENGINE_API AssetPtr geometryGetSDF(Asset* geometry);
ENGINE_API bool8 geometryHasSDF(Asset* geometry);

ENGINE_API void geometrySetMaterial(Asset* geometry, AssetPtr material);
ENGINE_API AssetPtr geometryGetMaterial(Asset* geometry);


