/**
 * WindowManager system is responsible for controlling, displaying, updating and informing windows.
 * It controls resources of the given objects and is responsible for freeing them whenever windows is closed.
 * Each created Window should be added to this system manually, otherwise it won't be considered and controlled.
 */ 

#pragma once

#include <string>
#include <vector>

#include "window.h"

struct WindowManager;

bool8 initWindowManager();
void shutdownWindowManager();

void windowManagerAddWindow(Window* window, bool8 initialize = TRUE);
bool8 windowManagerRemoveWindow(Window* window, bool8 free = TRUE);
bool8 windowManagerRemoveWindow(const std::string& identifier, bool8 free = TRUE);
bool8 windowManagerHasWindow(const std::string& identifier);
Window* windowManagerGetWindow(const std::string& identifier);
std::vector<Window*> windowManagerGetWindows();

void windowManagerDraw(float64 delta);
void windowManagerUpdate(float64 delta);
void windowManagerProcessInput(const EventData& eventData, void* sender);
