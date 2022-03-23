#include <shader_manager.h>
#include <renderer/renderer.h>
#include <renderer/renderer_utils.h>

#include "passes_common.h"

ShaderProgram* createAndLinkTriangleShadingProgram(const char* fragmentShaderPath)
{
  ShaderProgram* program = nullptr;
  
  createShaderProgram(&program);
  shaderProgramAttachShader(program, shaderManagerGetShader("triangle.vert"));
  shaderProgramAttachShader(program, shaderManagerLoadShader(GL_FRAGMENT_SHADER, fragmentShaderPath));

  if(linkShaderProgram(program) == FALSE)
  {
    destroyShaderProgram(program);
    return nullptr;
  }
  
  return program;
}

bool8 drawGeometryPostorder(Camera* camera,
                            AssetPtr geometry,
                            uint32 indexInBranch,
                            uint32 culledSiblingsCount,
                            uint32& culledObjCounter,
                            bool8 shadowPath)
{
  const AABB& geometryAABB = geometryGetFinalAABB(geometry);
  if(camera != nullptr && cameraGetFrustum(camera).intersects(geometryAABB) == FALSE)
  {
    // NOTE: We've culled the object + its children
    culledObjCounter += geometryGetTotalChildrenCount(geometry) + 1;
    return FALSE;
  }

  std::vector<AssetPtr>& children = geometryGetChildren(geometry);

  // NOTE: This counter stores number of culled objects on the current level, this number is crucial for
  // some optimizations (e.g knowing that all previous siblings were culled, we know
  // that we must not read the stack)
  uint32 culledChildrenCount = 0;
  for(uint32 i = 0; i < children.size(); i++)
  {
    if(drawGeometryPostorder(camera, children[i], i, culledChildrenCount, culledObjCounter, shadowPath) == FALSE)
    {
      culledChildrenCount++;
    }
    else
    {
      // NOTE: Read https://gamedev.stackexchange.com/questions/151563/synchronization-between-several-gldispatchcompute-with-same-ssbos;
      // The idea is that we need to tell OpenGL explicitly that we want to synchronize several draw calls, which are
      // reading/writing from the SSBO. Otherwise some strange artifacts may occur.
      glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }
  }

  if(!children.empty() && culledChildrenCount == children.size())
  {
    return FALSE;
  }
  
  // NOTE: If it's a root - omit further actions, because it's treated in a special way
  // (it's not a real geometry object)
  if(geometryIsRoot(geometry) == TRUE)
  {
    return TRUE;
  }
  
  ShaderProgram* geometryProgram = shadowPath == TRUE ? geometryGetShadowProgram(geometry) : geometryGetDrawProgram(geometry);
  if(geometryProgram == nullptr)
  {
    return FALSE;
  }

  GeometryTransformParameters geoTransforms = {};
  geoTransforms.position = float4(geometryGetPosition(geometry), 1.0);
  geoTransforms.geoWorldMat = geometryGetGeoWorldMat(geometry);
  geoTransforms.worldGeoMat = geometryGetWorldGeoMat(geometry);
  geoTransforms.geoParentMat = geometryGetGeoParentMat(geometry);
  geoTransforms.parentGeoMat = geometryGetParentGeoMat(geometry);
  
  glBindBuffer(GL_UNIFORM_BUFFER, rendererGetResourceHandle(RR_GEOTRANSFORM_PARAMS_UBO));
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GeometryTransformParameters), &geoTransforms);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);                  

  shaderProgramUse(geometryProgram);

  glUniform1ui(glGetUniformLocation(shaderProgramGetGLHandle(geometryProgram), "geometryID"),
               geometryGetID(geometry));
  glUniform1ui(glGetUniformLocation(shaderProgramGetGLHandle(geometryProgram), "indexInBranch"),
               indexInBranch);
  glUniform1ui(glGetUniformLocation(shaderProgramGetGLHandle(geometryProgram), "prevCulledSiblingsCount"),
               culledSiblingsCount);
  
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, rendererGetResourceHandle(RR_RAYS_MAP_TEXTURE));
  glUniform1i(0, 0);

  if(shadowPath == TRUE)
  {
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, rendererGetResourceHandle(RR_DISTANCES_MAP_TEXTURE));
    glUniform1i(1, 1);
  }
  
  drawTriangleNoVAO();
  
  shaderProgramUse(nullptr);

  return TRUE;  
}
