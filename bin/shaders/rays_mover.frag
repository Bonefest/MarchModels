#version 450 core
#extension GL_ARB_shader_stencil_export : require

#include stack.glsl

out float4 outCameraRay;

uniform uint32 curIterIdx;

void main()
{
    int2 ifragCoord = int2(gl_FragCoord.x, gl_FragCoord.y);

    float32 distance = stackFront(ifragCoord).distance;
    float32 totalDistance = stackGetTotalDistance(ifragCoord);

    if(totalDistance > 100.0 || distance < params.intersectionThreshold)
    {
      gl_FragStencilRefARB = 0;
      outCameraRay = float4(0.0f);
    }
    else
    {
      gl_FragStencilRefARB = 1;
      
      bool notLastIteration = (curIterIdx + 1 < params.rasterItersMaxCount);    
      if(notLastIteration)
      {
        stackClearSize(ifragCoord);
      }

      stackAddTotalDistance(ifragCoord, distance);

      outCameraRay = float4(0, 0, 0, distance);                
    }

}

