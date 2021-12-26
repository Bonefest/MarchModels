#include <string>
#include <vector>
using std::string;
using std::vector;

#include "logging.h"
#include "shader_build.h"
#include "shader_manager.h"
#include "memory_manager.h"

#include "geometry.h"

struct Geometry
{
  // Common data
  uint32 ID;
  
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

  ShaderProgram* program;

  bool8 needRebuild;
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

const char* combinationFunctionLabel(CombinationFunction function)
{
  static const char* labels[] =
  {
    "Intersection",
    "Union",
    "Subtraction"
  };

  return labels[(uint32)function];
}

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

static void geometryCollectParents(Asset* geometry, vector<Asset*>& collection)
{
  if(geometry == nullptr)
  {
    return;
  }
  
  if(geometryIsRoot(geometry) == FALSE)
  {
    geometryCollectParents(geometryGetParent(geometry), collection);
  }

  collection.push_back(geometry);
}

static uint32 geometryGetIndexInBranch(Asset* geometry)
{
  AssetPtr parent = geometryGetParent(geometry);
  if(parent == nullptr)
  {
    return (uint32)-1;
  }
  
  const std::vector<AssetPtr>& children = geometryGetChildren(parent);

  for(uint32 i = 0; i < children.size(); i++)
  {
    if(children[i] == geometry)
    {
      return i;
    }
  }

  assert(FALSE);
  return 0;
}

static const char* getCombinationFunctionShaderName(CombinationFunction function)
{
  switch(function)
  {
    case COMBINATION_INTERSECTION: return "intersectDistances";
    case COMBINATION_UNION: return "unionDistances";
    case COMBINATION_SUBTRACTION: return "subtractDistances";
    default: assert(FALSE); return "";
  }

  return "";
}

static void geometryGenerateDistancesCombinationCode(Asset* geometry, ShaderBuild* build)
{
  // Root and non-root leafs/branches have a bit different code because root geometry doesn't have a direct parent
  // (they are part of a scene's hierarchy), so we cannot deduce their position in branch from c++ code, additionally
  // for root objects we implicitly use a union distance.
  if(geometryIsRoot(geometry) == FALSE)
  {
    // Detect its number in parent's branch
    bool8 firstInBranch = geometryGetIndexInBranch(geometry) == 0 ? TRUE : FALSE;
    CombinationFunction parentCombFunction = geometryGetCombinationFunction(geometryGetParent(geometry));
    
    // (TODO: geometry ID should pushed/popped along with distance into stack)
    // This is the first leaf/branch in group - just push distance to the stack
    if(firstInBranch == TRUE)
    {
      shaderBuildAddCode(build, "\tstackPushDistance(ifragCoord, d);");
    }
    // This is not the first leaf/branch in group - pop previous distance from stack, combine, push new distance back
    else
    {
      shaderBuildAddCode(build, "\tfloat32 prevDistance = stackPopDistance(ifragCoord);");

      // Order of combination is important      
      shaderBuildAddCodefln(build, "\tstackPushDistance(ifragCoord, %s(prevDistance, d));",
                            getCombinationFunctionShaderName(parentCombFunction));
    }
  }
  else
  {
    shaderBuildAddCode(build, "\tuint32 stackSize = getStackSize(ifragCoord);");
    shaderBuildAddCode(build, "\tif(stackSize > 0) {");
    shaderBuildAddCode(build, "\t\tfloat32 prevDistance = stackPopDistance(ifragCoord);");
    shaderBuildAddCode(build, "\t\tstackPushDistance(ifragCoord, unionDistances(prevDistance, d));");
    shaderBuildAddCode(build, "\t}");
    shaderBuildAddCode(build, "\telse {");
    shaderBuildAddCode(build, "\t\tstackPushDistance(ifragCoord, d);");
    shaderBuildAddCode(build, "\t}");
  }
}

// NOTE: Leaf geometry uses IDFs, SDF and ODFs (only related to the geometry)
static void geometryGenerateLeafCode(Asset* geometry, ShaderBuild* build)
{
  // collect all parents (in order from the root to the leaf)
  vector<Asset*> parents;
  geometryCollectParents(geometry, parents);

  // register IDFs (in same order), parameters should be replaced via corresponding constant values (special function does that)
  uint32 registeredIDFCount = 0;
  for(Asset* parent: parents)
  {
    const std::vector<AssetPtr>& idfs = geometryGetIDFs(parent);
    for(AssetPtr idf: idfs)
    {
      string functionName = "IDF" + std::to_string(registeredIDFCount);
      string functionBody = scriptFunctionGetGLSLCode(idf);
      shaderBuildAddFunction(build, "float3", functionName.c_str(), "float3 p", functionBody.c_str());
      registeredIDFCount = registeredIDFCount + 1;
    }
  }

  // register SDF
  shaderBuildAddFunction(build,
                         "float32",
                         "SDF",
                         "float3 p",
                         scriptFunctionGetGLSLCode(geometryGetSDF(geometry)).c_str());
  
  // register ODFs (only of the geometry)
  const std::vector<AssetPtr>& odfs = geometryGetODFs(geometry);
  for(uint32 i = 0; i < odfs.size(); i++)
  {
    string functionName = "ODF" + std::to_string(i);
    string functionBody = scriptFunctionGetGLSLCode(odfs[i]);
    shaderBuildAddFunction(build, "float32", functionName.c_str(), "float32 d", functionBody.c_str());
  }

  
  // generate a transform function:
  // ------------------------------
  shaderBuildAddCode(build, "float32 transform(float3 p) {");
  // 1. Transform point p with all IDFs
  for(uint32 i = 0; i < registeredIDFCount; i++)
  {
    shaderBuildAddCodefln(build, "\tp = IDF%d(p);", i);
  }

  // 2. SDF is defined in local coordinates, but the geometry which uses SDF has a transformation:
  // apply transformation to the point, converting from world space to local space. Then calculate
  // SDF itself.
  shaderBuildAddCode(build, "\tfloat32 d = geo.position.z;");
  shaderBuildAddCode(build, "\tfloat4 tp = geo.worldGeoMat * float4(p, 1.0);");
  shaderBuildAddCode(build, "\td = SDF(tp.xyz) * geo.position.w;");
  
  // 3. Transform distance via ODFs
  for(uint32 i = 0; i < odfs.size(); i++)
  {
    shaderBuildAddCodefln(build, "\td = ODF%u(d);", i);
  }
  
  // 4. return distance
  shaderBuildAddCode(build, "\treturn d;");
  shaderBuildAddCode(build, "}");

  // generate a main function:
  // -------------------------
  shaderBuildAddCode(build, "void main() {");
  shaderBuildAddCode(build, "\tint2 ifragCoord = int2(gl_FragCoord.x, gl_FragCoord.y);");

  // 1. Extract point p from ray map
  shaderBuildAddCode(build, "\tfloat4 ray = texelFetch(raysMap, ifragCoord, 0);");
  shaderBuildAddCode(build, "\tfloat3 p = ray.xyz * ray.w + params.camPosition.xyz;");

  // 2. Transform point into distance
  shaderBuildAddCode(build, "\tfloat32 d = transform(p);");

  // 3. Combine distance with last distance on stack (if needed)
  geometryGenerateDistancesCombinationCode(geometry, build);
  
  shaderBuildAddCode(build, "}");
}

// NOTE: Branch geometry uses only ODFs (realted to the branch itself)
static void geometryGenerateBranchCode(Asset* geometry, ShaderBuild* build)
{
  // register ODFs (only of the geometry)
  const std::vector<AssetPtr>& odfs = geometryGetODFs(geometry);
  for(uint32 i = 0; i < odfs.size(); i++)
  {
    string functionName = "ODF" + std::to_string(i);
    string functionBody = scriptFunctionGetGLSLCode(odfs[i]);
    shaderBuildAddFunction(build, "float32", functionName.c_str(), "float32 d", functionBody.c_str());
  }

  // generate a main function:
  shaderBuildAddCode(build, "void main() {");
  shaderBuildAddCode(build, "\tint2 ifragCoord = int2(gl_FragCoord.x, gl_FragCoord.y);");  

  // 1. Extract distance from the stack
  shaderBuildAddCode(build, "\tfloat32 d = stackPopDistance(ifragCoord);");

  // 2. Apply ODFs to the distance  
  for(uint32 i = 0; i < odfs.size(); i++)
  {
    shaderBuildAddCode(build, "d = ODF%u(d);");
  }

  // 3. Combine distance with last distance on stack (if needed)
  geometryGenerateDistancesCombinationCode(geometry, build);

  shaderBuildAddCode(build, "}");
}

static void geometryRebuild(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);

