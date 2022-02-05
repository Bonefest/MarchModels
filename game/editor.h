#pragma once

#include <string>

#include <scene.h>
#include <application.h>

#include "windows/window.h"
#include "windows/window_manager.h"

bool8 initEditor(Application* app);
void shutdownEditor(Application* app);

void editorDraw(Application* app, float64 delta);
void editorUpdate(Application* app, float64 delta);
void editorProcessInput(Application* app, const EventData& eventData, void* sender);

void editorSetScene(Scene* scene);
Scene* editorGetCurrentScene();

bool8 editorAddSelectedGeometry(AssetWPtr geometry);
bool8 editorGeometryIsSelected(AssetWPtr geometry);
bool8 editorRemoveSelectedGeometry(AssetWPtr geometry);
void editorClearSelectedGeometry();
std::vector<AssetWPtr>& editorGetSelectedGeometry();

