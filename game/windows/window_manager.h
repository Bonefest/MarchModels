#pragma once

#include <string>
#include <vector>

#include "window.h"

struct WindowManager;

bool8 createWindowManager(WindowManager** outWindowManager);
void destroyWindowManager(WindowManager* manager);

void windowManagerAddWindow(WindowManager* manager, Window* window, bool8 initialize = TRUE);
bool8 windowManagerRemoveWindow(WindowManager* manager, Window* window, bool8 free = TRUE);
bool8 windowManagerRemoveWindow(WindowManager* manager, const std::string& identifier, bool8 free = TRUE);
bool8 windowManagerHasWindow(WindowManager* manager, const std::string& identifier);
Window* windowManagerGetWindow(WindowManager* manager, const std::string& identifier);
std::vector<Window*> windowManagerGetWindows(WindowManager* manager);

void windowManagerToggleWindow(WindowManager* manager, Window* window);
void windowManagerToggleWindow(WindowManager* manager, const std::string& identifier);

bool8 windowManagerIsWindowVisible(WindowManager* manager, Window* window);
bool8 windowManagerIsWindowVisible(WindowManager* manager, const std::string& identifier);

void windowManagerDraw(WindowManager* manager, float64 delta);
void windowManagerUpdate(WindowManager* manager, float64 delta);
void windowManagerProcessInput(WindowManager* manager, const EventData& eventData, void* sender);
