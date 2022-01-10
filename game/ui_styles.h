#pragma once

#include <imgui/imgui.h>

#include <logging.h>

#define LogErrorClr ImColor(192, 0, 0, 255)
#define LogWarningClr ImColor(192, 192, 0, 255)
#define LogVerboseClr ImColor(192, 0, 192, 255)
#define LogInfoClr ImColor(66, 135, 246, 255)
#define LogSuccessClr ImColor(23, 217, 11, 255)

#define DeleteClr ImColor(160, 0, 0, 255)
#define BrightDeleteClr ImColor(200, 0, 0, 255)
#define DarkDeleteClr ImColor(80, 0, 0, 255)

#define NewClr ImColor(127, 127, 160, 255)
#define BrightNewColr ImColor(159, 159, 200, 255)
#define DarkNewClr ImColor(63, 63, 80, 255)

#define PrimaryClr ImColor(2, 117, 216, 255)
#define BrightPrimaryClr ImColor(4, 130, 240, 255)
#define DarkPrimaryClr ImColor(1, 58, 108, 255)

#define SecondaryClr ImColor(192, 192, 192, 255)
#define BrightSecondaryClr ImColor(230, 230, 230, 255)
#define DarkSecondaryClr ImColor(128, 128, 128, 255)

#define SuccessClr ImColor(92, 184, 92, 255)
#define BrightSuccessClr ImColor(115, 230, 115, 255)
#define DarkSuccessClr ImColor(46, 92, 46, 255)

#define InfoClr ImColor(91, 192, 222, 255)
#define BrightInfoClr ImColor(114, 220, 245, 255)
#define DarkInfoClr ImColor(45, 96, 111, 255)

#define WarningClr ImColor(240, 173, 78, 255)
#define BrightWarningClr ImColor(255, 217, 97, 255)
#define DarkWarningClr ImColor(120, 86, 39, 255)

#define DangerClr ImColor(217, 83, 79, 255)
#define BrightDangerClr ImColor(240, 91, 87, 255)
#define DarkDangerClr ImColor(108, 41, 39, 255)

#define InverseClr ImColor(41, 43, 44, 255)
#define BrightInverseClr ImColor(51, 54, 55, 255)
#define DarkInverseClr ImColor(20, 21, 22, 255)

#define FadedClr ImColor(247, 247, 247, 255)
#define BrightFadedClr ImColor(255, 255, 255, 255)
#define DarkFadedClr ImColor(200, 200, 200, 255)

ImColor logTypeToClr(LogMessageType type);
