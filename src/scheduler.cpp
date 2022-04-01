#include <vector>
#include <unordered_map>

using std::vector;
using std::unordered_map;

#include "scheduler.h"

struct ScheduleFunctionData
{
  schedulerFunction function;
  void* userData;
  float32 updateTime;
  float32 elapsedTime;
};

struct SchedulerData
{
  bool8 initialized;
  
  unordered_map<void*, vector<ScheduleFunctionData>> ownerFunctions;
};

static SchedulerData data;

bool8 initSchedulerSystem()
{
  data.initialized = TRUE;
  return TRUE;
}

void shutdownSchedulerSystem()
{
  data.ownerFunctions.clear();
}

void schedulerRegisterFunction(schedulerFunction function,
                               float32 updateTime,
                               void* owner,
                               void* userData)
{
  data.ownerFunctions[owner].push_back(ScheduleFunctionData{function, userData, updateTime, 0.0f});
}

void schedulerUpdate(float32 delta)
{
  for(auto& ownerPair: data.ownerFunctions)
  {
    for(ScheduleFunctionData& functionData: ownerPair.second)
    {
      functionData.elapsedTime += delta;
      if(functionData.elapsedTime > functionData.updateTime)
      {

        functionData.function(functionData.updateTime, ownerPair.first, functionData.userData);
        functionData.elapsedTime = 0.0f;
      }
    }
  }
}

