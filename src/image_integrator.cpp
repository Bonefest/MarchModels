#include "lua/lua_system.h"
#include "memory_manager.h"

#include "shader_manager.h"
#include "image_integrator.h"

#include <../bin/shaders/declarations.h>

struct ImageIntegrator
{
  Scene* scene;
  Sampler* sampler;
  RayIntegrator* rayIntegrator;
  Film* film;
  Camera* camera;

  uint2 pixelGap;
  uint2 initialOffset;

  GLuint stackSSBO;

  GlobalParameters parameters;
  GLuint globalParamsUBO;

  GLuint drawingMaskTexture;
  GLuint rayMapTexture;

  GLuint rayMapInitFramebuffer;

  ShaderProgram* prepareFrameProgram;
  ShaderProgram* raysMoverProgram;
  
  void* internalData;
};

static void imageIntegratorSetupCommonScriptData(ImageIntegrator* integrator, float32 time)
{
  sol::state& lua = luaGetMainState();
  lua["args"]["time"] = time;
  // lua["args"]["camera"] = cameraGetLuaTable(integrator->camera);
}

static void imageIntegratorSetupGlobalParameters(ImageIntegrator* integrator, float32 time)
{
  GlobalParameters& parameters = integrator->parameters;
  
  parameters.time = time;
  parameters.tone = 0;
  parameters.pixelGapX = 0;
  parameters.pixelGapY = 0;    
  parameters.resolution = filmGetSize(integrator->film);
  parameters.invResolution = float2(1.0f / parameters.resolution.x, 1.0f / parameters.resolution.y);
  
  parameters.camPosition = float4(cameraGetPosition(integrator->camera), 1.0);
  parameters.camOrientation = cameraGetOrientation(integrator->camera);
  parameters.camNDCCameraMat = cameraGetNDCCameraMat(integrator->camera);
  parameters.camCameraNDCMat = cameraGetCameraNDCMat(integrator->camera);
  parameters.camNDCWorldMat = cameraGetNDCWorldMat(integrator->camera);
  parameters.camWorldNDCMat = cameraGetWorldNDCMat(integrator->camera);
  parameters.camCameraWorldMat = cameraGetCameraWorldMat(integrator->camera);
  parameters.camWorldCameraMat = cameraGetWorldCameraMat(integrator->camera);

  glBindBuffer(GL_UNIFORM_BUFFER, integrator->globalParamsUBO);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GlobalParameters), &parameters);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);                  
}

static bool8 imageIntegratorShouldIntegratePixelLocation(ImageIntegrator* integrator, int2 loc)
{
  uint2 gap = integrator->pixelGap;
  uint2 offset = integrator->initialOffset;
  
  return (loc.x + offset.x) % gap.x == 0 && (loc.y + offset.y) % gap.y == 0;  
}

static void drawGeometryInorder(AssetPtr geometry)
{
  std::vector<AssetPtr>& children = geometryGetChildren(geometry);
  for(AssetPtr child: children)
  {
    drawGeometryInorder(child);
  }
  
  ShaderProgram* geometryProgram = geometryGetProgram(geometry);

  GLuint geoPositionUniformPos = glGetUniformLocation(shaderProgramGetGLProgram(geometryProgram),
                                                      "geoPosition");

  GLuint geoGeoWorldMatUniformPos = glGetUniformLocation(shaderProgramGetGLProgram(geometryProgram),
                                                         "geoGeoWorldMat");
  
  GLuint geoWorldGeoMatUniformPos = glGetUniformLocation(shaderProgramGetGLProgram(geometryProgram),
                                                         "geoWorldGeoMat");

  float4 geoPosition = float4(geometryGetPosition(geometry), 1.0);
  float4x4 geoWorldMat = geometryGetGeoWorldMat(geometry);
  float4x4 worldGeoMat = geometryGetWorldGeoMat(geometry);

  shaderProgramUse(geometryProgram);
  
  glUniform4fv(geoPositionUniformPos, 1, (float32*)&geoPosition);
  glUniformMatrix4fv(geoGeoWorldMatUniformPos, 1, GL_FALSE, (float32*)&geoWorldMat[0]);
  glUniformMatrix4fv(geoWorldGeoMatUniformPos, 1, GL_FALSE, (float32*)&worldGeoMat[0]);                     
  
  glDrawArrays(GL_TRIANGLES, 0, 3);
  shaderProgramUse(nullptr);
}

