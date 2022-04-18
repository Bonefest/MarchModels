#include <string>
using std::string;

#include <image_manager.h>

#include <assets/material.h>
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
  geometrySetMaterial(newGeometry, assetsManagerFindAsset("default_material"));

  return AssetPtr(newGeometry);
}

AssetPtr createNewLight()
{
  Asset* newLight;
  createLightSource("point light", LIGHT_SOURCE_TYPE_POINT, &newLight);

  return AssetPtr(newLight);
}

AssetPtr createDefaultMaterial()
{
  Asset* material;
  createMaterial("default_material", &material);

  materialSetProjectionMode(material, MATERIAL_TEXTURE_PROJECTION_MODE_TRIPLANAR);
  materialSetTexture(material, MATERIAL_TEXTURE_TYPE_DIFFUSE, imageManagerLoadImage("assets/default_albedo.png"));
  materialSetTextureRegion(material, MATERIAL_TEXTURE_TYPE_DIFFUSE, uint4(0, 0, 512, 512));
  materialSetEnabledTexture(material, MATERIAL_TEXTURE_TYPE_DIFFUSE, TRUE);
  materialSetIOR(material, 1.0f);
  materialSetAO(material, 0.0f);
  materialSetMetallic(material, 0.0f);
  materialSetRoughness(material, 1.0f);
  
  return AssetPtr(material);
}
