#include "memory_manager.h"

#include "geometry.h"

struct Geometry
{
  // Common data
  std::string name;
  
  std::vector<ScriptFunction*> idfs;
  std::vector<ScriptFunction*> odfs;

  float32 scale;
  float3 position;
  quat orientation;

  Geometry* parent;
  
  // Branch geometry data
  std::vector<Geometry*> children;  
  CombinationFunction combinationFunction;

  // Leaf geometry data
  ScriptFunction* sdf;
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

  return TRUE;
}

void destroyGeometry(Geometry* geometry)
{
  engineFreeObject(geometry, MEMORY_TYPE_GENERAL);
}

void geometrySetScale(Geometry* geometry, float32 scale)
{
  geometry->scale = scale;
}

float32 geometryGetScale(Geometry* geometry)
{
  return geometry->scale;
}

void geometrySetPosition(Geometry* geometry, float3 position)
{
  geometry->position = position;
}

float3 geometryGetPosition(Geometry* geometry)
{
  return geometry->position;
}

void geometrySetOrientation(Geometry* geometry, quat orientation)
{
  geometry->orientation = orientation;
}

quat geometryGetOrientation(Geometry* geometry)
{
  return geometry->orientation;
}

void geometryAddIDF(Geometry* geometry, ScriptFunction* idf)
{
  geometry->idfs.push_back(idf);
}

std::vector<ScriptFunction*>& geometryGetIDFs(Geometry* geometry)
{
  return geometry->idfs;
}

void geometryAddODF(Geometry* geometry, ScriptFunction* odf)
{
  geometry->odfs.push_back(odf);
}

std::vector<ScriptFunction*>& geometryGetODFs(Geometry* geometry)
{
  return geometry->odfs;
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

float32 geometryCalculateDistanceToPoint(Geometry* geometry,
                                         float3 p,
                                         Geometry** outClosestLeafGeometry)
{
  for(ScriptFunction* idf: geometry->idfs)
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
    distance = executeSDF(geometry->sdf, p);
    if(outClosestLeafGeometry != nullptr)
    {
      *outClosestLeafGeometry = geometry;
    }
  }

  for(ScriptFunction* odf: geometry->odfs)
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

void geometrySetSDF(Geometry* geometry, ScriptFunction* sdf)
{
  geometry->sdf = sdf;
}

ScriptFunction* geometryGetSDF(Geometry* geometry)
{
  return geometry->sdf;
}