bool8 createImageIntegrator(Scene* scene,
                            Sampler* sampler,
                            RayIntegrator* rayIntegrator,
                            Film* film,
                            Camera* camera,
                            ImageIntegrator** outIntegrator)
{

  *outIntegrator = engineAllocObject<ImageIntegrator>(MEMORY_TYPE_GENERAL);
  ImageIntegrator* integrator = *outIntegrator;
  integrator->scene = scene;
  integrator->sampler = sampler;
  integrator->rayIntegrator = rayIntegrator;
  integrator->film = film;
  integrator->camera = camera;
  integrator->pixelGap = uint2(1, 1);
  integrator->initialOffset = uint2(0, 0);
  integrator->internalData = nullptr;

  // Stack SSBO
  glGenBuffers(1, &integrator->stackSSBO);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, integrator->stackSSBO);
  // TODO: We should be able to resize SSBO whenever we want. Now we simply preallocate as maximum
  // as we may want
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(DistancesStack) * 1280 * 720, NULL, GL_DYNAMIC_COPY);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, STACKS_SSBO_BINDING, integrator->stackSSBO);  

  // Global parameters UBO
  glGenBuffers(1, &integrator->globalParamsUBO);
  glBindBuffer(GL_UNIFORM_BUFFER, integrator->globalParamsUBO);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(GlobalParameters), NULL, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
  glBindBufferBase(GL_UNIFORM_BUFFER, GLOBAL_PARAMS_UBO_BINDING, integrator->globalParamsUBO);

  // Ray map texture
  glGenTextures(1, &integrator->rayMapTexture);
  glBindTexture(GL_TEXTURE_2D, integrator->rayMapTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  // TODO: We should be able to resize raymap texture
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1280, 720, 0, GL_RGBA, GL_FLOAT, NULL);
  glBindTexture(GL_TEXTURE_2D, 0);

  // Ray map init framebuffer
  glGenFramebuffers(1, &integrator->rayMapInitFramebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, integrator->rayMapInitFramebuffer);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, integrator->rayMapTexture, 0);
  // TODO: attach stencil too
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  {
    return FALSE;
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // Prepare frame program
  createShaderProgram(&integrator->prepareFrameProgram);
  
  shaderProgramAttachShader(integrator->prepareFrameProgram,
                            shaderManagerGetShader("triangle.vert"));
  
  shaderProgramAttachShader(integrator->prepareFrameProgram,
                            shaderManagerLoadShader(GL_FRAGMENT_SHADER, "shaders/frame_preparer.frag"));

  if(linkShaderProgram(integrator->prepareFrameProgram) == FALSE)
  {
    return FALSE;
  }

  // Rays mover program
  createShaderProgram(&integrator->raysMoverProgram);

  shaderProgramAttachShader(integrator->raysMoverProgram,
                            shaderManagerGetShader("triangle.vert"));

  shaderProgramAttachShader(integrator->raysMoverProgram,
                            shaderManagerLoadShader(GL_FRAGMENT_SHADER, "shaders/rays_mover.frag"));

  if(linkShaderProgram(integrator->raysMoverProgram) == FALSE)
  {
    return FALSE;
  }
  
  return TRUE;
}

void destroyImageIntegrator(ImageIntegrator* integrator)
{
  engineFreeObject(integrator, MEMORY_TYPE_GENERAL);
  glDeleteBuffers(1, &integrator->stackSSBO);
  glDeleteBuffers(1, &integrator->globalParamsUBO);
  glDeleteTextures(1, &integrator->rayMapTexture);
  glDeleteFramebuffers(1, &integrator->rayMapInitFramebuffer);
  destroyShaderProgram(integrator->prepareFrameProgram);
}

void imageIntegratorExecute(ImageIntegrator* integrator, float32 time)
{
  imageIntegratorSetupGlobalParameters(integrator, time);

  GLint previousViewport[4] = {};
  glGetIntegerv(GL_VIEWPORT, previousViewport);
  glViewport(0, 0, integrator->parameters.resolution.x, integrator->parameters.resolution.y);
  
  // Prepare ray map texture and clear stacks
  shaderProgramUse(integrator->prepareFrameProgram);
  glBindFramebuffer(GL_FRAMEBUFFER, integrator->rayMapInitFramebuffer);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  shaderProgramUse(nullptr);
  
  // TODO: generate a stencil mask (determines which pixels to render) based on imageIntegrator's function


  // Traverse scene inorder, render objects
  const uint32 MAX_ITERATIONS = 8;
  std::vector<AssetPtr>& sceneGeometry = sceneGetGeometry(integrator->scene);
  for(uint32 i = 0; i < MAX_ITERATIONS; i++)
  {
    for(AssetPtr geometry: sceneGeometry)
    {
      drawGeometryInorder(geometry);
    }

    glEnable(GL_BLEND);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ONE, GL_ONE);
    glBindFramebuffer(GL_FRAMEBUFFER, integrator->rayMapInitFramebuffer);
    shaderProgramUse(integrator->raysMoverProgram);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    shaderProgramUse(nullptr);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_BLEND);
  }

  // translate distances of stacks into colors

  glViewport(previousViewport[0], previousViewport[1], previousViewport[2], previousViewport[3]);
}

