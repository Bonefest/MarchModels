#pragma once

#include <vector>
#include <string>

using std::string;
using std::vector;


void deleteLuaFunction();

float32 executeSDF(LuaFunction* function, float3 p, const njson& arguments);
float32 executeODF(LuaFunction* function, float distance, const njson& arguments);
float3 executeIDF(LuaFunction* function, float3 p, const njson& arguments);

                          
enum CombinationFunction
{
  COMBINATION_INTERSECTION,
  COMBINATION_UNION,
  COMBINATION_SUBTRACTION
};

struct Primitive
{
  string name;
  SDF* sdf;
  vector<DF*> inputDeformations;
  vector<DF*> outputDeformations;
};


// User can register:
// 1. SDF (Signed Distance Function)
// 2. ODF (Ouput Deformation Function)
// 3. IDF (Input Deformation Function)
// 4. Primitive (SDF + list<IDF + arguments> + list<ODF + arguments> + Material)
// 5. Shape:
//   Common: list<IDF + arguments>, list<ODF + arguments>, Local Transform
//   Branch: List<Shape> Combination
//   Leaf: Instance of registered primitive (its params can be overriden)
