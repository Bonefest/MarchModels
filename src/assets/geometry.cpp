#include <cstring>

#include <string>
#include <vector>

#include "logging.h"
#include "shader_build.h"
#include "shader_manager.h"
#include "memory_manager.h"
#include "assets_manager.h"
#include "assets_factory.h"
#include "maths/json_serializers.h"
#include "renderer/passes/geometry_native_aabb_calculation_pass.h"

#include "geometry.h"

using std::set;
using std::string;
using std::vector;
using nlohmann::json;

DECLARE_CVAR(engine_AABBCalculation_IterationsCount, 12u);
DECLARE_CVAR(engine_AABBCalculation_RaysPerIteration, 1024u);
DECLARE_CVAR(engine_AABBCalculation_LocalWorkGroupSize, 32u);

struct Geometry
{
  // Common data
  uint32 ID;
  uint32 totalChildrenCount = 0;
  
  std::vector<AssetPtr> idfs;
  std::vector<AssetPtr> odfs;
  AssetPtr pcf;

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
  ShaderProgram* shadowProgram;
  ShaderProgram* aabbProgram;

  bool8 bounded;
  bool8 aabbAutomaticallyCalculated;
  bool8 needAABBRecalculation;
  bool8 needRebuild;
  bool8 dirty;
  bool8 selected;
  bool8 enabled;

  // Root geometry data
  set<AssetPtr> allChildren;
  
  // Branch geometry data
  std::vector<AssetPtr> children;  

  // Leaf geometry data
  AssetPtr sdf;
  Material* material;
};

static void geometryDestroy(Asset* geometry);
static bool8 geometrySerialize(AssetPtr geometry, json& jsonData);
static bool8 geometryDeserialize(AssetPtr geometry, json& jsonData);
static uint32 geometryGetSize(Asset* geometry) { /** TODO */ }

// ----------------------------------------------------------------------------
// Helper functions
// ----------------------------------------------------------------------------

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

    // NOTE: Recalculate dynamic AABB, if it's a leaf, based on the new transformations
    if(geometryIsLeaf(geometry) == TRUE)
    {
      geometryData->dynamicAABB = geometryData->nativeAABB.genTransformed(geometryData->transformToWorld);
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

  uint32 disabledSiblingsCount = 0;
  for(uint32 i = 0; i < children.size(); i++)
  {
    if(children[i] == geometry)
    {
      return i - disabledSiblingsCount;
    }
    else if(geometryIsEnabled(children[i]) == FALSE)
    {
      disabledSiblingsCount++;
    }
  }

  assert(FALSE);
  return 0;
}

