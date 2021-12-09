/**
 * TODO:
 * 1. Absraction over file for different platforms
 * 2. Async file reading
 */

#pragma once

#include "defines.h"

ENGINE_API bool8 readWholeFile(const char* filename, uint32* outFileSize, char** outFileContent);
ENGINE_API void freeFileContent(uint32 fileSize, char* fileContent);

