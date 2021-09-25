#include <cstring>
#include <cstdlib>

#include "logging.h"
#include "memory_manager.h"

bool8 engineInitMemoryManager()
{
  /** Allocate memory for each memory type */
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

  void* mem = malloc(memorySize);
  memset(mem, 0, memorySize);

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
