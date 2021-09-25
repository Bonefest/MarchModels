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

EDITOR_API bool8 editorInitMemoryManager();
EDITOR_API void editorShutdownMemoryManager();
EDITOR_API void* editorAllocMem(uint32 memorySize, MemoryType memoryType = MEMORY_TYPE_UNDEFINED);
EDITOR_API void editorFreeMem(void* memory, uint32 memorySize, MemoryType memoryType = MEMORY_TYPE_UNDEFINED);
EDITOR_API void editorSetZeroMem(void* memory, uint32 memorySize);
EDITOR_API void editorCopyMem(void* dst, void* src, uint32 memorySize);

template <typename T>
T* editorAllocObject(MemoryType memoryType)
{
  T* newObj = static_cast<T*>(editorAllocMem(sizeof(T), memoryType));
  new(newObj) T;

  return newObj;
}

template <typename T>
T* editorAllocObjectsArray(uint32 arraySize, MemoryType memoryType)
{
  T* newArray = static_cast<T*>(editorAllocMem(sizeof(T) * arraySize, memoryType));
  for(uint32 i = 0; i < arraySize; i++)
  {
    new(newArray + i) T;
  }

  return newArray;
}

template <typename T>
void editorFreeObject(T* memory, MemoryType memoryType)
{
  memory->~T();
  editorFreeMem(memory, sizeof(T), memoryType);
}

template <typename T>
void editorFreeObjectsArray(T* memory, uint32 arraySize, MemoryType memoryType)
{
  for(uint32 i = 0; i < arraySize; i++)
  {
    memory[i].~T();
  }
  editorFreeMem(memory, sizeof(T) * arraySize, memoryType);
}
