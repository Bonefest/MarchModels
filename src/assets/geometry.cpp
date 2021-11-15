#include "memory_manager.h"

#include "geometry.h"

struct Geometry
{
  // Common data
  std::vector<AssetPtr> idfs;
  std::vector<AssetPtr> odfs;

  AssetPtr parent;
  
  float32 scale;
  float3 origin;
  float3 position;
  quat orientation;

  float4x4 transformToLocal;
  float4x4 transformToWorld;
  
  float4x4 transformToLocalFromParent;
  float4x4 transformToParentFromLocal;
  
  bool8 dirty;
  
  // Branch geometry data
  std::vector<AssetPtr> children;  
  CombinationFunction combinationFunction;

  // Leaf geometry data
  AssetPtr sdf;
  Material* material;
};

static void geometryDestroy(Asset* geometry);
static bool8 geometrySerialize(Asset* geometry) { /** TODO */ }
static bool8 geometryDeserialize(Asset* geometry) { /** TODO */ }
static uint32 geometryGetSize(Asset* geometry) { /** TODO */ }

// ----------------------------------------------------------------------------
// Helper functions
// ----------------------------------------------------------------------------

float32 combineDistances(Asset* geometry,
                         float32 distanceA,
                         float32 distanceB,
                         Asset* closestGeometryA,
                         Asset* closestGeometryB,
                         Asset** newClosestGeometry)
{
  assert(geometryIsBranch(geometry));

  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  
  float32 result = distanceA;
  
  // max(distanceA, distanceB)
  if(geometryData->combinationFunction == COMBINATION_INTERSECTION)
  {
    if(distanceA > distanceB)
    {
      result = distanceA;
      *newClosestGeometry = closestGeometryA;
    }
    else
    {
      result = distanceB;
      *newClosestGeometry = closestGeometryB;
    }
  }
  // min(distanceA, distanceB)
  else if(geometryData->combinationFunction == COMBINATION_UNION)
  {
    if(distanceA > distanceB)
    {
      result = distanceB;
      *newClosestGeometry = closestGeometryB;
    }
    else
    {
      result = distanceA;
      *newClosestGeometry = closestGeometryA;
    }    
  }
  // max(distanceA, -distanceB)
  else if(geometryData->combinationFunction == COMBINATION_SUBTRACTION)
  {
    if(distanceA > -distanceB)
    {
      result = distanceA;
      *newClosestGeometry = closestGeometryB;
    }
    else
    {
      assert(FALSE && "Is it possible at all?");
    }    
  }

  return result;
}

static void geometryRecalculateFullTransforms(Asset* geometry, bool8 parentWasDirty = FALSE)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  
  // NOTE: If parent was dirty, then its matrices have been recalculated in some way, hence, even
  // if child is not dirty, its transforms still should be recalculated
  if(geometryData->dirty == TRUE || parentWasDirty == TRUE)
  {
    float4x4 transformFromLocal = mul(translation_matrix(geometryData->position),
                                      rotation_matrix(geometryData->orientation),
                                      scaling_matrix(float3(geometryData->scale,
                                                            geometryData->scale,
                                                            geometryData->scale)));

    float4x4 transformToLocal = inverse(transformFromLocal);
  
    if(geometryIsRoot(geometry))
    {
      // NOTE: Root geometry doesn't have a parent => use identity matrix for parent-related
      // transforms
      geometryData->transformToLocalFromParent = scaling_matrix(float3(1.0f, 1.0f, 1.0f));
      geometryData->transformToParentFromLocal = scaling_matrix(float3(1.0f, 1.0f, 1.0f));

      geometryData->transformToLocal = transformToLocal;
      geometryData->transformToWorld = transformFromLocal;
    }
    else
    {
      geometryData->transformToLocalFromParent = transformToLocal;
      geometryData->transformToParentFromLocal = transformFromLocal;
      
      // NOTE: To convert from world to local, first use parent transform to parent's space
      // and only then apply transform from parent's space to the space of the current geometry
      Geometry* parentData = (Geometry*)assetGetInternalData(geometryData->parent);
      geometryData->transformToLocal = mul(transformToLocal, parentData->transformToLocal);

      // NOTE: To convert from local to world, first convert to the parent space, for which transformation
      // from local space to world is known and use that.
      geometryData->transformToWorld = mul(parentData->transformToWorld, transformFromLocal);
    }
  }

  for(Asset* child: geometryData->children)
  {
    geometryRecalculateFullTransforms(child, geometryData->dirty || parentWasDirty);
  }

  geometryData->dirty = FALSE;      
}

