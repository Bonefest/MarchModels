#include <string>
using std::string;

#include <assets/geometry.h>
#include <assets/light_source.h>
#include <assets/assets_manager.h>

#include "editor_utils.h"

static AssetPtr createScriptFunction(const string& name)
{
  AssetPtr sfPrototype = assetsManagerFindAsset(name);
  assert(sfPrototype != nullptr);
  
  return sfPrototype;
}

AssetPtr createDefaultSDF()
{
  return createScriptFunction("sphereSDF");
}

AssetPtr createDefaultIDF()
{
  return createScriptFunction("emptyIDF");
}

AssetPtr createDefaultODF()
{
  return createScriptFunction("emptyODF");
}

AssetPtr createDefaultPCF()
{
  return createScriptFunction("unionPCF");
}

AssetPtr createNewGeometry()
{
  Asset* newGeometry;
  assert(createGeometry("sphere", &newGeometry));
  geometryAddFunction(newGeometry, createDefaultSDF());
  geometryAddFunction(newGeometry, createScriptFunction("unionPCF"));

  return AssetPtr(newGeometry);
}

AssetPtr createNewLight()
{
  Asset* newLight;
  createLightSource("point light", LIGHT_SOURCE_TYPE_POINT, &newLight);

  return AssetPtr(newLight);
}
