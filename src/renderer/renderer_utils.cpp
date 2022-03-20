#include <array>
#include <stack>
using std::array;
using std::stack;

#include "renderer.h"
#include "renderer_utils.h"

using ViewportData = array<GLint, 4>;
using BlendEqData = array<GLint, 2>;
using BlendFuncData = array<GLint, 4>;

struct UtilsData
{
  stack<ViewportData> viewports;
  stack<BlendEqData> blendEqData;
  stack<BlendFuncData> blendFuncData;
};

static UtilsData data;

static GLuint createFramebuffer(GLuint* colorTextureHandles, uint32 count,
                                GLuint dTextureHandle = 0,                                
                                GLuint dsTextureHandle = 0)

{
  GLuint framebuffer = 0;
  
  glGenFramebuffers(1, &framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

  for(uint32 i = 0; i < count; i++)
  {
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, colorTextureHandles[i], 0);
  }

  bool8 dsTextureIsPassed = dsTextureHandle != 0 ? TRUE : FALSE;
  bool8 dTextureIsPassed = dTextureHandle != 0 ? TRUE : FALSE;
  if(dsTextureIsPassed == TRUE)
  {
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, dsTextureHandle, 0);
  }
  else if(dTextureIsPassed == TRUE)
  {
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, dTextureHandle, 0);
  }

  // NOTE: If color attachments are more than 1, then we should explicitly
  // tell that each of the attachments is a draw buffer
  if(count > 1)
  {
    GLenum drawBuffers[count];
    for(uint32 i = 0; i < count; i++)
    {
      // i-th output of shader will be written into i-th texture
      drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
    }

    glDrawBuffers(count, drawBuffers);
  }
  
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  {
    return 0;
  }
  
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return framebuffer;  
}

GLuint createFramebuffer(GLuint colorTextureHandle)
{
  GLuint handles[] = {colorTextureHandle};
  return createFramebuffer(handles, 1);
}

GLuint createFramebuffer(GLuint colorTextureHandle0, GLuint colorTextureHandle1)
{
  GLuint handles[] = {colorTextureHandle0, colorTextureHandle1};
  return createFramebuffer(handles, 2);
}

GLuint createFramebufferD(GLuint colorTextureHandle, GLuint dTextureHandle)
{
  GLuint handles[] = {colorTextureHandle};
  return createFramebuffer(handles, 1, dTextureHandle);
}

GLuint createFramebufferD(GLuint colorTextureHandle0, GLuint colorTextureHandle1, GLuint dTextureHandle)
{
  GLuint handles[] = {colorTextureHandle0, colorTextureHandle1};
  return createFramebuffer(handles, 2, dTextureHandle);
}

GLuint createFramebufferDS(GLuint colorTextureHandle, GLuint dsTextureHandle)
{
  GLuint handles[] = {colorTextureHandle};
  return createFramebuffer(handles, 1, 0, dsTextureHandle);
}

GLuint createFramebufferDS(GLuint colorTextureHandle0, GLuint colorTextureHandle1, GLuint dsTextureHandle)
{
  GLuint handles[] = {colorTextureHandle0, colorTextureHandle1};
  return createFramebuffer(handles, 2, 0, dsTextureHandle);
}

void pushViewport(GLint x, GLint y, GLint width, GLint height)
{
  ViewportData viewport;
  glGetIntegerv(GL_VIEWPORT, &viewport[0]);
  data.viewports.push(viewport);

  glViewport(x, y, width, height);
}

bool8 popViewport()
{
  if(data.viewports.empty())
  {
    return FALSE;
  }

  ViewportData viewport = data.viewports.top();
  glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
  
  data.viewports.pop();
  return TRUE;
}

void pushBlendEquation(GLenum modeRGB, GLenum modeAlpha)
{
  BlendEqData eqData;
  glGetIntegerv(GL_BLEND_EQUATION_RGB, &eqData[0]);
  glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &eqData[1]);

  data.blendEqData.push(eqData);

  glBlendEquationSeparate(modeRGB, modeAlpha);
}

bool8 popBlendEquation()
{
  if(data.blendEqData.empty())
  {
    return FALSE;
  }

  BlendEqData eqData = data.blendEqData.top();

  glBlendEquationSeparate(eqData[0], eqData[1]);  

  data.blendEqData.pop();
  
  return TRUE;
}

void pushBlendFunction(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
  BlendFuncData funcData;
  glGetIntegerv(GL_BLEND_SRC_RGB, &funcData[0]);
  glGetIntegerv(GL_BLEND_DST_RGB, &funcData[1]);
  glGetIntegerv(GL_BLEND_SRC_ALPHA, &funcData[2]);
  glGetIntegerv(GL_BLEND_DST_ALPHA, &funcData[3]);
  data.blendFuncData.push(funcData);

  glBlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
}

bool8 popBlendFunction()
{
  if(data.blendFuncData.empty())
  {
    return FALSE;
  }

  BlendFuncData funcData = data.blendFuncData.top();

  glBlendFuncSeparate(funcData[0], funcData[1], funcData[2], funcData[3]);

  data.blendFuncData.pop();
  
  return TRUE;
}

void pushBlend(GLenum modeRGB, GLenum modeAlpha,
               GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
  pushBlendEquation(modeRGB, modeAlpha);
  pushBlendFunction(srcRGB, dstRGB, srcAlpha, dstAlpha);
}

bool8 popBlend()
{
  return popBlendEquation() == TRUE && popBlendFunction() == TRUE;
}

void drawTriangleNoVAO()
{
  glBindVertexArray(rendererGetResourceHandle(RR_EMPTY_VAO));
  glDrawArrays(GL_TRIANGLES, 0, 3);  
}
