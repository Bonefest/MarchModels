#include "memory_manager.h"
#include "application.h"
#include "logging.h"
 
int main(void)
{
  if(initLoggingSystem(2048) != TRUE)
  {
    fprintf(stderr, "Cannot initialize a logging system!");
    return -1;
  }
  
  if(engineInitMemoryManager() != TRUE)
  {
    LOG_ERROR("Cannot initialize a memory manager!");
    return -2;
  }

  if(!startApplication())
  {
    LOG_ERROR("Cannot start an application!");
    return -3;
  }
  
  engineShutdownMemoryManager();
  shutdownLoggingSystem();
  
  return 0;
}
