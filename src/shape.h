#pragma once
                          
enum CombinationFunction
{
  COMBINATION_INTERSECTION,
  COMBINATION_UNION,
  COMBINATION_SUBTRACTION
};

struct Shape
{
  // Common data
  string name;
  
  vector<IDF*> idfs;
  vector<ODF*> odfs;

  float32 scale;
  float3 position;
  quat orientation;

  // Branch shape data
  vector<Shape*> children;  
  CombinationFunction combinationFunction;

  // Leaf shape data
  Material* material;
};


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
