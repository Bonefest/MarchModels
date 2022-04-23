#include common.glsl

#define HALF_PI 1.5707963f
#define PI 3.1415926f
#define PI2 6.2831852f

// ----------------------------------------------------------------------------
// Projections
// ----------------------------------------------------------------------------

float32 saturate(float32 value)
{
  return clamp(value, 0.0f, 1.0f);
}

float4 sampleTriplanar(sampler2D text, float3 p, float3 n, float2 uvMin, float2 uvSize, float32 factor)
{
  p = fract(p);
  
  float4 x = texture(text, uvSize * p.yz + uvMin);
  float4 y = texture(text, uvSize * p.zx + uvMin);
  float4 z = texture(text, uvSize * p.xy + uvMin);

  float3 w = pow(n, factor.xxx);
  
  return (x * w.x + y * w.y + z * w.z) / (w.x + w.y + w.z);
}

float4 sampleSpherical(sampler2D text, float3 p, float2 uvMin, float2 uvSize)
{
  float3 np = normalize(p);
  
  float32 rotationAngle = atan(np.z, np.x);
  float32 elevationAngle = asin(np.y);  

  float2 uv = float2(saturate((rotationAngle + PI) / PI2),
                     saturate((elevationAngle + HALF_PI) / PI));
  
  return texture(text, uvSize * uv + uvMin);
}

float4 sampleCylindrical(sampler2D text, float3 p, float2 uvMin, float2 uvSize)
{
  float2 np = normalize(p.xz);

  float32 rotationAngle = atan(np.y, np.x);

  float2 uv = float2(saturate((rotationAngle + PI) / PI2), fract(p.y));
  return texture(text, uvSize * uv + uvMin);
}

float4 psample(sampler2D text, float3 p, float3 n, MaterialTextureParameters params, uint32 mode)
{
  if(params.enabled == TRUE)
  {
  
    float2 uvSize = params.uvRect.zw - params.uvRect.xy;
    float2 uvMin = params.uvRect.xy;

    switch(mode)
    {
      case MATERIAL_TEXTURE_PROJECTION_MODE_TRIPLANAR: return sampleTriplanar(text, p, n, uvMin, uvSize, params.blendingFactor);
      case MATERIAL_TEXTURE_PROJECTION_MODE_SPHERICAL: return sampleSpherical(text, p, uvMin, uvSize);
      case MATERIAL_TEXTURE_PROJECTION_MODE_CYLINDRICAL: return sampleCylindrical(text, p, uvMin, uvSize);
    }

    return 0.0f.xxxx;
  }
  else
  {
    return params.defaultValue;
  }
}

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
  return clamp(2.0f / ((roughness + 0.001) * (roughness + 0.001)) - 2, 1.0f, 10000.0f);
}

float3 iorToF0(float32 ior)
{
  float32 d = (ior - 1) / (ior + 1);
  return (d * d).xxx;
}

float32 ggxNDF(float3 n, float3 h, float32 roughness)
{
  float32 r2 = roughness * roughness;
  float32 NoH = max(dot(n, h), 0.0f);
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
  float32 NoL = max(dot(n, l), 0.0f);
  float32 NoV = max(dot(n, v), 0.0f);
  
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
  float3 radiance = ambientColor;
  float32 shininess = roughnessToShininess(roughness);
  
  for(uint32 i = 0; i < lightsCount; i++)
  {
    float32 lLen = 1.0f;
    float3 l = getLightDirection(i, pWorld, lLen);
    float32 NoL = max(dot(n, l), 0.0f);

    float3 h = normalize(l + v);
    float32 NoH = max(dot(n, h), 0.0f);
    
    float32 attenuation = getAttenuation(i, lLen, l);
    float32 shadow = shadows[i % 4];
    
    float3 irradiance = lightParams[i].intensity.rgb * attenuation * shadow;
    
    radiance += lambertBRDF(diffuseColor, irradiance, NoL) + blinnPhongBRDF(irradiance, NoL, NoH, shininess);
  }

  radiance *= (1.0f - ao);

  return radiance;
}
