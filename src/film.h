#pragma once

#include "defines.h"
#include "maths/common.h"
#include "memory_manager.h"

struct Film;

ENGINE_API bool8 createFilm(uint2 size, Film** film);
ENGINE_API void destroyFilm(Film* film);

ENGINE_API bool8 filmResize(Film* film, uint2 newSize);
ENGINE_API void filmClear(Film* film, float3 value = float3());

ENGINE_API void filmSetPixel(Film* film, int2 location, float3 value);
ENGINE_API float3 filmLoadPixel(Film* film, int2 location, int2 offset = int2());

ENGINE_API uint2 filmGetSize(Film* film);

ENGINE_API GLuint filmGetGLHandle(Film* film);
ENGINE_API GLuint filmGetGLFBOHandle(Film* film);


