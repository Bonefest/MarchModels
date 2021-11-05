#pragma once

#include <assets/asset.h>

#include "window.h"

typedef void(*fpOnAssetsListItemSelected)(Window*, Asset*, uint32);

bool8 createAssetsListWindowWithAllAssets(Window** outWindow);
bool8 createAssetsListWindowWithSomeAssets(const std::vector<Asset*>& assetsToList, Window** outWindow);

void createAssetsListWindowSetSelectCallback(Window* window, fpOnAssetsListItemSelected calback, void* userData);

std::string assetsListWindowGetIdentifier();
