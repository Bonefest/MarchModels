#include <string>
#include <vector>
using std::string;
using std::vector;

#include "logging.h"
#include "maths/aabb.h"
#include "shader_build.h"
#include "shader_manager.h"
#include "memory_manager.h"
#include "renderer/passes/geometry_native_aabb_calculation_pass.h"

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

  AABB nativeAABB;
  AABB dynamicAABB;
  AABB finalAABB;
  
  ShaderProgram* drawProgram;
  ShaderProgram* aabbProgram;

  bool8 bounded;
  bool8 aabbAutomaticallyCalculated;
  bool8 needAABBRecalculation;
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
    case COMBINATION_INTERSECTION: return "intersectGeometries";
    case COMBINATION_UNION: return "unionGeometries";
    case COMBINATION_SUBTRACTION: return "subtractGeometries";
    default: assert(FALSE); return "";
  }

  return "";
}

static void geometryGenerateDistancesCombinationCode(Asset* geometry, ShaderBuild* build)
{
  // Detect its number in parent's branch
  bool8 firstInBranch = geometryGetIndexInBranch(geometry) == 0 ? TRUE : FALSE;
  CombinationFunction parentCombFunction = geometryGetCombinationFunction(geometryGetParent(geometry));
  
  // This is the first leaf/branch in group - just push geometry to the stack
  if(firstInBranch == TRUE)
  {
    shaderBuildAddCode(build, "\tstackPushGeometry(ifragCoord, geometry);");
  }
  // This is not the first leaf/branch in group - pop previous geometry from stack, combine, push new geometry back
  else
  {
    shaderBuildAddCode(build, "\tGeometryData prevGeometry = stackPopGeometry(ifragCoord);");

    // Order of combination is important      
    shaderBuildAddCodefln(build, "\tstackPushGeometry(ifragCoord, %s(prevGeometry, geometry));",
                          getCombinationFunctionShaderName(parentCombFunction));
  }


}

static void geometryGenerateTransformCode(Asset* geometry, ShaderBuild* build, bool8 applyGeometryTransform)
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
  AssetPtr sdf = geometryGetSDF(geometry);
  if(sdf != nullptr)
  {
    shaderBuildAddFunction(build,
                           "float32",
                           "SDF",
                           "float3 p",
                           scriptFunctionGetGLSLCode(geometryGetSDF(geometry)).c_str());
  }
  else
  {
    shaderBuildAddFunction(build,
                           "float32",
                           "SDF",
                           "float3 p",
                           "return 1.0;");
  }
  
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

  // Apply transform only if requested (sometimes we may need to render relatively the origin,
  // e.g during AABB calculation)
  if(applyGeometryTransform == TRUE)
  {
    shaderBuildAddCode(build, "\tfloat4 tp = geo.worldGeoMat * float4(p, 1.0);");
  }
  else
  {
    shaderBuildAddCode(build, "\tfloat4 tp = float4(p, 1.0);");
  }
  
  shaderBuildAddCode(build, "\tfloat32 d = SDF(tp.xyz);");
  
  // 3. Transform distance via ODFs
  for(uint32 i = 0; i < odfs.size(); i++)
  {
    shaderBuildAddCodefln(build, "\td = ODF%u(d);", i);
  }
  
  // 4. return distance
  shaderBuildAddCode(build, "\treturn d;");
  shaderBuildAddCode(build, "}");

}

// NOTE: Leaf geometry uses IDFs, SDF and ODFs (only related to the geometry)
static void geometryGenerateLeafCode(Asset* geometry, ShaderBuild* build)
{
  geometryGenerateTransformCode(geometry, build, /** Use transformation of geometry */ TRUE);
  
  // generate a main function:
  // -------------------------
  shaderBuildAddCode(build, "void main() {");
  shaderBuildAddCode(build, "\tint2 ifragCoord = int2(gl_FragCoord.x, gl_FragCoord.y);");
  shaderBuildAddCode(build, "\tif(!all(lessThan(ifragCoord, params.gapResolution))) discard;");
  
  // 1. Extract point p from ray map
  shaderBuildAddCode(build, "\tfloat4 ray = texelFetch(raysMap, ifragCoord, 0);");
  shaderBuildAddCode(build, "\tfloat3 p = ray.xyz * ray.w + params.camPosition.xyz;");

  // 2. Transform point into geometry data (distance, id)
  shaderBuildAddCode(build, "\tGeometryData geometry = createGeometryData(transform(p), geometryID);");

  // 3. Combine distance with last distance on stack (if needed)
  geometryGenerateDistancesCombinationCode(geometry, build);

  shaderBuildAddCode(build, "\toutColor = 0.0f.xxxx;");
  
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
  shaderBuildAddCode(build, "\tif(!all(lessThan(ifragCoord, params.gapResolution))) discard;");
  
  // 1. Extract distance from the stack
  shaderBuildAddCode(build, "\tGeometryData geometry = stackPopGeometry(ifragCoord);");

  // 2. Apply ODFs to the distance  
  for(uint32 i = 0; i < odfs.size(); i++)
  {
    shaderBuildAddCode(build, "\tgeometry.distance = ODF%u(geometry.distance);");
  }

  // 3. Combine distance with last distance on stack (if needed)
  geometryGenerateDistancesCombinationCode(geometry, build);

  shaderBuildAddCode(build, "\toutColor = 0.0f.xxxx;");

  shaderBuildAddCode(build, "}");
}

