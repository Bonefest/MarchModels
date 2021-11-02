/**
 * WindowManager system is responsible for storing current opened windows. It has global knowledge
 * about windows. It also is responsible for freeing closed windows.
 *
 * @note Editor has a global window manager.
 * @note __It's highly advicable to add just created windows to the main window manager of
 * the editor because it moves responsibility of freeing resources from a user to the system.__
 */ 

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

void windowManagerDraw(WindowManager* manager, float64 delta);
void windowManagerUpdate(WindowManager* manager, float64 delta);
void windowManagerProcessInput(WindowManager* manager, const EventData& eventData, void* sender);
