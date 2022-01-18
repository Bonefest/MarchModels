#include <../bin/shaders/declarations.h>
#include <../bin/shaders/fp_math.h>

#include "renderer/renderer_utils.h"
#include "geometry_native_aabb_calculation_pass.h"

struct AABBCalculationPassCommonData
{
  GLuint aabbBufferHandle;

  bool8 initialized;
};

static AABBCalculationPassCommonData data;

bool8 initializeAABBCalculationPass()
{
  if(data.initialized == TRUE)
  {
    return FALSE;
  }
  
  glGenBuffers(1, &data.aabbBufferHandle);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, data.aabbBufferHandle);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(AABBCalculationBufferParameters), NULL, GL_DYNAMIC_READ);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, AABB_CALCULATION_SSBO_BINDING, data.aabbBufferHandle);
  
  data.initialized = TRUE;
  
  return TRUE;
}

void destroyAABBCalculationPass()
{
  if(data.initialized == FALSE)
  {
    return;
  }

  glDeleteBuffers(1, &data.aabbBufferHandle);
}

AABB AABBCalculationPassCalculateAABB(Asset* geometry)
{
  const uint32 iterationsCount = 12;
  const uint32 viewportSize = 1000;

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, data.aabbBufferHandle);

  AABBCalculationBufferParameters aabbParams =
  {
    uint4(floatToFixedPoint(1.0f), floatToFixedPoint(1.0f), floatToFixedPoint(1.0f), 0),
    uint4(floatToFixedPoint(-1.0f), floatToFixedPoint(-1.0f), floatToFixedPoint(-1.0f), 0),
  };
  
  glBufferSubData(GL_SHADER_STORAGE_BUFFER,
                  0,
                  sizeof(AABBCalculationBufferParameters),
                  &aabbParams);
                  
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);  
  
  ShaderProgram* aabbProgram = geometryGetAABBProgram(geometry);
  shaderProgramUse(aabbProgram);

  glUniform2ui(1, viewportSize, viewportSize);    
  glUniform2f(2, 1.0 / viewportSize, 1.0 / viewportSize);
  
  for(uint32 i = 0; i < iterationsCount; i++)
  {
    glUniform1ui(0, i);
    glDispatchCompute(viewportSize / 32, viewportSize / 32, 1);
  }

  shaderProgramUse(nullptr);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, data.aabbBufferHandle);
  AABBCalculationBufferParameters* mappedParams =
    (AABBCalculationBufferParameters*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER,
                                                       0,
                                                       sizeof(AABBCalculationBufferParameters),
                                                       GL_MAP_READ_BIT);

  AABB result = AABB(
    float3(fixedPointToFloat(mappedParams->min.x), fixedPointToFloat(mappedParams->min.y), fixedPointToFloat(mappedParams->min.z)),
    float3(fixedPointToFloat(mappedParams->max.x), fixedPointToFloat(mappedParams->max.y), fixedPointToFloat(mappedParams->max.z))
  );
  
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

  return result;
}
