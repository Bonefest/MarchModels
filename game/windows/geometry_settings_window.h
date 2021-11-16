#pragma once

#include <string>

#include <assets/geometry.h>

#include "window.h"

bool8 createGeometrySettingsWindow(AssetPtr geometry, Window** outWindow);

std::string geometrySettingsWindowIdentifier(Asset* geometry);
