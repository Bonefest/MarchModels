#include "memory_manager.h"

#include "shape.h"

struct Shape
{
  // Common data
  std::string name;
  
  std::vector<ScriptFunction*> idfs;
  std::vector<ScriptFunction*> odfs;

  float32 scale;
  float3 position;
  quat orientation;

  Shape* parent;
  
  // Branch shape data
  std::vector<Shape*> children;  
  CombinationFunction combinationFunction;

  // Leaf shape data
  ScriptFunction* sdf;
  Material* material;
};

// ----------------------------------------------------------------------------
// Helper functions
// ----------------------------------------------------------------------------

float32 combineDistances(Shape* shape,
                         float32 distanceA,
                         float32 distanceB,
                         Shape* closestShapeA,
                         Shape* closestShapeB,
                         Shape** newClosestShape)
{
  assert(shapeIsBranch(shape));

  float32 result = distanceA;
  
  // max(distanceA, distanceB)
  if(shape->combinationFunction == COMBINATION_INTERSECTION)
  {
    if(distanceA > distanceB)
    {
      result = distanceA;
      *newClosestShape = closestShapeA;
    }
    else
    {
      result = distanceB;
      *newClosestShape = closestShapeB;
    }
  }
  // min(distanceA, distanceB)
  else if(shape->combinationFunction == COMBINATION_UNION)
  {
    if(distanceA > distanceB)
    {
      result = distanceB;
      *newClosestShape = closestShapeB;
    }
    else
    {
      result = distanceA;
      *newClosestShape = closestShapeA;
    }    
  }
  // max(distanceA, -distanceB)
  else if(shape->combinationFunction == COMBINATION_SUBTRACTION)
  {
    if(distanceA > -distanceB)
    {
      result = distanceA;
      *newClosestShape = closestShapeB;
    }
    else
    {
      assert(FALSE && "Is it possible at all?");
    }    
  }

  return result;
}

// ----------------------------------------------------------------------------
// Shape common interface
// ----------------------------------------------------------------------------
bool8 createShape(const std::string& name, Shape** outShape)
{
  *outShape = engineAllocObject<Shape>(MEMORY_TYPE_GENERAL);
  Shape* shape = *outShape;
  shape->name = name;
  shape->combinationFunction = COMBINATION_UNION;
  shape->scale = 1.0f;
  shape->position = float3(0.0f, 0.0f, 0.0f);
  shape->orientation = quat(0.0f, 0.0f, 1.0f, 0.0f);

  return TRUE;
}

void destroyShape(Shape* shape)
{
  engineFreeObject(shape, MEMORY_TYPE_GENERAL);
}

void shapeSetScale(Shape* shape, float32 scale)
{
  shape->scale = scale;
}

float32 shapeGetScale(Shape* shape)
{
  return shape->scale;
}

void shapeSetPosition(Shape* shape, float3 position)
{
  shape->position = position;
}

float3 shapeGetPosition(Shape* shape)
{
  return shape->position;
}

void shapeSetOrientation(Shape* shape, quat orientation)
{
  shape->orientation = orientation;
}

quat shapeGetOrientation(Shape* shape)
{
  return shape->orientation;
}

void shapeAddIDF(Shape* shape, ScriptFunction* idf)
{
  shape->idfs.push_back(idf);
}

std::vector<ScriptFunction*>& shapeGetIDFs(Shape* shape)
{
  return shape->idfs;
}

void shapeAddODF(Shape* shape, ScriptFunction* odf)
{
  shape->odfs.push_back(odf);
}

std::vector<ScriptFunction*>& shapeGetODFs(Shape* shape)
{
  return shape->odfs;
}

void shapeSetName(Shape* shape, const std::string& name)
{
  shape->name = name;
}

const std::string& shapeGetName(Shape* shape)
{
  return shape->name;
}

void shapeSetParent(Shape* shape, Shape* parent)
{
  shape->parent = parent;
}

Shape* shapeGetParent(Shape* shape)
{
  return shape->parent;
}

bool8 shapeIsRoot(Shape* shape)
{
  return shape->parent == nullptr;
}

bool8 shapeIsBranch(Shape* shape)
{
  return shape->children.size() > 0;
}

bool8 shapeIsLeaf(Shape* shape)
{
  return shape->children.size() == 0;
}

float32 shapeCalculateDistanceToPoint(Shape* shape, float3 p, Shape** outClosestLeafShape)
{
  for(ScriptFunction* idf: shape->idfs)
  {
    p = executeIDF(idf, p);
  }

  float32 distance = 0.0f;
  
  if(shapeIsBranch(shape))
  {
    Shape* closestShape;
    
    float32 closestDistance = shapeCalculateDistanceToPoint(shape->children.front(), p, &closestShape);
    for(auto childIt = shape->children.begin() + 1; childIt != shape->children.end(); childIt++)
    {
      Shape* childClosestShape;
      float32 childDistance = shapeCalculateDistanceToPoint(*childIt, p, &childClosestShape);

      distance = combineDistances(shape,
                                  closestDistance,
                                  childDistance,
                                  closestShape,
                                  childClosestShape,
                                  &closestShape);
    }

    distance = closestDistance;
  }
  else
  {
    distance = executeSDF(shape->sdf, p);
    if(outClosestLeafShape != nullptr)
    {
      *outClosestLeafShape = shape;
    }
  }

  for(ScriptFunction* odf: shape->odfs)
  {
    distance = executeODF(odf, distance);
  }

  return distance;
}

// NOTE: Calculates normal using finite differences.
// We can optimized it in the next ways:
//   1) Pass already calculated distance, so we don't need to calculate it here
//   2) Pass already found leaf shape and use only its sdf, so we don't need to traverse
//      the whole shape's hierarchy (we still need to consider parent's ODFs/IDFs though)
float3 shapeCalculateNormal(Shape* shape, float3 p)
{
  float32 distance = shapeCalculateDistanceToPoint(shape, p);
  
  float32 step = 0.001f;
  float32 x = shapeCalculateDistanceToPoint(shape, p + float3(step, 0.0f, 0.0f)) - distance;
  float32 y = shapeCalculateDistanceToPoint(shape, p + float3(0.0f, step, 0.0f)) - distance;
  float32 z = shapeCalculateDistanceToPoint(shape, p + float3(0.0f, 0.0f, step)) - distance;    

  return normalize(float3(x, y, z) / step);
}

// ----------------------------------------------------------------------------
// Branch shape-related interface
// ----------------------------------------------------------------------------

void shapeAddChild(Shape* shape, Shape* child)
{
  shape->children.push_back(child);
}

bool8 shapeRemoveChild(Shape* shape, Shape* child)
{
  auto childIt = std::find(shape->children.begin(), shape->children.end(), child);
  if(childIt == shape->children.end())
  {
    return FALSE;
  }

  shape->children.erase(childIt);
  
  return TRUE;
}

void shapeSetCombinationFunction(Shape* shape, CombinationFunction function)
{
  shape->combinationFunction = function;
}

CombinationFunction shapeGetCombinationFunction(Shape* shape)
{
  return shape->combinationFunction;
}

// ----------------------------------------------------------------------------
// Leaft shape-related interface
// ----------------------------------------------------------------------------

void shapeSetSDF(Shape* shape, ScriptFunction* sdf)
{
  shape->sdf = sdf;
}

ScriptFunction* shapeGetSDF(Shape* shape)
{
  return shape->sdf;
}