static void geometryGenerateDistancesCombinationCode(Asset* geometry, ShaderBuild* build)
{
  // NOTE: We can eliminate if(indexInBrach == 0) condition, i.e precalculate here and
  // hence make code a bit faster. It works OK but is less flexible -- in case we want
  // to disable first geometry in group, we would need to recalculate all shaders
  // (because 2nd geometry would become first and so on).
  
  // This is the first leaf/branch in group - just push geometry to the stack  
  shaderBuildAddCode(build, "\tif(indexInBranch == 0)");
  shaderBuildAddCode(build, "\t{");
    shaderBuildAddCode(build, "\t\tstackPushGeometry(ifragCoord, geometry);");
  shaderBuildAddCode(build, "\t}");

  // This is not the first leaf/branch in group  
  shaderBuildAddCode(build, "\telse");
  shaderBuildAddCode(build, "\t{");
    // If at least one previous sibling weren't culled - pop previous geometry from stack,
    // combine, push new geometry back
    shaderBuildAddCode(build, "\t\tif(prevCulledSiblingsCount < indexInBranch)");
    shaderBuildAddCode(build, "\t\t{");
      shaderBuildAddCode(build, "\t\t\tGeometryData prevGeometry = stackPopGeometry(ifragCoord);");
      // Order of combination is important
      shaderBuildAddCode(build, "\t\t\tfloat2 pcfResult = PCF(prevGeometry.distance, geometry.distance);");
      shaderBuildAddCode(build, "\t\t\tstackPushGeometry(ifragCoord, createGeometryData(pcfResult.x, int32(mix(prevGeometry.id, geometry.id, int32(pcfResult.y)))));");
    shaderBuildAddCode(build, "\t\t}");    

    // Else - simply push geometry to the stack
    shaderBuildAddCode(build, "\t\telse");
    shaderBuildAddCode(build, "\t\t{");
      shaderBuildAddCode(build, "\t\t\tstackPushGeometry(ifragCoord, geometry);");
    shaderBuildAddCode(build, "\t\t}");
  shaderBuildAddCode(build, "\t}");

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
                           scriptFunctionGetGLSLCode(sdf).c_str());
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

  // register PCF
  AssetPtr pcf = geometryGetPCF(geometryGetParent(geometry));
  if(pcf != AssetPtr(nullptr))
  {
    shaderBuildAddFunction(build,
                           "float2",
                           "PCF",
                           "float32 d1, float32 d2",
                           scriptFunctionGetGLSLCode(pcf).c_str());
  }
  else
  {
    shaderBuildAddFunction(build,
                           "float2",
                           "PCF",
                           "float32 d1, float32 d2",
                           "return (d1 < d2 ? float2(d1, 0.0) : float2(d2, 1.0));");

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

  
  // generate a main function:
  // -------------------------
  shaderBuildAddCode(build, "void main() {");
  shaderBuildAddCode(build, "\tint2 ifragCoord = int2(gl_FragCoord.x, gl_FragCoord.y);");
  
  // 1. Extract point p from ray map
  shaderBuildAddCode(build, "\tfloat4 ray = texelFetch(raysMap, ifragCoord, 0);");
  shaderBuildAddCode(build, "\t#if NORMAL_PATH");
  shaderBuildAddCode(build, "\t\tfloat3 p = ray.xyz * ray.w + params.camPosition.xyz;");
  shaderBuildAddCode(build, "\t#elif SHADOW_PATH");
  shaderBuildAddCode(build, "\t\tfloat2 uv = fragCoordToUV(gl_FragCoord.xy);");
  shaderBuildAddCode(build, "\t\tfloat3 ro = getWorldPos(uv, ifragCoord, depthMap);");
  shaderBuildAddCode(build, "\t\tfloat3 p = ro + ray.xyz * ray.w;");
  shaderBuildAddCode(build, "\t#endif");

  // 2. Transform point into geometry data (distance, id)
  shaderBuildAddCode(build, "\tGeometryData geometry = createGeometryData(transform(p), geometryID);");

  // 3. Combine distance with last distance on stack (if needed)
  geometryGenerateDistancesCombinationCode(geometry, build);

  shaderBuildAddCode(build, "\toutColor = float4(0.0f, 0.0f, 0.0f, 0.0f);");
  
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
  shaderBuildAddCode(build, "layout(early_fragment_tests) in;");
  shaderBuildAddCode(build, "%s");
  
  assert(shaderBuildIncludeFile(build, "shaders/common.glsl") == TRUE);  
  assert(shaderBuildIncludeFile(build, "shaders/geometry_common.glsl") == TRUE);

  shaderBuildAddCode(build, "uniform uint32 geometryID;");
  shaderBuildAddCode(build, "uniform uint32 indexInBranch;");
  shaderBuildAddCode(build, "uniform uint32 prevCulledSiblingsCount;");  

  shaderBuildAddCode(build, "layout(location = 0) out float4 outColor;");

  geometryGenerateTransformCode(geometry, build, /** Use transformation of geometry */ TRUE);
  
  if(geometryIsLeaf(geometry))
  {
    geometryGenerateLeafCode(geometry, build);
  }
  else
  {
    geometryGenerateBranchCode(geometry, build);
  }

  ShaderPtr vertexShader = shaderManagerGetShader("triangle.vert");
  if(vertexShader == nullptr)
  {
    LOG_ERROR("Cannot load a triangle vertex shader!");
    return FALSE;
  }
  
  string unformattedCode = shaderBuildGetCode(build);
  destroyShaderBuild(build);
  
  // Normal path program generation
  char normalPathCode[32 * KIBIBYTE];
  sprintf(normalPathCode, unformattedCode.c_str(), "#define NORMAL_PATH 1");
  
  Shader* normalPathShader = nullptr;
  assert(createShaderFromMemory(GL_FRAGMENT_SHADER, normalPathCode, &normalPathShader));
  if(compileShader(normalPathShader) == FALSE)
  {
    LOG_ERROR("Cannot generate a normal path shader for geometry!");
    return FALSE;
  }

  ShaderProgram* normalPathShaderProgram = nullptr;
  assert(createShaderProgram(&normalPathShaderProgram));

  shaderProgramAttachShader(normalPathShaderProgram, vertexShader);  
  shaderProgramAttachShader(normalPathShaderProgram, ShaderPtr(normalPathShader));

  if(linkShaderProgram(normalPathShaderProgram) == FALSE)
  {
    return FALSE;
  }

  if(geometryData->drawProgram != nullptr)
  {
    destroyShaderProgram(geometryData->drawProgram);
  }
  
  geometryData->drawProgram = normalPathShaderProgram;

  // Shadow path program generation
  char shadowPathCode[32 * KIBIBYTE];
  sprintf(shadowPathCode, unformattedCode.c_str(), "#define SHADOW_PATH 1");
  
  Shader* shadowPathShader = nullptr;
  assert(createShaderFromMemory(GL_FRAGMENT_SHADER, shadowPathCode, &shadowPathShader));
  if(compileShader(shadowPathShader) == FALSE)
  {
    LOG_ERROR("Cannot generate a shadow path shader for geometry!");
    return FALSE;
  }

  ShaderProgram* shadowPathShaderProgram = nullptr;
  assert(createShaderProgram(&shadowPathShaderProgram));

  shaderProgramAttachShader(shadowPathShaderProgram, vertexShader);  
  shaderProgramAttachShader(shadowPathShaderProgram, ShaderPtr(shadowPathShader));

  if(linkShaderProgram(shadowPathShaderProgram) == FALSE)
  {
    return FALSE;
  }

  if(geometryData->shadowProgram != nullptr)
  {
    destroyShaderProgram(geometryData->shadowProgram);
  }
  
  geometryData->shadowProgram = shadowPathShaderProgram;
  
  return TRUE;
}

