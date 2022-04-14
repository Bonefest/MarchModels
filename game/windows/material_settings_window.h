#pragma once

#include <string>

#include <scene.h>
#include <assets/geometry.h>

#include "window.h"

bool8 createMaterialSettingsWindow(AssetPtr material, AssetPtr geometryOwner, Window** outWindow);

std::string materialSettingsWindowIdentifier(Asset* material);
