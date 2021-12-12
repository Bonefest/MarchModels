#ifndef COMMON_GLSL_INCLUDED
#define COMMON_GLSL_INCLUDED

  #include declarations.h

  layout(std140, binding = GLOBAL_PARAMS_UBO_BINDING) uniform GlobalParametersUBO
  {
    GlobalParameters parameters;
  };

#endif