static bool8 geometryRebuildAABBCalculationProgram(Asset* geometry)
{
  const static uint32& localWorkgroupSize = CVarSystemReadUint("engine_AABBCalculation_LocalWorkGroupSize");

  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);

  ShaderBuild* build = nullptr;
  assert(createShaderBuild(&build));

  shaderBuildAddVersion(build, 430, "core");
  shaderBuildAddCodefln(build, "layout(local_size_x = %u, local_size_y = %u) in;",
                        localWorkgroupSize,
                        localWorkgroupSize);
  
  assert(shaderBuildIncludeFile(build, "shaders/common.glsl") == TRUE);  

  geometryGenerateTransformCode(geometry, build, /** Use transformation of geometry */ FALSE);  

  assert(shaderBuildIncludeFile(build, "shaders/calculate_aabb.glsl") == TRUE);    
  
  ShaderPtr computeShader = shaderBuildGenerateShader(build, GL_COMPUTE_SHADER);
  if(computeShader == nullptr)
  {
    LOG_ERROR("Cannot generate a shader for aabb calculation!");
    return FALSE;
  }

  destroyShaderBuild(build);

  ShaderProgram* shaderProgram = nullptr;
  assert(createShaderProgram(&shaderProgram));
  shaderProgramAttachShader(shaderProgram, computeShader);

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

  geometryData->totalChildrenCount = idCounter - geometryData->ID - 1;
}

