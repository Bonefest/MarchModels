#include <array>
#include <stack>
using std::array;
using std::stack;

#include "renderer_utils.h"

using ViewportData = array<GLint, 4>;

struct UtilsData
{
  stack<ViewportData> viewports;
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