  ShaderBuild* build = nullptr;
  assert(createShaderBuild(&build));

  shaderBuildAddVersion(build, 430, "core");
  shaderBuildAddMacro(build, "GEOMETRY_ID", std::to_string(777).c_str());

  assert(shaderBuildIncludeFile(build, "shaders/common.glsl") == TRUE);  
  assert(shaderBuildIncludeFile(build, "shaders/geometry_common.glsl") == TRUE);

  if(geometryIsLeaf(geometry))
  {
    geometryGenerateLeafCode(geometry, build);
  }
  else
  {
    geometryGenerateBranchCode(geometry, build);
  }

  ShaderPtr fragmentShader = shaderBuildGenerateShader(build, GL_FRAGMENT_SHADER);
  if(fragmentShader == nullptr)
  {
    LOG_ERROR("Cannot generate a shader for geometry!");
    return;
  }

  ShaderPtr vertexShader = shaderManagerGetShader("triangle.vert");
  if(vertexShader == nullptr)
  {
    LOG_ERROR("Cannot load a triangle vertex shader!");
    return;
  }
  
  destroyShaderBuild(build);

  if(geometryData->program != nullptr)
  {
    destroyShaderProgram(geometryData->program);
  }

  ShaderProgram* shaderProgram = nullptr;
  assert(createShaderProgram(&shaderProgram));