static void geometryRecalculateIDs(Asset* geometry)
{
  uint32 idCounter = 0;
  geometryRecalculateIDs(geometryGetRoot(geometry), idCounter);

  assert(idCounter < 65535);
}

static void geometryChildWasAdded(Asset* parent, AssetPtr child)
{
  Geometry* childData = (Geometry*)assetGetInternalData(child);
  for(AssetPtr childOfChild: childData->children)
  {
    geometryChildWasAdded(child, childOfChild);
  }
  
  Asset* root = geometryGetRoot(parent);
  Geometry* rootData = (Geometry*)assetGetInternalData(root);
  auto insertInfo = rootData->allChildren.insert(child);
  assert(insertInfo.second);
}

static void geometryChildWasRemoved(Asset* parent, AssetPtr child)
{
  Geometry* childData = (Geometry*)assetGetInternalData(child);
  for(AssetPtr childOfChild: childData->children)
  {
    geometryChildWasRemoved(child, childOfChild);
  }
  childData->parent = AssetPtr(nullptr);
  childData->children.clear();

  
  Asset* root = geometryGetRoot(parent);
  Geometry* rootData = (Geometry*)assetGetInternalData(root);
  assert(rootData->allChildren.erase(child) == 1);
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
  geometryData->scale = 1.0f;
  geometryData->position = float3(0.0f, 0.0f, 0.0f);
  geometryData->orientation = quat(0.0f, 0.0f, 0.0f, 1.0f);
  geometryData->bounded = TRUE;  
  geometryData->aabbAutomaticallyCalculated = TRUE;  
  geometryData->needAABBRecalculation = TRUE;    
  geometryData->needRebuild = TRUE;
  geometryData->dirty = TRUE;
  geometryData->enabled = TRUE;
  geometryData->nativeAABB = AABB::createUnbounded();
  geometryData->dynamicAABB = AABB::createUnbounded();
  geometryData->finalAABB = AABB::createUnbounded();  
  geometryData->drawProgram = nullptr;
  geometryData->shadowProgram = nullptr;
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

  if(geometryData->shadowProgram != nullptr)
  {
    destroyShaderProgram(geometryData->shadowProgram);
    geometryData->shadowProgram = nullptr;
  }
  
  if(geometryData->aabbProgram != nullptr)
  {
    destroyShaderProgram(geometryData->aabbProgram);
    geometryData->aabbProgram = nullptr;
  }
  
  engineFreeObject(geometryData, MEMORY_TYPE_GENERAL);
}

static json serializeScriptFunction(AssetPtr scriptFunction)
{
  json result;
  bool8 isPrototype = assetsManagerHasAsset(scriptFunction);
  result["is_prototype"] = isPrototype;
  result["name"] = assetGetName(scriptFunction);
  
  if(isPrototype == FALSE)
  {
    assetSerialize(scriptFunction, result);
  }

  return result;
}

static AssetPtr deserializeScriptFunction(json& jsonData)
{
  bool8 isPrototype = jsonData.value("is_prototype", FALSE);
  AssetPtr result = AssetPtr(nullptr);
  
  if(isPrototype == TRUE)
  {
    result = assetsManagerFindAsset(jsonData["name"]);
  }
  else
  {
    result = createAssetFromJson(jsonData);
  }

  assert(result != nullptr && "Geometry contains invalid forked script function!");

  return result;
}

