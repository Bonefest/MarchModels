#version 450 core

#include common.glsl

layout(location = 0) out float3 outColor;
layout(location = 0) uniform sampler2D LDRMap;

void main()
{
    int2 ifragCoord = int2(gl_FragCoord.x, gl_FragCoord.y);
    
    if(ifragCoord.x % (params.pixelGapX + 1) == 0 && ifragCoord.y % (params.pixelGapY + 1) == 0)
    {
      int2 noGapCoord = int2(ifragCoord.x / (params.pixelGapX + 1),
                             ifragCoord.y / (params.pixelGapY + 1));

      if(all(lessThan(noGapCoord, params.gapResolution)))
      {
        outColor = gammaEncode(texelFetch(LDRMap, noGapCoord, 0).rgb);
      }
      else
      {
        outColor = float3(0.0, 0.0, 0.0);
      }
    }
    else
    {
      outColor = float3(0.0, 0.0, 0.0);
    }
}
