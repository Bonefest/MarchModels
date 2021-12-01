#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <image_integrator.h>

// ----------------------------------------------------------------------------
// Mocking sampler
// ----------------------------------------------------------------------------
struct MockedSampler
{
  uint32 destroyRefCount = 0;
  uint32 startSamplingPixelRefCount = 0;
  uint32 generateSamplePixelRefCount = 0;
};

void mockedSamplerDestroy(Sampler* sampler)
{
  MockedSampler* data = (MockedSampler*)samplerGetInternalData(sampler);
  data->destroyRefCount++;
}

void mockedSamplerStartSamplingPixel(Sampler* sampler, int2 location)
{
  MockedSampler* data = (MockedSampler*)samplerGetInternalData(sampler);
  data->startSamplingPixelRefCount++;
}

bool8 mockedSamplerGenerateSampler(Sampler* sampler, Sample& outSample)
{
  MockedSampler* data = (MockedSampler*)samplerGetInternalData(sampler);
  data->generateSamplePixelRefCount++;
  return data->generateSamplePixelRefCount > 0 ? FALSE : TRUE;
}

Sampler* createMockedSampler()
{
  SamplerInterface interface = {};
  interface.destroy = mockedSamplerDestroy;
  interface.startSamplingPixel = mockedSamplerStartSamplingPixel;
  interface.generateSample = mockedSamplerGenerateSampler;

  Sampler* result = nullptr;
  allocateSampler(interface, uint2(), &result);

  MockedSampler* data = engineAllocObject<MockedSampler>(MEMORY_TYPE_GENERAL);
  samplerSetInternalData(result, data);
  
  return result;
}

void removeMockedSampler(Sampler* sampler)
{
  MockedSampler* data = (MockedSampler*)samplerGetInternalData(sampler);
  engineFreeObject(data, MEMORY_TYPE_GENERAL);
  
  destroySampler(sampler);
}

// ----------------------------------------------------------------------------
// Mocking ray integrator
// ----------------------------------------------------------------------------

struct MockedRayIntegrator
{
  uint32 destroyRefCount = 0;
  uint32 calculateRadianceRefCount = 0;
};

void mockedRayIntegratorDestroy(RayIntegrator* integrator)
{
  MockedRayIntegrator* data = (MockedRayIntegrator*)rayIntegratorGetInternalData(integrator);
  data->destroyRefCount++;
}

float3 mockedRayIntegratorCalculateRadiance(RayIntegrator* integrator, Ray viewRay, Scene* scene, float32 time)
{
  MockedRayIntegrator* data = (MockedRayIntegrator*)rayIntegratorGetInternalData(integrator);
  data->calculateRadianceRefCount++;

  return float3();
}

RayIntegrator* createMockedRayIntegrator()
{
  RayIntegratorInterface interface = {};
  interface.destroy = mockedRayIntegratorDestroy;
  interface.calculateRadiance = mockedRayIntegratorCalculateRadiance;

  RayIntegrator* integrator = nullptr;
  allocateRayIntegrator(interface, &integrator);

  MockedRayIntegrator* data = engineAllocObject<MockedRayIntegrator>(MEMORY_TYPE_GENERAL);
  rayIntegratorSetInternalData(integrator, data);

  return integrator;
}

void removeMockedRayIntegrator(RayIntegrator* integrator)
{
  MockedRayIntegrator* data = (MockedRayIntegrator*)rayIntegratorGetInternalData(integrator);  
  destroyRayIntegrator(integrator);  
  engineFreeObject(data, MEMORY_TYPE_GENERAL);
}

class ImageIntegratorTests: public ::testing::Test
{
protected:
  void SetUp() override
  {
    sampler = createMockedSampler();
    rayIntegrator = createMockedRayIntegrator();
    createPerspectiveCamera(1.0f, toRad(45.0f), 0.01f, 100.0f, &camera);
    createFilm(uint2(8, 8), &film);
    createScene(&scene);
    createImageIntegrator(nullptr, sampler, rayIntegrator, film, camera, &imageIntegrator);
  }

  void TearDown() override
  {
    if(imageIntegrator != nullptr)
    {
      destroyImageIntegrator(imageIntegrator);
    }
    
    destroyScene(scene);
    destroyCamera(camera);
    destroyFilm(film);
  }

  ImageIntegrator* imageIntegrator;
  Scene* scene;
  Sampler* sampler;
  RayIntegrator* rayIntegrator;
  Film* film;
  Camera* camera;
};

TEST_F(ImageIntegratorTests, StoredObjectsNotDestroyed)
{
  destroyImageIntegrator(imageIntegrator);
  imageIntegrator = nullptr;

  MockedSampler* mockedSampler = (MockedSampler*)samplerGetInternalData(sampler);
  MockedRayIntegrator* mockedRayIntegrator = (MockedRayIntegrator*)rayIntegratorGetInternalData(rayIntegrator);

  EXPECT_EQ(mockedSampler->destroyRefCount, 0);
  EXPECT_EQ(mockedRayIntegrator->destroyRefCount, 0);
}

TEST_F(ImageIntegratorTests, ResizeAffectsSampler)
{
  imageIntegratorSetSize(imageIntegrator, uint2(12, 12));

  MockedSampler* mockedSampler = (MockedSampler*)samplerGetInternalData(sampler);
  EXPECT_EQ(mockedSampler->startSamplingPixelRefCount, 0);
}

TEST_F(ImageIntegratorTests, ExecuteUsesSamplerAndIntegrator)
{
  imageIntegratorExecute(imageIntegrator);

  MockedSampler* mockedSampler = (MockedSampler*)samplerGetInternalData(sampler);
  MockedRayIntegrator* mockedRayIntegrator = (MockedRayIntegrator*)rayIntegratorGetInternalData(rayIntegrator);  
  EXPECT_GT(mockedSampler->startSamplingPixelRefCount, 0);
  EXPECT_GT(mockedSampler->generateSamplePixelRefCount, 0);
}