bool8 geometrySerialize(AssetPtr geometry, json& jsonData)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);

  for(uint32 i = 0; i < geometryData->idfs.size(); i++)
  {
    jsonData["idfs"][i] = serializeScriptFunction(geometryData->idfs[i]);
  }

  for(uint32 i = 0; i < geometryData->odfs.size(); i++)
  {
    jsonData["odfs"][i] = serializeScriptFunction(geometryData->odfs[i]);
  }

  jsonData["pcf"] = serializeScriptFunction(geometryData->pcf);
  if(geometryData->sdf != nullptr)
  {
    jsonData["sdf"] = serializeScriptFunction(geometryData->sdf);
  }

  jsonData["scale"] = geometryData->scale;
  jsonData["origin"] = vecToJson(geometryData->origin);
  jsonData["position"] = vecToJson(geometryData->position);
  jsonData["orientation"] = vecToJson(geometryData->orientation);

  jsonData["native_aabb"]["min"] = vecToJson(geometryData->nativeAABB.min);
  jsonData["native_aabb"]["max"] = vecToJson(geometryData->nativeAABB.max);

  jsonData["dynamic_aabb"]["min"] = vecToJson(geometryData->dynamicAABB.min);
  jsonData["dynamic_aabb"]["max"] = vecToJson(geometryData->dynamicAABB.max);

  jsonData["bounded"] = geometryData->bounded;
  jsonData["aabb_automatically_calculated"] = geometryData->aabbAutomaticallyCalculated;

  for(uint32 i = 0; i < geometryData->children.size(); i++)
  {
    if(assetSerialize(geometryData->children[i], jsonData["children"][i]) == FALSE)
    {
      return FALSE;
    }
  }

  return TRUE;
}

bool8 geometryDeserialize(AssetPtr geometry, json& jsonData)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);

  if(jsonData.contains("idfs"))
  {
    for(auto& idf: jsonData["idfs"])
    {
      geometryData->idfs.push_back(deserializeScriptFunction(idf));
    }
  }

  if(jsonData.contains("odfs"))
  {
    for(auto& odf: jsonData["odfs"])
    {
      geometryData->odfs.push_back(deserializeScriptFunction(odf));
    }
  }

  geometryData->pcf = deserializeScriptFunction(jsonData.at("pcf"));
  if(jsonData.contains("sdf"))
  {
    geometryData->sdf = deserializeScriptFunction(jsonData.at("sdf"));
  }

  geometryData->scale = geometryData->scale;
  geometryData->origin = jsonToVec<float32, 3>(jsonData["origin"]);
  geometryData->position = jsonToVec<float32, 3>(jsonData["position"]);
  geometryData->orientation = jsonToVec<float32, 4>(jsonData["orientation"]);
  
  geometryData->nativeAABB.min = jsonToVec<float32, 3>(jsonData["native_aabb"]["min"]);
  geometryData->nativeAABB.max = jsonToVec<float32, 3>(jsonData["native_aabb"]["max"]);
  geometryData->dynamicAABB.min = jsonToVec<float32, 3>(jsonData["dynamic_aabb"]["min"]);
  geometryData->dynamicAABB.max = jsonToVec<float32, 3>(jsonData["dynamic_aabb"]["max"]);  

  geometryData->bounded = jsonData.value("bounded", FALSE);
  geometryData->aabbAutomaticallyCalculated = jsonData.value("aabb_automatically_calculated", FALSE);

  // NOTE: Remove previous children
  for(AssetPtr child: geometryData->children)
  {
    geometryChildWasRemoved(geometry, child);
  }
  geometryData->children.clear();

  // NOTE: Generate new children
  if(jsonData.contains("children"))
  {
    for(auto& childJson: jsonData["children"])
    {
      AssetPtr child = createAssetFromJson(childJson);
      geometryAddChild(geometry, child);
    }
  }

  return TRUE;
}

