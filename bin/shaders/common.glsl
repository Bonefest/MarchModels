#include "declarations.h"

layout(std140, binding = GLOBAL_PARAMS_UBO_BINDING) uniform GlobalParametersUBO
{
  GlobalParameters parameters;
};
