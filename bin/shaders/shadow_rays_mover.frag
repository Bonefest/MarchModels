#version 450 core
#extension GL_ARB_shader_stencil_export : require

#include stack.glsl

out float4 outCameraRay;

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
    // Distances larger than 0.001 cause "ringing" effect
    // during normals visualization
    else if(totalDistance > 100.0 || distance < 0.001)
    {
      gl_FragStencilRefARB = 0;
      outCameraRay = float4(0.0f, 0.0f, 0.0f, distance);
      stackAddTotalDistance(ifragCoord, INF_DISTANCE);
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

