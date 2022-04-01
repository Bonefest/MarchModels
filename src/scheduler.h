#pragma once

#include <functional>

#include <logging.h>
#include "defines.h"

using schedulerFunction = std::function<void(float32, void*, void*)>;

ENGINE_API bool8 initSchedulerSystem();
ENGINE_API void shutdownSchedulerSystem();

ENGINE_API void schedulerRegisterFunction(schedulerFunction function,
                                          float32 updateTime,
                                          void* owner = nullptr,
                                          void* userData = nullptr);

template <typename FunctionType, typename T>
void schedulerRegisterFunctionT1(FunctionType function,
                                 float32 updateTime,
                                 T param1,                                
                                 void* owner = nullptr,
                                 void* userData = nullptr)
{
  auto lambda = [function, param1](float32 time, void* owner, void* userData)
  {

    function(time, param1, owner, userData);
  };

  schedulerRegisterFunction(lambda, updateTime, owner, userData);
}


void schedulerUpdate(float32 delta);
