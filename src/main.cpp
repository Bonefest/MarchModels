#include "memory_manager.h"
#include "application.h"
#include "logging.h"
 
int main(void)
{
  if(engineInitMemoryManager() != TRUE)
  {
    LOG_ERROR("Cannot initialize a memory manager!");
    return -1;
  }

  if(!startApplication())
  {
    LOG_ERROR("Cannot start an application!");
    return -2;
  }
  
  engineShutdownMemoryManager();
  return 0;
}
