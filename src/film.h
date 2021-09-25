#pragma once


#include "defines.h"
#include "linalg/linalg.h"
#include "memory_manager.h"

using namespace linalg::aliases;

struct Film;

EDITOR_API bool8 createFilm(uint2 size, Film** film);
EDITOR_API void destroyFilm(Film* film);

EDITOR_API bool8 filmResize(Film* film, uint2 newSize);
EDITOR_API void filmClear(Film* film, float3 value = float3());

EDITOR_API void filmSetPixel(Film* film, int2 location, float3 value);
EDITOR_API float3 filmLoadPixel(Film* film, int2 location, int2 offset = int2());

EDITOR_API uint2 filmGetSize(Film* film);

EDITOR_API GLenum filmGetGLTexture(Film* film);


