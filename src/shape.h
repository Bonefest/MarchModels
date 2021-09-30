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
  Material* material;
};

float3 shapeCalculateNormal(Shape* shape, Ray ray);

// User can register:
// 1. SDF (Signed Distance Function)
// 2. ODF (Ouput Deformation Function)
// 3. IDF (Input Deformation Function)
// 4. Shape:
//   Common: list<IDF + arguments>, list<ODF + arguments>, Local Transform
//   Branch: List<Shape> Combination
//   Leaf: Material
//
// If user needs to create a prototype, she can create a new shape with a single
// leaf.
