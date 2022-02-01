#include <string>
using std::string;

#include <assets/geometry.h>
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

AssetPtr createNewGeometry()
{
  Asset* newGeometry;
  assert(createGeometry("sphere", &newGeometry));
  geometryAddFunction(newGeometry, createDefaultSDF());
  geometryAddFunction(newGeometry, createScriptFunction("unionPCF"));

  return AssetPtr(newGeometry);
}