static bool8 geometryRebuildDrawProgram(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);

  ShaderBuild* build = nullptr;
  assert(createShaderBuild(&build));

  shaderBuildAddVersion(build, 430, "core");

  assert(shaderBuildIncludeFile(build, "shaders/common.glsl") == TRUE);  
  assert(shaderBuildIncludeFile(build, "shaders/geometry_common.glsl") == TRUE);

  shaderBuildAddCode(build, "uniform uint32 geometryID;");

  shaderBuildAddCode(build, "layout(location = 0) out float4 outColor;");
  
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
    return FALSE;
  }

  ShaderPtr vertexShader = shaderManagerGetShader("triangle.vert");
  if(vertexShader == nullptr)
  {
    LOG_ERROR("Cannot load a triangle vertex shader!");
    return FALSE;
  }
  
  destroyShaderBuild(build);

  ShaderProgram* shaderProgram = nullptr;
  assert(createShaderProgram(&shaderProgram));

  shaderProgramAttachShader(shaderProgram, vertexShader);  
  shaderProgramAttachShader(shaderProgram, fragmentShader);

  if(linkShaderProgram(shaderProgram) == FALSE)
  {
    return FALSE;
  }

  if(geometryData->drawProgram != nullptr)
  {
    destroyShaderProgram(geometryData->drawProgram);
  }
  
  geometryData->drawProgram = shaderProgram;

  return TRUE;
}

static bool8 geometryRebuildAABBCalculationProgram(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);

  ShaderBuild* build = nullptr;
  assert(createShaderBuild(&build));

  shaderBuildAddVersion(build, 430, "core");

  assert(shaderBuildIncludeFile(build, "shaders/common.glsl") == TRUE);  

  geometryGenerateTransformCode(geometry, build, /** Use transformation of geometry */ FALSE);  

  assert(shaderBuildIncludeFile(build, "shaders/calculate_aabb.glsl") == TRUE);    
  
  ShaderPtr fragmentShader = shaderBuildGenerateShader(build, GL_FRAGMENT_SHADER);
  if(fragmentShader == nullptr)
  {
    LOG_ERROR("Cannot generate a shader for geometry!");
    return FALSE;
  }

  ShaderPtr vertexShader = shaderManagerGetShader("triangle.vert");
  if(vertexShader == nullptr)
  {
    LOG_ERROR("Cannot load a triangle vertex shader!");
    return FALSE;
  }
  
  destroyShaderBuild(build);

  ShaderProgram* shaderProgram = nullptr;
  assert(createShaderProgram(&shaderProgram));

  shaderProgramAttachShader(shaderProgram, vertexShader);  
  shaderProgramAttachShader(shaderProgram, fragmentShader);

  if(linkShaderProgram(shaderProgram) == FALSE)
  {
    return FALSE;
  }

  if(geometryData->aabbProgram != nullptr)
  {
    destroyShaderProgram(geometryData->aabbProgram);
  }
  
  geometryData->aabbProgram = shaderProgram;

  return TRUE;
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
  geometryData->ID = idCounter++;
  
  for(AssetPtr child: geometryData->children)
  {
    geometryRecalculateIDs(child, idCounter);
  }
}

static void geometryRecalculateIDs(Asset* geometry)
{
  uint32 idCounter = 0;
  geometryRecalculateIDs(geometryGetRoot(geometry), idCounter);

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
  geometryData->orientation = quat(0.0f, 0.0f, 0.0f, 1.0f);
  geometryData->bounded = TRUE;  
  geometryData->aabbAutomaticallyCalculated = TRUE;  
  geometryData->needAABBRecalculation = TRUE;    
  geometryData->needRebuild = TRUE;
  geometryData->dirty = TRUE;
  geometryData->drawProgram = nullptr;
  geometryData->aabbProgram = nullptr;  
  
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

  if(geometryData->drawProgram != nullptr)
  {
    destroyShaderProgram(geometryData->drawProgram);
    geometryData->drawProgram = nullptr;
  }

  if(geometryData->aabbProgram != nullptr)
  {
    destroyShaderProgram(geometryData->aabbProgram);
    geometryData->aabbProgram = nullptr;
  }
  
  engineFreeObject(geometryData, MEMORY_TYPE_GENERAL);
}

