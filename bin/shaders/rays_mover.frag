#version 450 core
#extension GL_ARB_shader_stencil_export : require

#define BIG_CONSTANT 1e5

#include stack.glsl

out float4 outCameraRay;

uniform uint32 curIterIdx;

void main()
{
    int2 ifragCoord = int2(gl_FragCoord.x, gl_FragCoord.y);

    float32 distance = stackFront(ifragCoord).distance;
    float32 totalDistance = stackGetTotalDistance(ifragCoord);

    // If totalDistance is larger than, then we know that this
    // pixel was already processed previously --> do not add
    // distance from the stack, simply quit
    if(totalDistance >= BIG_CONSTANT)
    {
      gl_FragStencilRefARB = 0;
      outCameraRay = float4(0.0f);
    }
    // Distances larger than 0.001 causes "ringing" effect
    // during normals visualization
    else if(totalDistance > 100.0 || distance < 0.001)
    {
      gl_FragStencilRefARB = 0;
      outCameraRay = float4(0.0f, 0.0f, 0.0f, distance);
      stackAddTotalDistance(ifragCoord, BIG_CONSTANT);
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

