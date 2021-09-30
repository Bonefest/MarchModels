#pragma once

#include <script_function.h>

#include "view.h"

bool8 createSDFEditorView(View** outView);
void sdfEditorSetSDF(ScriptFunction* sdf);
