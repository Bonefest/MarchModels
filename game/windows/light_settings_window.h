#pragma once

#include <string>

#include <scene.h>
#include <assets/geometry.h>

#include "window.h"

bool8 createLightSettingsWindow(AssetPtr light, Window** outWindow);

std::string lightSettingsWindowIdentifier(Asset* geometry);
