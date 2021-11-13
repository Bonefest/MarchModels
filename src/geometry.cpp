#include "memory_manager.h"

#include "geometry.h"

struct Geometry
{
  // Common data
  std::string name;
  
  std::vector<AssetPtr> idfs;
  std::vector<AssetPtr> odfs;

  Geometry* parent;
  
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
  std::vector<Geometry*> children;  
  CombinationFunction combinationFunction;

  // Leaf geometry data
  AssetPtr sdf;
  Material* material;
};

// ----------------------------------------------------------------------------
// Helper functions
// ----------------------------------------------------------------------------

float32 combineDistances(Geometry* geometry,
                         float32 distanceA,
                         float32 distanceB,
                         Geometry* closestGeometryA,
                         Geometry* closestGeometryB,
                         Geometry** newClosestGeometry)
{
  assert(geometryIsBranch(geometry));

  float32 result = distanceA;
  
  // max(distanceA, distanceB)
  if(geometry->combinationFunction == COMBINATION_INTERSECTION)
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
  else if(geometry->combinationFunction == COMBINATION_UNION)
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
  else if(geometry->combinationFunction == COMBINATION_SUBTRACTION)
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

static void geometryRecalculateFullTransforms(Geometry* geometry, bool8 parentWasDirty = FALSE)
{
  // NOTE: If parent was dirty, then its matrices have been recalculated in some way, hence, even
  // if child is not dirty, its transforms still should be recalculated
  if(geometry->dirty == TRUE || parentWasDirty == TRUE)
  {
    float4x4 transformFromLocal = mul(translation_matrix(geometry->position),
                                      mul(rotation_matrix(geometry->orientation),
                                          scaling_matrix(float3(geometry->scale, geometry->scale, geometry->scale))));

    float4x4 transformToLocal = inverse(transformFromLocal);
  
    if(geometryIsRoot(geometry))
    {
      // NOTE: Root geometry doesn't have a parent => use identity matrix for parent-related
      // transforms
      geometry->transformToLocalFromParent = scaling_matrix(float3(1.0f, 1.0f, 1.0f));
      geometry->transformToParentFromLocal = scaling_matrix(float3(1.0f, 1.0f, 1.0f));

      geometry->transformToLocal = transformToLocal;
      geometry->transformToWorld = transformFromLocal;
    }
    else
    {
      geometry->transformToLocalFromParent = transformToLocal;
      geometry->transformToParentFromLocal = transformFromLocal;
      
      // NOTE: To convert from world to local, first use parent transform to parent's space
      // and only then apply transform from parent's space to the space of the current geometry
      geometry->transformToLocal = mul(transformToLocal, geometry->parent->transformToLocal);

      // NOTE: To convert from local to world, first convert to the parent space, for which transformation
      // from local space to world is known and use that.
      geometry->transformToWorld = mul(geometry->parent->transformToWorld, transformFromLocal);
    }
  }

  for(Geometry* child: geometry->children)
  {
    geometryRecalculateFullTransforms(child, geometry->dirty || parentWasDirty);
  }

  geometry->dirty = FALSE;      
}

static void geometryRecalculateTransforms(Geometry* geometry)
{
  geometryRecalculateFullTransforms(geometryGetRoot(geometry));
}

// ----------------------------------------------------------------------------
// Geometry common interface
// ----------------------------------------------------------------------------
bool8 createGeometry(const std::string& name, Geometry** outGeometry)
{
  *outGeometry = engineAllocObject<Geometry>(MEMORY_TYPE_GENERAL);
  Geometry* geometry = *outGeometry;
  geometry->name = name;
  geometry->combinationFunction = COMBINATION_UNION;
  geometry->scale = 1.0f;
  geometry->position = float3(0.0f, 0.0f, 0.0f);
  geometry->orientation = quat(0.0f, 0.0f, 1.0f, 0.0f);
  geometry->dirty = TRUE;

  return TRUE;
}

void destroyGeometry(Geometry* geometry)
{
  for(Geometry* child: geometry->children)
  {
    destroyGeometry(child);
  }

  auto functions = geometryGetScriptFunctions(geometry);
  for(AssetPtr function: functions)
  {
    destroyAsset(function);
  }
  
  engineFreeObject(geometry, MEMORY_TYPE_GENERAL);
}

void geometrySetScale(Geometry* geometry, float32 scale)
{
  geometry->dirty = TRUE;
  geometry->scale = scale;
}

float32 geometryGetScale(Geometry* geometry)
{
  return geometry->scale;
}

void geometrySetPosition(Geometry* geometry, float3 position)
{
  geometry->dirty = TRUE;
  geometry->position = position;
}

float3 geometryGetPosition(Geometry* geometry)
{
  return geometry->position;
}

void geometrySetOrientation(Geometry* geometry, quat orientation)
{
  geometry->dirty = TRUE;
  geometry->orientation = orientation;
}

quat geometryGetOrientation(Geometry* geometry)
{
  return geometry->orientation;
}

bool8 geometryRemoveFunction(Geometry* geometry, AssetPtr function)
{
  ScriptFunctionType type = scriptFunctionGetType(function);
  if(type == SCRIPT_FUNCTION_TYPE_SDF)
  {
    if(geometry->sdf != nullptr)
    {
      geometry->sdf = nullptr;
      return TRUE;
    }

    return FALSE;
  }
  else if(type == SCRIPT_FUNCTION_TYPE_IDF)
  {
    auto idfIt = std::find(geometry->idfs.begin(), geometry->idfs.end(), function);
    if(idfIt != geometry->idfs.end())
    {
      geometry->idfs.erase(idfIt);
      return TRUE;
    }

    return FALSE;
  }
  else if(type == SCRIPT_FUNCTION_TYPE_ODF)
  {
    auto odfIt = std::find(geometry->odfs.begin(), geometry->odfs.end(), function);
    if(odfIt != geometry->odfs.end())
    {
      geometry->odfs.erase(odfIt);
      return TRUE;
    }

    return FALSE;
  }

  return FALSE;
}

void geometryAddIDF(Geometry* geometry, AssetPtr idf)
{
  assert(assetGetType(idf) == ASSET_TYPE_SCRIPT_FUNCTION &&
         scriptFunctionGetType(idf) == SCRIPT_FUNCTION_TYPE_IDF);
  
  geometry->idfs.push_back(idf);
}

std::vector<AssetPtr>& geometryGetIDFs(Geometry* geometry)
{
  return geometry->idfs;
}

void geometryAddODF(Geometry* geometry, AssetPtr odf)
{
  assert(assetGetType(odf) == ASSET_TYPE_SCRIPT_FUNCTION &&
         scriptFunctionGetType(odf) == SCRIPT_FUNCTION_TYPE_ODF);

  geometry->odfs.push_back(odf);
}

std::vector<AssetPtr>& geometryGetODFs(Geometry* geometry)
{
  return geometry->odfs;
}

std::vector<AssetPtr> geometryGetScriptFunctions(Geometry* geometry)
{
  std::vector<AssetPtr> functions;
  functions.reserve(1 + geometry->idfs.size() + geometry->odfs.size());

  if(geometry->sdf != nullptr)
  {
    functions.push_back(geometry->sdf);
  }

  functions.insert(functions.end(), geometry->idfs.begin(), geometry->idfs.end());
  functions.insert(functions.end(), geometry->odfs.begin(), geometry->odfs.end());  

  return functions;
}

void geometrySetName(Geometry* geometry, const std::string& name)
{
  geometry->name = name;
}

const std::string& geometryGetName(Geometry* geometry)
{
  return geometry->name;
}

void geometrySetParent(Geometry* geometry, Geometry* parent)
{
  geometry->parent = parent;
}

Geometry* geometryGetParent(Geometry* geometry)
{
  return geometry->parent;
}

Geometry* geometryGetRoot(Geometry* geometry)
{
  if(geometryIsRoot(geometry))
  {
    return geometry;
  }

  return geometryGetRoot(geometry->parent);
}

bool8 geometryIsRoot(Geometry* geometry)
{
  return geometry->parent == nullptr;
}

bool8 geometryIsBranch(Geometry* geometry)
{
  return geometry->children.size() > 0;
}

bool8 geometryIsLeaf(Geometry* geometry)
{
  return geometry->children.size() == 0;
}

float3 geometryTransformToParent(Geometry* geometry, float3 p)
{
  if(geometry->dirty == TRUE)
  {
    geometryRecalculateTransforms(geometry);
  }

  float4 transformedP = mul(geometry->transformToParentFromLocal, float4(p.x, p.y, p.z, 1.0f));
  return swizzle<0, 1, 2>(transformedP);
}

float3 geometryTransformFromParent(Geometry* geometry, float3 p)
{
  if(geometry->dirty == TRUE)
  {
    geometryRecalculateTransforms(geometry);
  }

  float4 transformedP = mul(geometry->transformToLocalFromParent, float4(p.x, p.y, p.z, 1.0f));
  return swizzle<0, 1, 2>(transformedP);
}

float3 geometryTransformToLocal(Geometry* geometry, float3 p)
{
  if(geometry->dirty == TRUE)
  {
    geometryRecalculateTransforms(geometry);
  }

  float4 transformedP = mul(geometry->transformToLocal, float4(p.x, p.y, p.z, 1.0f));
  return swizzle<0, 1, 2>(transformedP);
}

float3 geometryTransformToWorld(Geometry* geometry, float3 p)
{
  if(geometry->dirty == TRUE)
  {
    geometryRecalculateTransforms(geometry);
  }

  float4 transformedP = mul(geometry->transformToWorld, float4(p.x, p.y, p.z, 1.0f));
  return swizzle<0, 1, 2>(transformedP);
}

float32 geometryCalculateDistanceToPoint(Geometry* geometry,
                                         float3 p,
                                         Geometry** outClosestLeafGeometry)
{
  for(AssetPtr idf: geometry->idfs)
  {
    p = executeIDF(idf, p);
  }

  float32 distance = 0.0f;
  
  if(geometryIsBranch(geometry))
  {
    Geometry* closestGeometry;
    
    float32 closestDistance = geometryCalculateDistanceToPoint(geometry->children.front(), p, &closestGeometry);
    for(auto childIt = geometry->children.begin() + 1; childIt != geometry->children.end(); childIt++)
    {
      Geometry* childClosestGeometry;
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
    
    distance = executeSDF(geometry->sdf, transformedP);
    if(outClosestLeafGeometry != nullptr)
    {
      *outClosestLeafGeometry = geometry;
    }
  }

  for(AssetPtr odf: geometry->odfs)
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
float3 geometryCalculateNormal(Geometry* geometry, float3 p)
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

void geometryAddChild(Geometry* geometry, Geometry* child)
{
  geometry->children.push_back(child);
}

bool8 geometryRemoveChild(Geometry* geometry, Geometry* child)
{
  auto childIt = std::find(geometry->children.begin(), geometry->children.end(), child);
  if(childIt == geometry->children.end())
  {
    return FALSE;
  }

  geometry->children.erase(childIt);
  
  return TRUE;
}

std::vector<Geometry*>& geometryGetChildren(Geometry* geometry)
{
  return geometry->children;
}

void geometrySetCombinationFunction(Geometry* geometry, CombinationFunction function)
{
  geometry->combinationFunction = function;
}

CombinationFunction geometryGetCombinationFunction(Geometry* geometry)
{
  return geometry->combinationFunction;
}

// ----------------------------------------------------------------------------
// Leaft geometry-related interface
// ----------------------------------------------------------------------------

void geometrySetSDF(Geometry* geometry, AssetPtr sdf)
{
  assert(assetGetType(sdf) == ASSET_TYPE_SCRIPT_FUNCTION &&
         scriptFunctionGetType(sdf) == SCRIPT_FUNCTION_TYPE_SDF);
  
  geometry->sdf = sdf;
}

AssetPtr geometryGetSDF(Geometry* geometry)
{
  return geometry->sdf;
}

bool8 geometryHasSDF(Geometry* geometry)
{
  return geometry->sdf != nullptr;
}