  shaderProgramAttachShader(shaderProgram, vertexShader);  
  shaderProgramAttachShader(shaderProgram, fragmentShader);

  assert(linkShaderProgram(shaderProgram));
  
  geometryData->program = shaderProgram;
  geometryData->needRebuild = FALSE;
}

static void geometryMarkNeedRebuild(Asset* geometry, bool8 forwardToChildren)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  geometryData->needRebuild = TRUE;

  if(forwardToChildren == TRUE)
  {
    for(AssetPtr child: geometryData->children)
    {
      geometryMarkNeedRebuild(child, forwardToChildren);
    }
  }
}

static void geometryRecalculateIDs(Asset* geometry, uint32& idCounter)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  for(AssetPtr child: geometryData->children)
  {
    geometryRecalculateIDs(child, idCounter);
  }

  geometryData->ID = idCounter++;
}

static void geometryRecalculateIDs(Asset* geometry)
{
  uint32 idCounter = 0;
  geometryRecalculateIDs(geometry, idCounter);

  assert(idCounter < 65535);
}

// ----------------------------------------------------------------------------
// Geometry common interface
// ----------------------------------------------------------------------------
bool8 createGeometry(const string& name, Asset** outGeometry)
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
  geometryData->needRebuild = TRUE;  
  geometryData->dirty = TRUE;
  geometryData->program = nullptr;
  
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

  if(geometryData->program != nullptr)
  {
    destroyShaderProgram(geometryData->program);
    geometryData->program = nullptr;
  }
  
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
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  ScriptFunctionType type = scriptFunctionGetType(function);
  if(type == SCRIPT_FUNCTION_TYPE_SDF)
  {
    geometryData->sdf = function;
    geometryMarkNeedRebuild(geometry, /** Mark children */ FALSE);
  }
  else if(type == SCRIPT_FUNCTION_TYPE_IDF)
  {
    geometryData->idfs.push_back(function);
    // NOTE: Children also should be marked, because IDFs are integrated into leafs
    geometryMarkNeedRebuild(geometry, TRUE);    
  }
  else if(type == SCRIPT_FUNCTION_TYPE_ODF)
  {
    geometryData->odfs.push_back(function);
    geometryMarkNeedRebuild(geometry, FALSE);        
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
      geometryMarkNeedRebuild(geometry, /** Mark children */ FALSE);      
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
      geometryMarkNeedRebuild(geometry, /** Mark children */ TRUE);            
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
      geometryMarkNeedRebuild(geometry, /** Mark children */ FALSE);                  
      return TRUE;
    }

    return FALSE;
  }

  return FALSE;
}

