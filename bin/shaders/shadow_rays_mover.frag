#version 450 core
#extension GL_ARB_shader_stencil_export : require

#include stack.glsl

out float4 outCameraRay;

uniform sampler2D depthMap;
uniform uint32 curIterIdx;
uniform uint32 lightIndex;

void main()
{
    int2 ifragCoord = int2(gl_FragCoord.x, gl_FragCoord.y);
    
    float32 distance = stackFront(ifragCoord).distance;
    float32 totalDistance = stackGetTotalDistance(ifragCoord);
    
    // If totalDistance is larger than INF_DISTANCE, then we know that this
    // pixel was already processed previously --> do not add distance from
    // the stack, simply quit    
    if(totalDistance >= INF_DISTANCE)
    {
      gl_FragStencilRefARB = 0;
      outCameraRay = float4(0.0f);
    }
    // If shadow ray's intersected something - discard it
    else if(distance < INT_DISTANCE)
    {
      gl_FragStencilRefARB = 0;
      outCameraRay = float4(0.0f, 0.0f, 0.0f, distance);
      
      stackAddTotalDistance(ifragCoord, INF_DISTANCE);
      stackClearSize(ifragCoord);
    }
    else
    {
      float32 movedDistance = totalDistance + distance;
      float32 distanceToLight = INF_DISTANCE;
      
      if(lightParams[lightIndex].type != LIGHT_SOURCE_TYPE_DIRECTIONAL)
      {
        float2 uv = fragCoordToUV(gl_FragCoord.xy);
        float3 worldPos = getWorldPos(uv, ifragCoord, depthMap);
      
        distanceToLight = distance2(worldPos, lightParams[lightIndex].position.xyz);
      }

      // If shadow ray's moved behind the light source, then it's surely's intersected it,
      // thus we can discard it.
      gl_FragStencilRefARB = movedDistance * movedDistance >= distanceToLight ? 0 : 1;
      outCameraRay = float4(0.0f, 0.0f, 0.0f, distance);
      
      stackAddTotalDistance(ifragCoord, distance);
      stackClearSize(ifragCoord);
    }

}

