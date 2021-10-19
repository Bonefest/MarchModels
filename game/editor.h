#pragma once

#include <string>

#include <scene.h>
#include <application.h>

#include "windows/window.h"

bool8 initEditor(Application* app);
void shutdownEditor(Application* app);

void drawEditor(Application* app, float64 delta);
void updateEditor(Application* app, float64 delta);
void processInputEditor(Application* app, const EventData& eventData, void* sender);

void editorSetScene(Scene* scene);
Scene* editorGetCurrentScene();

