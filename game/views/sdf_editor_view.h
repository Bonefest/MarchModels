#pragma once

#include <dfunction.h>

#include "view.h"

bool8 createSDFEditorView(uint2 initialSize, View** outView);
void sdfEditorSetSDF(SDF* sdf);