uint32 geometryGetID(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);

  return geometryData->ID;
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

  geometryMarkNeedRebuild(geometry, /** Mark children */ TRUE); 
}

bool8 geometryHasParent(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  
  return geometryData->parent != nullptr;
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
  return transformedP.xyz();
}

float3 geometryTransformFromParent(Asset* geometry, float3 p)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  
  if(geometryData->dirty == TRUE)
  {
    geometryRecalculateTransforms(geometry);
  }

  float4 transformedP = mul(geometryData->transformToLocalFromParent, float4(p.x, p.y, p.z, 1.0f));
  return transformedP.xyz();
}

float3 geometryTransformToLocal(Asset* geometry, float3 p)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  
  if(geometryData->dirty == TRUE)
  {
    geometryRecalculateTransforms(geometry);
  }

  float4 transformedP = mul(geometryData->transformToLocal, float4(p.x, p.y, p.z, 1.0f));
  return transformedP.xyz();
}

float3 geometryTransformToWorld(Asset* geometry, float3 p)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  
  if(geometryData->dirty == TRUE)
  {
    geometryRecalculateTransforms(geometry);
  }

  float4 transformedP = mul(geometryData->transformToWorld, float4(p.x, p.y, p.z, 1.0f));
  return transformedP.xyz();
}

float4x4 geometryGetWorldGeoMat(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  
  if(geometryData->dirty == TRUE)
  {
    geometryRecalculateTransforms(geometry);
  }

  return geometryData->transformToLocal;
}
float4x4 geometryGetGeoWorldMat(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  
  if(geometryData->dirty == TRUE)
  {
    geometryRecalculateTransforms(geometry);
  }

  return geometryData->transformToWorld;
}

  
float4x4 geometryGetParentGeoMat(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  
  if(geometryData->dirty == TRUE)
  {
    geometryRecalculateTransforms(geometry);
  }

  return geometryData->transformToLocalFromParent;
}
float4x4 geometryGetGeoParentMat(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  
  if(geometryData->dirty == TRUE)
  {
    geometryRecalculateTransforms(geometry);
  }

  return geometryData->transformToParentFromLocal;
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

bool8 geometryNeedRebuild(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  return geometryData->needRebuild;
}

ShaderProgram* geometryGetProgram(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  if(geometryData->needRebuild == TRUE)
  {
    geometryRebuild(geometry);
  }

  return geometryData->program;
}

// ----------------------------------------------------------------------------
// Branch geometry-related interface
// ----------------------------------------------------------------------------

void geometryAddChild(AssetPtr geometry, AssetPtr child)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);  
  
  geometryData->children.push_back(child);
  geometrySetParent(child, geometry);

  geometryMarkNeedRebuild(geometry, /** Mark children */ TRUE);

  geometryRecalculateIDs(geometry);
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

  geometryMarkNeedRebuild(geometry, /** Mark children */ TRUE);    

  geometryRecalculateIDs(geometry);
  
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

  geometryMarkNeedRebuild(geometry, /** Mark children */ TRUE);    
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
