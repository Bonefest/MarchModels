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

void windowManagerAddWindow(WindowPtr window, bool8 initialize = TRUE);
// NOTE: We use raw window pointers to prevent cases where shared ptr is allocated by mistake
bool8 windowManagerRemoveWindow(Window* window);
bool8 windowManagerRemoveWindow(const std::string& identifier);
bool8 windowManagerHasWindow(const std::string& identifier);
WindowPtr windowManagerGetWindow(const std::string& identifier);
std::vector<WindowPtr> windowManagerGetWindows();

void windowManagerDraw(float64 delta);
void windowManagerUpdate(float64 delta);
void windowManagerProcessInput(const EventData& eventData, void* sender);
