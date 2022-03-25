#pragma once

#include <vector>

#include "assets/geometry.h"
#include "assets/light_source.h"

struct Scene;

ENGINE_API bool8 createScene(Scene** outScene);
ENGINE_API void destroyScene(Scene* scene);

ENGINE_API void updateScene(Scene* scene, float64 delta);

ENGINE_API void sceneAddGeometry(Scene* scene, AssetPtr geometry);
ENGINE_API bool8 sceneRemoveGeometry(Scene* scene, AssetPtr geometry);
ENGINE_API std::vector<AssetPtr>& sceneGetChildren(Scene* scene);
ENGINE_API AssetPtr sceneGetGeometryRoot(Scene* scene);
ENGINE_API const std::set<AssetPtr>& sceneGetAllChildren(Scene* scene);

ENGINE_API bool8 sceneAddLightSource(Scene* scene, AssetPtr lightSource);
ENGINE_API bool8 sceneRemoveLightSource(Scene* scene, AssetPtr lightSource);
ENGINE_API std::vector<AssetPtr>& sceneGetLightSources(Scene* scene);
ENGINE_API std::vector<AssetPtr> sceneGetEnabledLightSources(Scene* scene);
