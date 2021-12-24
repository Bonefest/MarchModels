#include "program.h"
#include "memory_manager.h"
#include "shader_manager.h"
#include "renderer/renderer.h"
#include "renderer/renderer_utils.h"
#include <../bin/shaders/declarations.h>

#include "rasterization_pass.h"

struct RasterizationPassData
{
  GLuint raysMapFB;
  GLuint geometryAndDistancesFB;
  
  ShaderProgram* preparingProgram;
  ShaderProgram* raysMoverProgram;
  ShaderProgram* resultsExtractionProgram;
};

static void destroyRasterizationPass(RenderPass* pass)
{
  RasterizationPassData* data = (RasterizationPassData*)renderPassGetInternalData(pass);
  glDeleteFramebuffers(1, &data->raysMapFB);
  glDeleteFramebuffers(1, &data->geometryAndDistancesFB);
  
  destroyShaderProgram(data->preparingProgram);
  destroyShaderProgram(data->raysMoverProgram);
  destroyShaderProgram(data->resultsExtractionProgram);
  
  engineFreeObject(data, MEMORY_TYPE_GENERAL);
}

static bool8 rasterizationPassPrepareToRasterize(RasterizationPassData* data)
{
  shaderProgramUse(data->preparingProgram);
  glBindFramebuffer(GL_FRAMEBUFFER, data->raysMapFB);

  drawTriangleNoVAO();

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  shaderProgramUse(nullptr);

  return TRUE;
}

static void drawGeometryInorder(AssetPtr geometry)
{
  std::vector<AssetPtr>& children = geometryGetChildren(geometry);
  for(AssetPtr child: children)
  {
    drawGeometryInorder(child);
  }
  
  ShaderProgram* geometryProgram = geometryGetProgram(geometry);

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

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, rendererGetResourceHandle(RR_RAYS_MAP_TEXTURE));
  
  drawTriangleNoVAO();
  
  shaderProgramUse(nullptr);
}

static bool8 rasterizationPassRasterize(RasterizationPassData* data)
{
  const RenderingParameters& renderingParams = rendererGetPassedRenderingParameters();
  
  Scene* sceneToRasterize = rendererGetPassedScene();
  std::vector<AssetPtr>& sceneGeometry = sceneGetGeometry(sceneToRasterize);

  for(uint32 i = 0; i < renderingParams.rasterItersMaxCount; i++)
  {
    // Calculate distances
    for(AssetPtr geometry: sceneGeometry)
    {
      drawGeometryInorder(geometry);
    }

    // Move per-pixel rays based on calculated distances
    glEnable(GL_BLEND);
    pushBlend(GL_FUNC_ADD, GL_FUNC_ADD, GL_ZERO, GL_ONE, GL_ONE, GL_ONE);
    glBindFramebuffer(GL_FRAMEBUFFER, data->raysMapFB);
    
    shaderProgramUse(data->raysMoverProgram);

    glUniform1ui(glGetUniformLocation(shaderProgramGetGLProgram(data->raysMoverProgram), "curItemIdx"), i);
    drawTriangleNoVAO();
    shaderProgramUse(nullptr);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    assert(popBlend() == TRUE);
    glDisable(GL_BLEND);
  }
  

  return TRUE;
}

static bool8 rasterizationPassExtractResults(RasterizationPassData* data)
{
  glBindFramebuffer(GL_FRAMEBUFFER, data->geometryAndDistancesFB);
  shaderProgramUse(data->resultsExtractionProgram);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, rendererGetResourceHandle(RR_RAYS_MAP_TEXTURE));  
  drawTriangleNoVAO();
  
  shaderProgramUse(nullptr);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return TRUE;
}

static bool8 rasterizationPassExecute(RenderPass* pass)
{
  RasterizationPassData* data = (RasterizationPassData*)renderPassGetInternalData(pass);

  assert(rasterizationPassPrepareToRasterize(data));
  assert(rasterizationPassRasterize(data));
  assert(rasterizationPassExtractResults(data));
  
  return TRUE;
}

static const char* rasterizationPassGetName(RenderPass* pass)
{
  return "RasterizationPass";
}

static ShaderProgram* createAndLinkProgram(const char* fragmentShaderPath)
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

static GLuint createRayMapFramebuffer()
{
  GLuint framebuffer = 0;
  
  glGenFramebuffers(1, &framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, rendererGetResourceHandle(RR_RAYS_MAP_TEXTURE), 0);
  // TODO: attach stencil too
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  {
    return 0;
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return framebuffer;
}

static GLuint createGeometryAndDistancesFramebuffer()
{
  GLuint framebuffer = 0;
  
  glGenFramebuffers(1, &framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, rendererGetResourceHandle(RR_DISTANCES_MAP_TEXTURE), 0);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, rendererGetResourceHandle(RR_GEOIDS_MAP_TEXTURE), 0);  

  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  {
    return 0;
  }
  
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return framebuffer;
}

bool8 createRasterizationPass(RenderPass** outPass)
{
  RenderPassInterface interface = {};
  interface.destroy = destroyRasterizationPass;
  interface.execute = rasterizationPassExecute;
  interface.getName = rasterizationPassGetName;
  interface.type = RENDER_PASS_TYPE_RASTERIZATION;

  if(allocateRenderPass(interface, outPass) == FALSE)
  {
    return FALSE;
  }

  RasterizationPassData* data = engineAllocObject<RasterizationPassData>(MEMORY_TYPE_GENERAL);

  data->raysMapFB = createRayMapFramebuffer();
  assert(data->raysMapFB != 0);

  data->geometryAndDistancesFB = createGeometryAndDistancesFramebuffer();
  assert(data->geometryAndDistancesFB != 0);

  data->preparingProgram = createAndLinkProgram("shaders/prepare_to_raster.frag");
  assert(data->preparingProgram != nullptr);

  data->raysMoverProgram = createAndLinkProgram("shaders/rays_mover.frag");
  assert(data->raysMoverProgram != nullptr);

  data->resultsExtractionProgram = createAndLinkProgram("shaders/extract_raster_results.frag");
  assert(data->resultsExtractionProgram != nullptr);
  
  renderPassSetInternalData(*outPass, data);
  
  return TRUE;
}