void imageIntegratorExecute2(ImageIntegrator* integrator, float32 time)
{
  imageIntegratorSetupCommonScriptData(integrator, time);
  
  uint2 filmSize = filmGetSize(integrator->film);
  for(int32 y = 0; y < filmSize.y; y++)
  {
    for(int32 x = 0; x < filmSize.x; x++)
    {
      int2 pixelLocation(x, y);

      float3 radiance(0.0f, 0.0f, 0.0f);
      // TODO: imageIntegratorShouldIntegratePixelLocation's function should be integrated in shader
      if(imageIntegratorShouldIntegratePixelLocation(integrator, pixelLocation))
      {
        samplerStartSamplingPixel(integrator->sampler, pixelLocation);

        Sample sample;
        while(samplerGenerateSample(integrator->sampler, sample))
        {
          Ray viewRay = cameraGenerateWorldRay(integrator->camera, sample.ndc);
          
          float3 sampleRadiance = rayIntegratorCalculateRadiance(integrator->rayIntegrator,
                                                                 viewRay,
                                                                 integrator->scene,
                                                                 time);

          radiance += sampleRadiance * sample.weight;
        }
      }

      filmSetPixel(integrator->film, pixelLocation, radiance);
    }
  }
}

void imageIntegratorSetSize(ImageIntegrator* integrator, uint2 size)
{
  filmResize(integrator->film, size);
  cameraSetAspectRatio(integrator->camera, float32(size.x) / float32(size.y));
  samplerSetSampleAreaSize(integrator->sampler, size);
}

void imageIntegratorSetScene(ImageIntegrator* integrator, Scene* scene)
{
  integrator->scene = scene;
}

Scene* imageIntegratorGetScene(ImageIntegrator* integrator)
{
  return integrator->scene;
}
 
void imageIntegratorSetSampler(ImageIntegrator* integrator, Sampler* sampler)
{
  integrator->sampler = sampler;
}

Sampler* imageIntegratorGetSampler(ImageIntegrator* integrator)
{
  return integrator->sampler;
}

void imageIntegratorSetRayIntegrator(ImageIntegrator* integrator, RayIntegrator* rayIntegrator)
{
  integrator->rayIntegrator = rayIntegrator;
}

RayIntegrator* imageIntegratorGetRayIntegrator(ImageIntegrator* integrator)
{
  return integrator->rayIntegrator;
}

void imageIntegratorSetFilm(ImageIntegrator* integrator, Film* film)
{
  integrator->film = film;
}

Film* imageIntegratorGetFilm(ImageIntegrator* integrator)
{
  return integrator->film;
}
 
void imageIntegratorSetCamera(ImageIntegrator* integrator, Camera* camera)
{
  integrator->camera = camera;
}

Camera* imageIntegratorGetCamera(ImageIntegrator* integrator)
{
  return integrator->camera;
}

void imageIntegratorSetPixelGap(ImageIntegrator* integrator, uint2 gap)
{
  integrator->pixelGap = gap + uint2(1, 1);
}

uint2 imageIntegratorGetPixelGap(ImageIntegrator* integrator)
{
  return integrator->pixelGap - uint2(1, 1);  
}

void imageIntegratorSetInitialOffset(ImageIntegrator* integrator, uint2 offset)
{
  integrator->initialOffset = offset;
}

uint2 imageIntegratorGetInitialOffset(ImageIntegrator* integrator)
{
  return integrator->initialOffset;
}

void imageIntegratorSetInternalData(ImageIntegrator* integrator, void* internalData)
{
  integrator->internalData = internalData;
}

void* imageIntegratorGetInternalData(ImageIntegrator* integrator)
{
  return integrator->internalData;
}
