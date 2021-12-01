#include "memory_manager.h"
#include "application.h"
#include "logging.h"

#if TEST_COMPILE_PATH
  #include <gtest/gtest.h>
  #include "memory_manager_unit_tests.h"
  #include "shared_ptr_unit_tests.h"
  #include "event_system_unit_tests.h"
  #include "image_integrator_integration_tests.h"

  #include "event_system.h"
  #include "lua/lua_system.h"

  int initTests(int argc, char** argv)
  {
    ::testing::InitGoogleTest(&argc, argv);

    initLoggingSystem(2048);
    engineInitMemoryManager();
    initEventSystem();
    initializeLuaSystem();
    
    return RUN_ALL_TESTS();
  }
#endif

int initNormal(int argc, char** argv)
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


int main(int argc, char** argv)
{
  #if TEST_COMPILE_PATH
    return initTests(argc, argv);
  #else
    return initNormal(argc, argv);
  #endif
}
