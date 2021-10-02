#include "film.h"

struct Film
{
  uint2 size;
  float3* pixels;

  bool8 textureGLAllocated;
  GLenum textureGL;
};

static inline uint32 calculateMemSize(uint2 size)
{
  return sizeof(float3) * size.x * size.y;
}

static inline uint32 calculateMemStride(uint2 size)
{
  return sizeof(float3) * size.x;
}

static inline uint32 calculatePixelIndex(Film* film, int2 location)
{
  return film->size.x * location.y + location.x;
}

static inline float3* calculatePixel(Film* film, int2 location)
{
  return film->pixels + calculatePixelIndex(film, location);
}

static void filmReallocateGLTexture(Film* film)
{
  if(film->textureGLAllocated == FALSE)
  {
    glGenTextures(1, &film->textureGL);

    glBindTexture(GL_TEXTURE_2D, film->textureGL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);    
    glBindTexture(GL_TEXTURE_2D, 0);                    
  }

  glBindTexture(GL_TEXTURE_2D, film->textureGL);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, film->size.x, film->size.y, 0, GL_RGB, GL_FLOAT, film->pixels);
  glBindTexture(GL_TEXTURE_2D, 0);

  assert(glGetError() == GL_NO_ERROR);    
}

bool8 createFilm(uint2 size, Film** film)
{
  *film = engineAllocObject<Film>(MEMORY_TYPE_FILM);
  (*film)->size = size;
  (*film)->pixels = nullptr;

  filmResize(*film, size);
  
  return TRUE;
}

void destroyFilm(Film* film)
{
  if(film->pixels != nullptr)
  {
    engineFreeMem(film->pixels, calculateMemSize(film->size), MEMORY_TYPE_FILM);
  }

  if(film->textureGLAllocated == TRUE)
  {
    glDeleteTextures(1, &film->textureGL);
  }
  
  engineFreeObject(film, MEMORY_TYPE_FILM);
}

bool8 filmResize(Film* film, uint2 newSize)
{
  if(film->pixels != nullptr)
  {
    engineFreeMem(film->pixels, calculateMemSize(film->size), MEMORY_TYPE_FILM);
  }

  film->pixels = (float3*)engineAllocMem(calculateMemSize(newSize), MEMORY_TYPE_FILM);

  if(film->textureGLAllocated)
  {
    filmReallocateGLTexture(film);
  }

  film->size = newSize;
  
  return TRUE;
}

void filmClear(Film* film, float3 value)
{
  float3* pixel = film->pixels;
  for(uint32 y = 0; y < film->size.y; y++)
  {
    for(uint32 x = 0; x < film->size.x; x++, pixel++)
    {
      *pixel = value;
    }
  }
}

void filmSetPixel(Film* film, int2 location, float3 value)
{
  *calculatePixel(film, location) = value;
}

float3 filmLoadPixel(Film* film, int2 location, int2 offset)
{
  return *calculatePixel(film, location + offset);
}

uint2 filmGetSize(Film* film)
{
  return film->size;
}

GLenum filmGetGLTexture(Film* film)
{
  if(film->textureGLAllocated == FALSE)
  {
    // NOTE: Reallocation also uploads pixels data
    filmReallocateGLTexture(film);
    film->textureGLAllocated = TRUE;
  }
  else
  {
    glBindTexture(GL_TEXTURE_2D, film->textureGL);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, film->size.x, film->size.y, GL_RGB, GL_FLOAT, film->pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  return film->textureGL;
}
