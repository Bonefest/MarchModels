#pragma once

#include <string>

#include <scene.h>
#include <assets/geometry.h>

#include "window.h"

bool8 createGeometrySettingsWindow(Scene* scene, AssetPtr geometry, Window** outWindow);

std::string geometrySettingsWindowIdentifier(Asset* geometry);
