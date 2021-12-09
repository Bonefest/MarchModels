#pragma once

#include <new>

#include "defines.h"

using MemoryType = uint32;

// ----------------------------------------------------------------------------
// Memory allocator
// ----------------------------------------------------------------------------
struct MemoryAllocator;
struct MemoryAllocatorInterface
{
  bool8 (*initialize)(MemoryAllocator* allocator);
  void (*destroy)(MemoryAllocator* allocator);

  // NOTE: Allocation/Freeing functions accept memory type which in order allows a single allocator
  // to manage several memory types
  void* (*allocMem)(MemoryAllocator* allocator, uint32 memorySize, MemoryType memoryType);
  void (*freeMem)(MemoryAllocator* allocator, void* memory, uint32 memorySize, MemoryType memoryType);

  uint32 (*getBankSize)(MemoryAllocator* allocator);
  uint32 (*getUsedMemorySize)(MemoryAllocator* allocator);

  const char* (*getName)(MemoryAllocator* allocator);
  
  MemoryType type;
};

ENGINE_API bool8 memoryAllocatorAlloc(MemoryAllocatorInterface interface, MemoryAllocator** outAllocator);
ENGINE_API bool8 memoryAllocatorInitialize(MemoryAllocator* allocator);
ENGINE_API void memoryAllocatorDestroy(MemoryAllocator* allocator);
ENGINE_API void* memoryAllocatorAllocateMem(MemoryAllocator* allocator,
                                            uint32 memorySize,
                                            MemoryType memoryType);

ENGINE_API void memoryAllocatorFreeMem(MemoryAllocator* allocator,
                                       void* memory,
                                       uint32 memorySize,
                                       MemoryType memoryType);

ENGINE_API uint32 memoryAllocatorGetBankSize(MemoryAllocator* allocator);
ENGINE_API uint32 memoryAllocatorGetUsedMemorySize(MemoryAllocator* allocator);
ENGINE_API const char* memoryAllocatorGetName(MemoryAllocator* allocator);

ENGINE_API MemoryType memoryAllocatorGetType(MemoryAllocator* allocator);
ENGINE_API void memoryAllocatorSetInternalData(MemoryAllocator* allocator, void* data);
ENGINE_API void* memoryAllocatorGetInternalData(MemoryAllocator* allocator);

// ----------------------------------------------------------------------------
// Memory manager
// ----------------------------------------------------------------------------

const static MemoryType MEMORY_TYPE_UNDEFINED = 0;
const static MemoryType MEMORY_TYPE_GENERAL = 1; // NOTE: Now it's basically a wrapper of malloc
const static MemoryType MEMORY_TYPE_APPLICATION = 2;
const static MemoryType MEMORY_TYPE_PER_FRAME = 3;
const static MemoryType MEMORY_TYPE_FILM = 4;

ENGINE_API bool8 engineInitMemoryManager();
ENGINE_API void engineShutdownMemoryManager();
ENGINE_API void* engineAllocMem(uint32 memorySize, MemoryType memoryType = MEMORY_TYPE_UNDEFINED);
ENGINE_API void engineFreeMem(void* memory, uint32 memorySize, MemoryType memoryType = MEMORY_TYPE_UNDEFINED);
ENGINE_API void engineSetZeroMem(void* memory, uint32 memorySize);
ENGINE_API void engineCopyMem(void* dst, const void* src, uint32 memorySize);

ENGINE_API bool8 engineRegisterAllocator(MemoryAllocator* allocator);
ENGINE_API bool8 engineHasAllocatorWithType(MemoryType type);
ENGINE_API MemoryAllocator* engineGetAllocatorByType(MemoryType type);

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
