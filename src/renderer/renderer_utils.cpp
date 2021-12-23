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