static void geometryRecalculateTransforms(Asset* geometry)
{
  geometryRecalculateFullTransforms(geometryGetRoot(geometry));
}

// ----------------------------------------------------p------------------------
// Geometry common interface
// ----------------------------------------------------------------------------
bool8 createGeometry(const std::string& name, Asset** outGeometry)
{
  AssetInterface interface = {};
  interface.destroy = geometryDestroy;
  interface.serialize = geometrySerialize;
  interface.deserialize = geometryDeserialize;
  interface.getSize = geometryGetSize;
  interface.type = ASSET_TYPE_GEOMETRY;

  assert(allocateAsset(interface, name, outGeometry));

  Geometry* geometryData = engineAllocObject<Geometry>(MEMORY_TYPE_GENERAL);
  geometryData->combinationFunction = COMBINATION_UNION;
  geometryData->scale = 1.0f;
  geometryData->position = float3(0.0f, 0.0f, 0.0f);
  geometryData->orientation = quat(0.0f, 0.0f, 1.0f, 0.0f);
  geometryData->dirty = TRUE;

  assetSetInternalData(*outGeometry, geometryData);
  
  return TRUE;
}

void geometryDestroy(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);

  geometryData->parent = AssetPtr(nullptr);
  geometryData->children.clear();
  geometryData->idfs.clear();
  geometryData->odfs.clear();
  geometryData->sdf = AssetPtr(nullptr);
  
  engineFreeObject(geometryData, MEMORY_TYPE_GENERAL);
}

void geometrySetScale(Asset* geometry, float32 scale)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  
  geometryData->dirty = TRUE;
  geometryData->scale = scale;
}

float32 geometryGetScale(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  
  return geometryData->scale;
}

void geometrySetPosition(Asset* geometry, float3 position)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  
  geometryData->dirty = TRUE;
  geometryData->position = position;
}

float3 geometryGetPosition(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  
  return geometryData->position;
}

void geometrySetOrientation(Asset* geometry, quat orientation)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  
  geometryData->dirty = TRUE;
  geometryData->orientation = orientation;
}

quat geometryGetOrientation(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  
  return geometryData->orientation;
}

void geometryAddFunction(Asset* geometry, AssetPtr function)
{
  ScriptFunctionType type = scriptFunctionGetType(function);
  if(type == SCRIPT_FUNCTION_TYPE_SDF)
  {
    geometrySetSDF(geometry, function);
  }
  else if(type == SCRIPT_FUNCTION_TYPE_IDF)
  {
    geometryAddIDF(geometry, function);
  }
  else if(type == SCRIPT_FUNCTION_TYPE_ODF)
  {
    geometryAddODF(geometry, function);
  }
}

bool8 geometryRemoveFunction(Asset* geometry, Asset* function)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  
  ScriptFunctionType type = scriptFunctionGetType(function);
  if(type == SCRIPT_FUNCTION_TYPE_SDF)
  {
    if(geometryData->sdf != nullptr)
    {
      geometryData->sdf = AssetPtr(nullptr);
      return TRUE;
    }

    return FALSE;
  }
  else if(type == SCRIPT_FUNCTION_TYPE_IDF)
  {
    auto idfIt = std::find(geometryData->idfs.begin(), geometryData->idfs.end(), function);
    if(idfIt != geometryData->idfs.end())
    {
      geometryData->idfs.erase(idfIt);
      return TRUE;
    }

    return FALSE;
  }
  else if(type == SCRIPT_FUNCTION_TYPE_ODF)
  {
    auto odfIt = std::find(geometryData->odfs.begin(), geometryData->odfs.end(), function);
    if(odfIt != geometryData->odfs.end())
    {
      geometryData->odfs.erase(odfIt);
      return TRUE;
    }

    return FALSE;
  }

  return FALSE;
}