static void geometryUpdateChild(Asset* geometry, float64 delta)
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

      if(geometryData->shadowProgram != nullptr)
      {
        destroyShaderProgram(geometryData->shadowProgram);
        geometryData->shadowProgram = nullptr;
      }
    }
  }

  if(geometryData->needAABBRecalculation == TRUE && geometryData->aabbAutomaticallyCalculated == TRUE)
  {
    geometryData->nativeAABB = AABBCalculationPassCalculateAABB(geometry);
    if(geometryData->nativeAABB.getWidth() > 40.0 ||
       geometryData->nativeAABB.getHeight() > 40.0 ||
       geometryData->nativeAABB.getDepth() > 40.0)
    {
      geometryData->bounded = FALSE;
    }

    // NOTE: Mark as dirty, so that a new dynamic AABB will be calculated when requested
    geometryData->dirty = TRUE;
    geometryData->needAABBRecalculation = FALSE;
  }

  for(auto child: geometryData->children)
  {
    geometryUpdateChild(child, delta);
  }
}

static void geometryCalculateBranchesFinalAABB(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  
  if(geometryIsLeaf(geometry) == TRUE)
  {
    geometryData->finalAABB = geometryData->dynamicAABB;
    return;
  }

  for(auto child: geometryData->children)
  {
    geometryCalculateBranchesFinalAABB(child);
  }

  AABB finalAABB = geometryGetFinalAABB(geometryData->children[0]);

  PCFNativeType combinationType = geometryGetPCFNativeType(geometry);
  // NOTE: For subtraction AABB of the first child is used
  if(combinationType != PCF_NATIVE_TYPE_SUBTRACTION)
  {
    for(uint32 i = 1; i < geometryData->children.size(); i++)
    {
      AABB childAABB = geometryGetFinalAABB(geometryData->children[i]);
      
      if(combinationType == PCF_NATIVE_TYPE_UNION)
      {
        finalAABB = AABBUnion(finalAABB, childAABB);
      }
      else if(combinationType == PCF_NATIVE_TYPE_INTERSECTION)
      {
        finalAABB = AABBIntersection(finalAABB, childAABB);
      }
      else
      {
        assert(false);
      }
    }
  }

  geometryData->finalAABB = finalAABB;
}

static void geometryCalculateLeafsFinalAABB(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  
  if(geometryIsLeaf(geometry) == TRUE)
  {
    // NOTE: If this geometry is subtracted from other geometry in the branch -
    // calculate the intersection of AABB of those two first (because only that region of
    // AABB matters)
    if(geometryGetPCFNativeType(geometryData->parent) == PCF_NATIVE_TYPE_SUBTRACTION &&
       geometryGetIndexInBranch(geometry) > 0)
    {
      // NOTE: We know that in case of subtraction, parent has an AABB of the first child, the one
      // from which we subract the rest of children
      geometryData->finalAABB = AABBIntersection(geometryGetFinalAABB(geometryData->parent),
                                                 geometryData->finalAABB);
    }

    // NOTE: Traverse from the leaf to the parents, compare AABB of the parent to the leaf's AABB, pick
    // the smallest (it's possible, for example, in case where two subsequent geometry levels have
    // intersection combination function --> parent's AABB will be always smaller)
    AssetPtr parent = geometryData->parent;
    while(parent != nullptr)
    {
      Geometry* parentsGeometryData = (Geometry*)assetGetInternalData(parent);

      if(geometryData->finalAABB.getVolume() > parentsGeometryData->finalAABB.getVolume())
      {
        geometryData->finalAABB = parentsGeometryData->finalAABB;
      }
      
      parent = parentsGeometryData->parent;
    }
  }
  else
  {
    for(auto child: geometryData->children)
    {
      geometryCalculateLeafsFinalAABB(child);
    }
  }
}

