#include "ui_styles.h"

const static ImColor mapLogMessageTypeToColor[] =
{
  LogErrorClr,
  LogWarningClr,
  LogVerboseClr,
  LogInfoClr,
  LogSuccessClr
};

ImColor logTypeToClr(LogMessageType type)
{
  return mapLogMessageTypeToColor[type];
}
