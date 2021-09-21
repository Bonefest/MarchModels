#include <cstring>
#include <cstdlib>

#include "logging.h"
#include "memory_manager.h"

bool8 editorInitMemoryManager()
{
  /** Allocate memory for each memory type */
  return TRUE;
}

void editorShutdownMemoryManager()
{

}

void* editorAllocMem(uint32 memorySize, MemoryType memoryType)
{
  if(memoryType == MEMORY_TYPE_UNDEFINED)
  {
    LOG_WARNING("Allocation with undefined memory is undesired!");
  }

  void* mem = malloc(memorySize);
  memset(mem, 0, memorySize);

  return mem;
}

void editorFreeMem(void* memory, uint32 memorySize, MemoryType memoryType)
{
  return free(memory);
}

void editorSetZeroMem(void* memory, uint32 memorySize)
{
  memset(memory, 0, memorySize);
}

void editorCopyMem(void* dst, void* src, uint32 memorySize)
{
  memcpy(dst, src, memorySize);
}