void geometryUpdate(Asset* geometry, float64 delta)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  
  for(auto child: geometryData->children)
  {
    geometryUpdateChild(child, delta);
  }

  for(auto child: geometryData->children)
  {
    geometryCalculateBranchesFinalAABB(child);
  }

  for(auto child: geometryData->children)
  {
    geometryCalculateLeafsFinalAABB(child);
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
    geometryMarkNeedAABBRecalculation(geometry);
  }
  else if(type == SCRIPT_FUNCTION_TYPE_IDF)
  {
    geometryData->idfs.push_back(function);
    // NOTE: Children also should be marked, because IDFs are integrated into leafs
    geometryMarkNeedRebuild(geometry, /** Mark children */ TRUE);
    geometryMarkNeedAABBRecalculation(geometry);    
  }
  else if(type == SCRIPT_FUNCTION_TYPE_ODF)
  {
    geometryData->odfs.push_back(function);
    geometryMarkNeedRebuild(geometry, /** Mark children */ FALSE);
    geometryMarkNeedAABBRecalculation(geometry);    
  }
  else if(type == SCRIPT_FUNCTION_TYPE_PCF)
  {
    geometryData->pcf = function;
    geometryMarkNeedRebuild(geometry, /** Mark children */ TRUE);    
    geometryMarkNeedAABBRecalculation(geometry);
  }
}

bool8 geometryRemoveFunction(Asset* geometry, Asset* function)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  
  ScriptFunctionType type = scriptFunctionGetType(function);
  if(type == SCRIPT_FUNCTION_TYPE_SDF)
  {
    if(geometryData->sdf != nullptr && geometryData->sdf == function)
    {
      geometryData->sdf = AssetPtr(nullptr);
      geometryMarkNeedRebuild(geometry, /** Mark children */ FALSE);
      geometryMarkNeedAABBRecalculation(geometry);      
      return TRUE;
    }
  }
  else if(type == SCRIPT_FUNCTION_TYPE_IDF)
  {
    auto idfIt = std::find(geometryData->idfs.begin(), geometryData->idfs.end(), function);
    if(idfIt != geometryData->idfs.end())
    {
      geometryData->idfs.erase(idfIt);
      geometryMarkNeedRebuild(geometry, /** Mark children */ TRUE);
      geometryMarkNeedAABBRecalculation(geometry);      
      return TRUE;
    }
  }
  else if(type == SCRIPT_FUNCTION_TYPE_ODF)
  {
    auto odfIt = std::find(geometryData->odfs.begin(), geometryData->odfs.end(), function);
    if(odfIt != geometryData->odfs.end())
    {
      geometryData->odfs.erase(odfIt);
      geometryMarkNeedRebuild(geometry, /** Mark children */ FALSE);
      geometryMarkNeedAABBRecalculation(geometry);      
      return TRUE;
    }
  }
  else if(type == SCRIPT_FUNCTION_TYPE_PCF)
  {
    if(geometryData->pcf != nullptr && geometryData->pcf == function)
    {
      geometryData->pcf = AssetPtr(nullptr);
      geometryMarkNeedRebuild(geometry, /** Mark children */ TRUE);
      geometryMarkNeedAABBRecalculation(geometry);      
      return TRUE;      
    }
  }
  
  return FALSE;
}