void geometryAddIDF(Asset* geometry, AssetPtr idf)
{
  assert(assetGetType(idf) == ASSET_TYPE_SCRIPT_FUNCTION &&
         scriptFunctionGetType(idf) == SCRIPT_FUNCTION_TYPE_IDF);

  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  
  geometryData->idfs.push_back(idf);
}

std::vector<AssetPtr>& geometryGetIDFs(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  
  return geometryData->idfs;
}

void geometryAddODF(Asset* geometry, AssetPtr odf)
{
  assert(assetGetType(odf) == ASSET_TYPE_SCRIPT_FUNCTION &&
         scriptFunctionGetType(odf) == SCRIPT_FUNCTION_TYPE_ODF);

  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  
  geometryData->odfs.push_back(odf);
}

std::vector<AssetPtr>& geometryGetODFs(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  
  return geometryData->odfs;
}

std::vector<AssetPtr> geometryGetScriptFunctions(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  
  std::vector<AssetPtr> functions;
  functions.reserve(1 + geometryData->idfs.size() + geometryData->odfs.size());

  if(geometryData->sdf != nullptr)
  {
    functions.push_back(geometryData->sdf);
  }

  functions.insert(functions.end(), geometryData->idfs.begin(), geometryData->idfs.end());
  functions.insert(functions.end(), geometryData->odfs.begin(), geometryData->odfs.end());  

  return functions;
}

void geometrySetParent(Asset* geometry, AssetPtr parent)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  
  geometryData->parent = parent;
}

AssetPtr geometryGetParent(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  
  return geometryData->parent;
}

Asset* geometryGetRoot(Asset* geometry)
{
  if(geometryIsRoot(geometry))
  {
    return geometry;
  }

  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  
  return geometryGetRoot(geometryData->parent);
}

bool8 geometryIsRoot(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);  

  return geometryData->parent == nullptr;
}

bool8 geometryIsBranch(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  
  return geometryData->children.size() > 0;
}

bool8 geometryIsLeaf(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  
  return geometryData->children.size() == 0;
}

float3 geometryTransformToParent(Asset* geometry, float3 p)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  
  if(geometryData->dirty == TRUE)
  {
    geometryRecalculateTransforms(geometry);
  }

  float4 transformedP = mul(geometryData->transformToParentFromLocal, float4(p.x, p.y, p.z, 1.0f));
  return swizzle<0, 1, 2>(transformedP);
}

float3 geometryTransformFromParent(Asset* geometry, float3 p)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  
  if(geometryData->dirty == TRUE)
  {
    geometryRecalculateTransforms(geometry);
  }

  float4 transformedP = mul(geometryData->transformToLocalFromParent, float4(p.x, p.y, p.z, 1.0f));
  return swizzle<0, 1, 2>(transformedP);
}

float3 geometryTransformToLocal(Asset* geometry, float3 p)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  
  if(geometryData->dirty == TRUE)
  {
    geometryRecalculateTransforms(geometry);
  }

  float4 transformedP = mul(geometryData->transformToLocal, float4(p.x, p.y, p.z, 1.0f));
  return swizzle<0, 1, 2>(transformedP);
}

float3 geometryTransformToWorld(Asset* geometry, float3 p)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  
  if(geometryData->dirty == TRUE)
  {
    geometryRecalculateTransforms(geometry);
  }

  float4 transformedP = mul(geometryData->transformToWorld, float4(p.x, p.y, p.z, 1.0f));
  return swizzle<0, 1, 2>(transformedP);
}

