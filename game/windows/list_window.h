#pragma once

#include <string>
#include <vector>
#include <functional>

#include <assets/asset.h>

#include "window.h"

using fpOnListItemSelected =  std::function<void(Window* window, void* itemData, uint32 itemIdx, void* userData)>;
// typedef void(*fpOnListItemSelected)(Window* window, void* itemData, uint32 itemIdx, void* userData);

struct ListItem
{
  std::string label;
  void* data;
};

bool8 createListWindow(const std::string& identifier,
                       const std::string& tip,
                       const std::vector<ListItem>& items,
                       fpOnListItemSelected selectedCallback,
                       void* userData,                       
                       Window** outWindow);

void listWindowSetCloseOnLoseFocus(Window* window, bool8 close);
void listWindowSetShowIndex(Window* window, bool8 show);
void listWindowSetMarker(Window* window, const std::string& marker);