void geometryNotifyFunctionHasChanged(Asset* geometry, Asset* function)
{
  ScriptFunctionType type = scriptFunctionGetType(function);
  bool8 markChildren = (type == SCRIPT_FUNCTION_TYPE_IDF || type == SCRIPT_FUNCTION_TYPE_PCF) ? TRUE : FALSE;
  geometryMarkNeedRebuild(geometry, markChildren);
  geometryMarkNeedAABBRecalculation(geometry, markChildren);  
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

AssetPtr geometryGetPCF(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  
  return geometryData->pcf;
}

std::vector<AssetPtr> geometryGetScriptFunctions(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  
  std::vector<AssetPtr> functions;
  functions.reserve(1 + geometryData->idfs.size() + geometryData->odfs.size());

  if(geometryData->pcf != nullptr)
  {
    functions.push_back(geometryData->pcf);
  }
  
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

  if(function == geometryData->pcf)
  {
    return TRUE;
  }
  
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

void geometrySetSelected(Asset* geometry, bool8 selected)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  geometryData->selected = selected;  
}

bool8 geometryIsSelected(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  return geometryData->selected;
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

const float4x4& geometryGetWorldGeoMat(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  
  if(geometryData->dirty == TRUE)
  {
    geometryRecalculateTransforms(geometry);
  }

  return geometryData->transformToLocal;
}

const float4x4& geometryGetGeoWorldMat(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  
  if(geometryData->dirty == TRUE)
  {
    geometryRecalculateTransforms(geometry);
  }

  return geometryData->transformToWorld;
}

  
const float4x4& geometryGetParentGeoMat(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  
  if(geometryData->dirty == TRUE)
  {
    geometryRecalculateTransforms(geometry);
  }

  return geometryData->transformToLocalFromParent;
}

const float4x4& geometryGetGeoParentMat(Asset* geometry)
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

void geometrySetEnabled(Asset* geometry, bool8 enabled)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  geometryData->enabled = enabled;
}

bool8 geometryIsEnabled(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  return geometryData->enabled;
}

void geometrySetAABBAutomaticallyCalculated(Asset* geometry, bool8 automatically)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  geometryData->aabbAutomaticallyCalculated = automatically;
  if(automatically == TRUE)
  {
    geometryMarkNeedAABBRecalculation(geometry);
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
  geometryData->dirty = TRUE;
}

const AABB& geometryGetNativeAABB(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  return geometryData->nativeAABB;
}

const AABB& geometryGetDynamicAABB(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  if(geometryData->dirty == TRUE)
  {
    geometryRecalculateTransforms(geometry);
  }
  
  return geometryData->dynamicAABB;
}

const AABB& geometryGetFinalAABB(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  return geometryData->finalAABB;
}

void geometryMarkNeedAABBRecalculation(Asset* geometry, bool8 markChildren)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  geometryData->needAABBRecalculation = TRUE;

  if(markChildren == TRUE)
  {
    if(geometryData->children.size() > 0)
    {
      for(AssetPtr child: geometryData->children)
      {
        geometryMarkNeedAABBRecalculation(child, markChildren);
      }
    }
  }
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

ShaderProgram* geometryGetShadowProgram(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  return geometryData->shadowProgram;  
}

ShaderProgram* geometryGetAABBProgram(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  return geometryData->aabbProgram;
}

PCFNativeType geometryGetPCFNativeType(Asset* geometry)
{
  if(geometryIsRoot(geometry) == TRUE)
  {
    return PCF_NATIVE_TYPE_UNION;
  }
  
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);
  if(geometryData->pcf == nullptr)
  {
    return PCF_NATIVE_TYPE_UNKNOWN;
  }

  return pcfGetNativeType(geometryData->pcf);
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

const std::set<AssetPtr>& geometryRootGetAllChildren(Asset* root)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(root);
  return geometryData->allChildren;
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

  geometryChildWasAdded(geometry, child);
}

bool8 geometryRemoveChild(Asset* geometry, Asset* child)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);  
  
  auto childIt = std::find(geometryData->children.begin(), geometryData->children.end(), child);
  if(childIt == geometryData->children.end())
  {
    return FALSE;
  }

  geometryChildWasRemoved(geometry, *childIt);
  
  geometryData->children.erase(childIt);

  geometryMarkNeedRebuild(geometry, /** Mark children */ TRUE);
  geometryMarkNeedAABBRecalculation(geometry);  

  geometryRecalculateIDs(geometry);
  
  return TRUE;
}

std::vector<AssetPtr>& geometryGetChildren(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);  
  
  return geometryData->children;
}

uint32 geometryGetTotalChildrenCount(Asset* geometry)
{
  Geometry* geometryData = (Geometry*)assetGetInternalData(geometry);

  return geometryData->totalChildrenCount;
}

// ----------------------------------------------------------------------------
// Leaf geometry-related interface
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
