#pragma once

#include "defines.h"

ENGINE_API GLuint createFramebuffer(GLuint colorTextureHandle);
ENGINE_API GLuint createFramebuffer(GLuint colorTextureHandle0, GLuint colorTextureHandle1);

ENGINE_API GLuint createFramebufferD(GLuint colorTextureHandle, GLuint dTextureHandle);
ENGINE_API GLuint createFramebufferD(GLuint colorTextureHandle0, GLuint colorTextureHandle1, GLuint dTextureHandle);

ENGINE_API GLuint createFramebufferDS(GLuint colorTextureHandle, GLuint dsTextureHandle);
ENGINE_API GLuint createFramebufferDS(GLuint colorTextureHandle0, GLuint colorTextureHandle1, GLuint dsTextureHandle);


ENGINE_API void pushViewport(GLint x, GLint y, GLint width, GLint height);
ENGINE_API bool8 popViewport();

ENGINE_API void pushBlendEquation(GLenum modeRGB, GLenum modeAlpha);
ENGINE_API bool8 popBlendEquation();

ENGINE_API void pushBlendFunction(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
ENGINE_API bool8 popBlendFunction();

ENGINE_API void pushBlend(GLenum modeRGB, GLenum modeAlpha,
                          GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
ENGINE_API bool8 popBlend();

ENGINE_API void drawTriangleNoVAO();


