#version 450 core

#include geometry_common.glsl

out float4 outCameraRay;

void main()
{
    int2 ifragCoord = int2(gl_FragCoord.x, gl_FragCoord.y);

    stackClear(ifragCoord);

    // TODO: Now we generate a ray going through a center of a pixel. In future
    // we may want to use some sort of src/samplers/sampler.h
    float2 uv = fragCoordToUV(float2(ifragCoord.x, ifragCoord.y));

    // TODO: Somehow generate stencil mask here (if possible with GL_ARB_shader_stencil_export)
    
    outCameraRay = float4(generateRayDir(uv), 0.0);
}
