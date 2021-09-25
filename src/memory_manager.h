#pragma once

#include "defines.h"

enum MemoryType
{
  MEMORY_TYPE_UNDEFINED,
  MEMORY_TYPE_APPLICATION,
  MEMORY_TYPE_GENERAL,
  MEMORY_TYPE_PER_FRAME,
  MEMORY_TYPE_FILM,
};

ENGINE_API bool8 engineInitMemoryManager();
ENGINE_API void engineShutdownMemoryManager();
ENGINE_API void* engineAllocMem(uint32 memorySize, MemoryType memoryType = MEMORY_TYPE_UNDEFINED);
ENGINE_API void engineFreeMem(void* memory, uint32 memorySize, MemoryType memoryType = MEMORY_TYPE_UNDEFINED);
ENGINE_API void engineSetZeroMem(void* memory, uint32 memorySize);
ENGINE_API void engineCopyMem(void* dst, void* src, uint32 memorySize);

template <typename T>
T* engineAllocObject(MemoryType memoryType)
{
  T* newObj = static_cast<T*>(engineAllocMem(sizeof(T), memoryType));
  new(newObj) T;

  return newObj;
}

template <typename T>
T* engineAllocObjectsArray(uint32 arraySize, MemoryType memoryType)
{
  T* newArray = static_cast<T*>(engineAllocMem(sizeof(T) * arraySize, memoryType));
  for(uint32 i = 0; i < arraySize; i++)
  {
    new(newArray + i) T;
  }

  return newArray;
}

template <typename T>
void engineFreeObject(T* memory, MemoryType memoryType)
{
  memory->~T();
  engineFreeMem(memory, sizeof(T), memoryType);
}

template <typename T>
void engineFreeObjectsArray(T* memory, uint32 arraySize, MemoryType memoryType)
{
  for(uint32 i = 0; i < arraySize; i++)
  {
    memory[i].~T();
  }
  engineFreeMem(memory, sizeof(T) * arraySize, memoryType);
}
