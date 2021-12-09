#include <set>
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


void memoryAllocatorDestroy(MemoryAllocator* allocator)
{
  allocator->interface.destroy(allocator);
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
// General allocator implementation
// ----------------------------------------------------------------------------

struct GeneralMemoryAllocatorData
{
  uint32 maxAllowedMemorySize = 0;
  uint32 usedMemorySize = 0;
};

static bool8 initializeGeneralMemoryAllocator(MemoryAllocator* allocator)
{
  return TRUE;
}

static void destroyGeneralMemoryAllocator(MemoryAllocator* allocator)
{
  free(memoryAllocatorGetInternalData(allocator));
}

static void* generalMemoryAllocatorAllocMem(MemoryAllocator* allocator, uint32 memorySize, MemoryType memoryType)
{
  GeneralMemoryAllocatorData* data = (GeneralMemoryAllocatorData*)memoryAllocatorGetInternalData(allocator);
  data->usedMemorySize += memorySize;
  
  assert(data->usedMemorySize < data->maxAllowedMemorySize);

  return calloc(1, memorySize);
}

static void generalMemoryAllocatorFreeMem(MemoryAllocator* allocator,
                                          void* memory,
                                          uint32 memorySize,
                                          MemoryType memoryType)
{
  GeneralMemoryAllocatorData* data = (GeneralMemoryAllocatorData*)memoryAllocatorGetInternalData(allocator);
  assert(data->usedMemorySize >= memorySize);

  free(memory);
}

uint32 generalMemoryAllocatorGetBankSize(MemoryAllocator* allocator)
{
  GeneralMemoryAllocatorData* data = (GeneralMemoryAllocatorData*)memoryAllocatorGetInternalData(allocator);
  return data->maxAllowedMemorySize;
}

uint32 generalMemoryAllocatorGetUsedMemorySize(MemoryAllocator* allocator)
{
  GeneralMemoryAllocatorData* data = (GeneralMemoryAllocatorData*)memoryAllocatorGetInternalData(allocator);
  return data->usedMemorySize;
}

const char* generalMemoryAllocatorGetName(MemoryAllocator* allocator)
{
  return "General Memory Allocator";
}

static MemoryAllocator* createGeneralMemoryAllocator(uint32 maxAllowedMemorySize)
{
  MemoryAllocatorInterface interface = {};
  interface.initialize = initializeGeneralMemoryAllocator;
  interface.destroy = destroyGeneralMemoryAllocator;
  interface.allocMem = generalMemoryAllocatorAllocMem;
  interface.freeMem = generalMemoryAllocatorFreeMem;
  interface.getBankSize = generalMemoryAllocatorGetBankSize;
  interface.getUsedMemorySize = generalMemoryAllocatorGetUsedMemorySize;
  interface.getName = generalMemoryAllocatorGetName;
  interface.type = MEMORY_TYPE_GENERAL;
  
  // NOTE: We are force to use malloc instead of memoryAllocatorAlloc, because the latter
  // function internally relies on the general allocator
  MemoryAllocator* allocator = (MemoryAllocator*)malloc(sizeof(MemoryAllocator));
  allocator->interface = interface;

  GeneralMemoryAllocatorData* data = (GeneralMemoryAllocatorData*)malloc(sizeof(GeneralMemoryAllocatorData));
  *data = GeneralMemoryAllocatorData{};
  data->maxAllowedMemorySize = maxAllowedMemorySize;

  memoryAllocatorSetInternalData(allocator, data);

  return allocator;
}

// ----------------------------------------------------------------------------
// Memory manager
// ----------------------------------------------------------------------------

static std::unordered_map<MemoryType, MemoryAllocator*> registeredAllocators;

bool8 engineInitMemoryManager()
{
  MemoryAllocator* allocator = createGeneralMemoryAllocator(1 * GIBIBYTE);
  if(allocator == nullptr)
  {
    return FALSE;
  }

  // NOTE: Now we do not need to implement custom allocator for each type - use a general for everything
  registeredAllocators[MEMORY_TYPE_UNDEFINED] = allocator;  
  registeredAllocators[MEMORY_TYPE_GENERAL] = allocator;
  registeredAllocators[MEMORY_TYPE_APPLICATION] = allocator;
  registeredAllocators[MEMORY_TYPE_PER_FRAME] = allocator;
  registeredAllocators[MEMORY_TYPE_FILM] = allocator;    
  
  return TRUE;
}

void engineShutdownMemoryManager()
{
  // NOTE: Collect unique memory allocators
  std::set<MemoryAllocator*> uniqueAllocators;
  for(auto allocPair: registeredAllocators)
  {
    uniqueAllocators.insert(allocPair.second);
  }
  
  for(MemoryAllocator* allocator: uniqueAllocators)
  {
    memoryAllocatorDestroy(allocator);
  }

  registeredAllocators.clear();
}

void* engineAllocMem(uint32 memorySize, MemoryType memoryType)
{
  if(memoryType == MEMORY_TYPE_UNDEFINED)
  {
    LOG_WARNING("Allocation with undefined memory is undesired!");
  }
  
  auto allocIt = registeredAllocators.find(memoryType);
  if(allocIt == registeredAllocators.end())
  {
    LOG_ERROR("Allocation with type %u is requested but no associated allocator is found!", (uint32)memoryType);
    return nullptr;
  }

  return memoryAllocatorAllocateMem(allocIt->second, memorySize, memoryType);
}

void engineFreeMem(void* memory, uint32 memorySize, MemoryType memoryType)
{
  return free(memory);
}

void engineSetZeroMem(void* memory, uint32 memorySize)
{
  memset(memory, 0, memorySize);
}

void engineCopyMem(void* dst, const void* src, uint32 memorySize)
{
  memcpy(dst, src, memorySize);
}

bool8 engineRegisterAllocator(MemoryAllocator* allocator)
{
  if(allocator == nullptr)
  {
    return FALSE;
  }
  
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
