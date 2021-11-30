#include <cstring>
#include <cstdlib>
#include <unordered_map>

#include "logging.h"
#include "memory_manager.h"

// ----------------------------------------------------------------------------
// Memory allocator
// ----------------------------------------------------------------------------
struct MemoryAllocator
{
  MemoryAllocatorInterface interface;
  
  void* internalData;
};

bool8 memoryAllocatorAlloc(MemoryAllocatorInterface interface, MemoryAllocator** outAllocator)
{
  *outAllocator = engineAllocObject<MemoryAllocator>(MEMORY_TYPE_GENERAL);
  MemoryAllocator* allocator = *outAllocator;
  allocator->interface = interface;
  allocator->internalData = nullptr;

  return TRUE;
}

bool8 memoryAllocatorInitialize(MemoryAllocator* allocator)
{
  return allocator->interface.initialize(allocator);
}


void memoryAllocatorShutdown(MemoryAllocator* allocator)
{
  allocator->interface.shutdown(allocator);
  engineFreeObject(allocator, MEMORY_TYPE_GENERAL);
}

void* memoryAllocatorAllocateMem(MemoryAllocator* allocator,
                                 uint32 memorySize,
                                 MemoryType memoryType)
{
  return allocator->interface.allocMem(allocator, memorySize, memoryType);
}

void memoryAllocatorFreeMem(MemoryAllocator* allocator,
                            void* memory,
                            uint32 memorySize,
                            MemoryType memoryType)
{
  return allocator->interface.freeMem(allocator, memory, memorySize, memoryType);
}

uint32 memoryAllocatorGetBankSize(MemoryAllocator* allocator)
{
  return allocator->interface.getBankSize(allocator);
}

uint32 memoryAllocatorGetUsedMemorySize(MemoryAllocator* allocator)
{
  return allocator->interface.getUsedMemorySize(allocator);
}

const char* memoryAllocatorGetName(MemoryAllocator* allocator)
{
  return allocator->interface.getName(allocator);
}

MemoryType memoryAllocatorGetType(MemoryAllocator* allocator)
{
  return allocator->interface.type;
}

void memoryAllocatorSetInternalData(MemoryAllocator* allocator, void* data)
{
  allocator->internalData = data;
}

void* memoryAllocatorGetInternalData(MemoryAllocator* allocator)
{
  return allocator->internalData;
}

// ----------------------------------------------------------------------------
// Memory manager
// ----------------------------------------------------------------------------

using std::unordered_map;
static unordered_map<MemoryType, MemoryAllocator*> registeredAllocators;
static bool8 managerIsInitialized = FALSE;

bool8 engineInitMemoryManager()
{
  /** Allocate memory for each memory type */
  managerIsInitialized = TRUE;
  return TRUE;
}

void engineShutdownMemoryManager()
{

}

void* engineAllocMem(uint32 memorySize, MemoryType memoryType)
{
  if(memoryType == MEMORY_TYPE_UNDEFINED)
  {
    LOG_WARNING("Allocation with undefined memory is undesired!");
  }

  void* mem = nullptr;
  if(TRUE)// NOTE: Everything now is allocated with malloc
          // in the future it will be only for memoryType == MEMORY_TYPE_UNDEFINED || memoryType == MEMORY_TYPE_GENERAL)
  {
    mem = malloc(memorySize);
    memset(mem, 0, memorySize);
  }
  else
  {
    auto allocIt = registeredAllocators.find(memoryType);
    if(allocIt == registeredAllocators.end())
    {
      LOG_ERROR("Allocation with type %u is requested but no associated allocator is found!", (uint32)memoryType);
    }
    else
    {
      return memoryAllocatorAllocateMem(allocIt->second, memorySize, memoryType);
    }
  }

  return mem;
}

void engineFreeMem(void* memory, uint32 memorySize, MemoryType memoryType)
{
  return free(memory);
}

void engineSetZeroMem(void* memory, uint32 memorySize)
{
  memset(memory, 0, memorySize);
}

void engineCopyMem(void* dst, void* src, uint32 memorySize)
{
  memcpy(dst, src, memorySize);
}

bool8 engineRegisterAllocator(MemoryAllocator* allocator)
{
  MemoryType type = memoryAllocatorGetType(allocator);
  if(engineHasAllocatorWithType(type) == TRUE)
  {
    return FALSE;
  }

  registeredAllocators[type] = allocator;
  assert(memoryAllocatorInitialize(allocator));

  return TRUE;
}

bool8 engineHasAllocatorWithType(MemoryType type)
{
  return registeredAllocators.find(type) != registeredAllocators.end() ? TRUE : FALSE;
}

MemoryAllocator* engineGetAllocatorByType(MemoryType type)
{
  auto allocIt = registeredAllocators.find(type);
  if(allocIt == registeredAllocators.end())
  {
    return nullptr;
  }

  return allocIt->second;
}
