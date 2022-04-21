#include common.glsl

#define PI 3.1415926f

// ----------------------------------------------------------------------------
// Helpers
// ----------------------------------------------------------------------------

float3 getLightDirection(uint32 lightIndex, float3 p, inout float32 lpDistance)
{
  if(lightParams[lightIndex].type == LIGHT_SOURCE_TYPE_DIRECTIONAL)
  {
    return -lightParams[lightIndex].forward;
  }

  lpDistance = distance(p, lightParams[lightIndex].position.xyz);
  return (lightParams[lightIndex].position.xyz - p) / lpDistance;
}

float32 getAttenuation(uint32 lightIndex, float32 lpDistance, float3 l)
{
  float2 attFactors = lightParams[lightIndex].attenuationDistanceFactors;
  float32 attenuation = 1.0f;
  1.0f / (attFactors.x * lpDistance + attFactors.y * lpDistance * lpDistance);

  if(lightParams[lightIndex].type == LIGHT_SOURCE_TYPE_SPOT)
  {
    float2 angFactors = lightParams[lightIndex].attenuationAngleFactors;
    float32 t = max(dot(-l, lightParams[lightIndex].forward.xyz), 0.0);

    attenuation *= mix(0.0f, 1.0f, clamp((t - angFactors.y) / (angFactors.x - angFactors.y), 0, 1));
  }

  return attenuation;
}

float32 roughnessToShininess(float32 roughness)
{
  return clamp(2.0f / (roughness * roughness) - 2, 0.0f, 10000.0f);
}

float3 iorToF0(float32 ior)
{
  float32 d = (ior - 1) / (ior + 1);
  return (d * d).xxx;
}

float32 ggxNDF(float3 n, float3 h, float32 roughness)
{
  float32 r2 = roughness * roughness;
  float32 NoH = dot(n, h);
  float32 factor = (1 + NoH * NoH * (r2 - 1));
  return r2 / (PI * factor * factor);
}

float32 ggxSchlickGeometry(float32 NoV, float32 roughness)
{
  return NoV / (NoV * (1.0 - roughness) + roughness);
}

float32 smithGeometry(float3 n, float3 l, float3 v, float32 roughness)
{
  float32 shadowing = ggxSchlickGeometry(max(dot(n, l), 0.0f), roughness);
  float32 masking = ggxSchlickGeometry(max(dot(n, v), 0.0f), roughness);

  return shadowing * masking;
}

float3 schlickFresnel(float3 F0, float32 NoL)
{
  float32 t = 1 - NoL;
  float32 t2 = t * t;

  return F0 + (1 - F0) * max(t2 * t2 * t, 0.0f);
}

// ----------------------------------------------------------------------------
// BRDFs
// ----------------------------------------------------------------------------

float3 lambertBRDF(float3 diffuse, float3 irradiance, float32 NoL)
{
  return diffuse * irradiance * NoL;
}

float3 blinnPhongBRDF(float3 irradiance, float32 NoL, float32 NoH, float32 shininess)
{
  return irradiance * pow(NoH, shininess) * NoL;
}

float3 cookTorranceBRDF(float3 n,
                        float3 h,
                        float3 v,
                        float3 l,
                        float3 F0,
                        float3 irradiance,
                        float32 roughness)
{
  float32 NoL = dot(n, l);
  float32 NoV = dot(n, v);
  
  float32 NDF = ggxNDF(n, h, roughness);
  float32 Geometry = smithGeometry(n, l, v, roughness);
  float3 Fresnel = schlickFresnel(F0, NoL);

  return (NDF * Geometry * Fresnel / (4 * NoV)) * irradiance;
}

// ----------------------------------------------------------------------------
// Rendering equations
// ----------------------------------------------------------------------------

float3 simplifiedRenderingEquation(float3 pWorld,
                                   float3 n,
                                   float3 v,
                                   float3 ambientColor,
                                   float3 diffuseColor,
                                   float32 roughness,
                                   float32 ao,
                                   float4 shadows,
                                   uint32 lightsCount)
                     
{
  bool isSurface = dot(n, n) > 0.1;

  float3 radiance = ambientColor;
  float32 shininess = roughnessToShininess(roughness);
  
  for(uint32 i = 0; i < lightsCount; i++)
  {
    float32 lLen = 1.0f;
    float3 l = getLightDirection(i, pWorld, lLen);
    float32 NoL = dot(n, l);

    float3 h = normalize(l + v);
    float32 NoH = dot(n, h);
    
    float32 attenuation = getAttenuation(i, lLen, l);
    float32 shadow = shadows[i % 4];
    
    float3 irradiance = lightParams[i].intensity.rgb * attenuation * shadow;
    
    radiance += lambertBRDF(diffuseColor, irradiance, NoL) + blinnPhongBRDF(irradiance, NoL, NoH, shininess);
  }

  radiance *= (1.0f - ao);

  return radiance;
}
