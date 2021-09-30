#include "shape.h"

struct Shape
{
  // Common data
  std::string name;
  
  std::vector<IDF*> idfs;
  std::vector<ODF*> odfs;

  float32 scale;
  float3 position;
  quat orientation;

  Shape* parent;
  
  // Branch shape data
  std::vector<Shape*> children;  
  CombinationFunction combinationFunction;

  // Leaf shape data
  SDF* sdf;
  Material* material;
};