float32 geometryCalculateDistanceToPoint(Asset* geometry,
                                         float3 p,
                                         Asset** outClosestLeafGeometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  
  for(AssetPtr idf: geometryData->idfs)
  {
    p = executeIDF(idf, p);
  }

  float32 distance = 0.0f;
  
  if(geometryIsBranch(geometry))
  {
    Asset* closestGeometry;
    
    float32 closestDistance = geometryCalculateDistanceToPoint(geometryData->children.front(), p, &closestGeometry);
    for(auto childIt = geometryData->children.begin() + 1; childIt != geometryData->children.end(); childIt++)
    {
      Asset* childClosestGeometry;
      float32 childDistance = geometryCalculateDistanceToPoint(*childIt, p, &childClosestGeometry);

      distance = combineDistances(geometry,
                                  closestDistance,
                                  childDistance,
                                  closestGeometry,
                                  childClosestGeometry,
                                  &closestGeometry);
    }

    distance = closestDistance;
  }
  else
  {
    // NOTE: To use transformations in SDF, we need to translate point in local space. That's because
    // SDF is expressed in local space. For example, sdf of sphere should be expressed relative to the
    // local origin.
    //
    // TODO: If a geometry has scaling, this transformation will be wrong!
    float3 transformedP = geometryTransformToLocal(geometry, p);
    
    distance = executeSDF(geometryData->sdf, transformedP);
    if(outClosestLeafGeometry != nullptr)
    {
      *outClosestLeafGeometry = geometry;
    }
  }

  for(AssetPtr odf: geometryData->odfs)
  {
    distance = executeODF(odf, distance);
  }

  return distance;
}

// NOTE: Calculates normal using finite differences.
// We can optimized it in the next ways:
//   1) Pass already calculated distance, so we don't need to calculate it here
//   2) Pass already found leaf geometry and use only its sdf, so we don't need to traverse
//      the whole geometry's hierarchy (we still need to consider parent's ODFs/IDFs though)
float3 geometryCalculateNormal(Asset* geometry, float3 p)
{
  // NOTE: If geometry is not root - find its root and use that instead.
  // We need to use root of the hierarchy, because otherwise IDFs and ODFs of its parents won't
  // be accounted for.
  if(!geometryIsRoot(geometry))
  {
    return geometryCalculateNormal(geometryGetRoot(geometry), p);
  }

  float32 distance = geometryCalculateDistanceToPoint(geometry, p);
  
  float32 step = 0.001f;
  float32 x = geometryCalculateDistanceToPoint(geometry, p + float3(step, 0.0f, 0.0f)) - distance;
  float32 y = geometryCalculateDistanceToPoint(geometry, p + float3(0.0f, step, 0.0f)) - distance;
  float32 z = geometryCalculateDistanceToPoint(geometry, p + float3(0.0f, 0.0f, step)) - distance;    

  return normalize(float3(x, y, z) / step);
}

// ----------------------------------------------------------------------------
// Branch geometry-related interface
// ----------------------------------------------------------------------------

void geometryAddChild(Asset* geometry, AssetPtr child)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);  
  
  geometryData->children.push_back(child);
}

bool8 geometryRemoveChild(Asset* geometry, Asset* child)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);  
  
  auto childIt = std::find(geometryData->children.begin(), geometryData->children.end(), child);
  if(childIt == geometryData->children.end())
  {
    return FALSE;
  }

  geometryData->children.erase(childIt);
  
  return TRUE;
}

std::vector<AssetPtr>& geometryGetChildren(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);  
  
  return geometryData->children;
}

void geometrySetCombinationFunction(Asset* geometry, CombinationFunction function)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);  
  
  geometryData->combinationFunction = function;
}

CombinationFunction geometryGetCombinationFunction(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);  
  
  return geometryData->combinationFunction;
}

// ----------------------------------------------------------------------------
// Leaft geometry-related interface
// ----------------------------------------------------------------------------

void geometrySetSDF(Asset* geometry, AssetPtr sdf)
{
  assert(assetGetType(sdf) == ASSET_TYPE_SCRIPT_FUNCTION &&
         scriptFunctionGetType(sdf) == SCRIPT_FUNCTION_TYPE_SDF);

  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);  
  
  geometryData->sdf = sdf;
}

AssetPtr geometryGetSDF(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);  
  
  return geometryData->sdf;
}

bool8 geometryHasSDF(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);  
  
  return geometryData->sdf != nullptr;
}
