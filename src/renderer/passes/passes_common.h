#pragma once

#include <camera.h>
#include <shader_program.h>
#include <assets/geometry.h>

ShaderProgram* createAndLinkTriangleShadingProgram(const char* fragmentShaderPath);

/**
 * @return boolean value which indicates whether it was rendered or not
 */
bool8 drawGeometryPostorder(Camera* camera,
                            AssetPtr geometry,
                            uint32 indexInBranch,
                            uint32 culledSiblingsCount,
                            uint32& culledObjCounter,
                            bool8 shadowPath = FALSE);