void geometryUpdate(Asset* geometry, float64 delta)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);

  if(geometryData->needRebuild == TRUE)
  {
    if(geometryRebuildDrawProgram(geometry) == TRUE)
    {
      if(geometryIsLeaf(geometry))
      {
        if(geometryRebuildAABBCalculationProgram(geometry) == FALSE)
        {
          LOG_ERROR("Geometry rebuild of AABB calculation program has failed!");

          if(geometryData->aabbProgram != nullptr)
          {
            destroyShaderProgram(geometryData->aabbProgram);
            geometryData->aabbProgram = nullptr;
          }          
        }
      }

      geometryData->needRebuild = FALSE;
    }
    else
    {
      LOG_ERROR("Geometry rebuild of draw program has failed!");

      if(geometryData->drawProgram != nullptr)
      {
        destroyShaderProgram(geometryData->drawProgram);
        geometryData->drawProgram = nullptr;
      }
    }
  }

  if(geometryData->needAABBRecalculation == TRUE && geometryData->aabbAutomaticallyCalculated == TRUE)
  {
    geometryData->nativeAABB = AABBCalculationPassCalculateAABB(geometry);
    geometryData->needAABBRecalculation = FALSE;
  }

  for(auto child: geometryData->children)
  {
    geometryUpdate(child, delta);
  }
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
    geometryMarkAsNeedAABBRecalculation(geometry);
  }
  else if(type == SCRIPT_FUNCTION_TYPE_IDF)
  {
    geometryData->idfs.push_back(function);
    // NOTE: Children also should be marked, because IDFs are integrated into leafs
    geometryMarkNeedRebuild(geometry, /** Mark children */ TRUE);
    geometryMarkAsNeedAABBRecalculation(geometry);    
  }
  else if(type == SCRIPT_FUNCTION_TYPE_ODF)
  {
    geometryData->odfs.push_back(function);
    geometryMarkNeedRebuild(geometry, /** Mark children */ FALSE);
    geometryMarkAsNeedAABBRecalculation(geometry);    
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
      geometryMarkAsNeedAABBRecalculation(geometry);      
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
      geometryMarkAsNeedAABBRecalculation(geometry);      
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
      geometryMarkAsNeedAABBRecalculation(geometry);      
      return TRUE;
    }

    return FALSE;
  }

  return FALSE;
}

void geometryNotifyFunctionHasChanged(Asset* geometry, Asset* function)
{
  ScriptFunctionType type = scriptFunctionGetType(function);
  geometryMarkNeedRebuild(geometry, /** Mark children */ type == SCRIPT_FUNCTION_TYPE_IDF ? TRUE : FALSE);
  geometryMarkAsNeedAABBRecalculation(geometry);  
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

bool8 geometryHasFunction(Asset* geometry, Asset* function)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  
  if(function == geometryData->sdf)
  {
    return TRUE;
  }

  for(AssetPtr sf: geometryData->idfs)
  {
    if(sf == function)
    {
      return TRUE;
    }
  }
  
  for(AssetPtr sf: geometryData->odfs)
  {
    if(sf == function)
    {
      return TRUE;
    }
  }

  return FALSE;
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

void geometrySetBounded(Asset* geometry, bool8 bounded)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);

  geometryData->bounded = bounded;
}

bool8 geometryIsBounded(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  return geometryData->bounded;
}

void geometrySetAABBAutomaticallyCalculated(Asset* geometry, bool8 automatically)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  geometryData->aabbAutomaticallyCalculated = automatically;
  if(automatically == TRUE)
  {
    geometryMarkAsNeedAABBRecalculation(geometry);
  }
}

bool8 geometryAABBIsAutomaticallyCalculated(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  return geometryData->aabbAutomaticallyCalculated;
}

void geometrySetNativeAABB(Asset* geometry, const AABB& nativeAABB)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  geometryData->nativeAABB = nativeAABB;
}

const AABB& geometryGetNativeAABB(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  return geometryData->nativeAABB;
}

const AABB& geometryGetDynamicAABB(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  return geometryData->dynamicAABB;
}

const AABB& geometryGetFinalAABB(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  return geometryData->finalAABB;
}

void geometryMarkAsNeedAABBRecalculation(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  geometryData->needAABBRecalculation = TRUE;
}

bool8 geometryNeedAABBRecalculation(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  return geometryData->needAABBRecalculation;
}

bool8 geometryNeedRebuild(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  return geometryData->needRebuild;
}

ShaderProgram* geometryGetDrawProgram(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  return geometryData->drawProgram;
}

ShaderProgram* geometryGetAABBProgram(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  return geometryData->aabbProgram;
}

bool8 geometryTraversePostorder(Asset* geometry, fpTraverseFunction traverseFunction, void* userData)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);

  for(Asset* child: geometryData->children)
  {
    // If stop traversing is requested - terminate function
    if(geometryTraversePostorder(child, traverseFunction, userData) == TRUE)
    {
      return TRUE;
    }
  }

  return traverseFunction(geometry, userData);
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
  geometryMarkAsNeedAABBRecalculation(geometry);  

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
  if(geometry == nullptr)
  {
    return COMBINATION_UNION;
  }
  
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
