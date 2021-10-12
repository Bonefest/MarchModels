#if defined(ENABLE_EDITOR_GAME)

#include <game_framework.h>

#include "editor.h"


static bool8 extractSetupConfig(Application* app,
                                uint32* outScreenWidth,
                                uint32* outScreenHeight,
                                const char** outName)
{
  *outScreenWidth = 1280;
  *outScreenHeight = 720;
  *outName = "Editor";

  return TRUE;
}


bool8 initializeGameFramework(GameFramework* outFramework)
{
  outFramework->extractSetupConfig = extractSetupConfig;
  outFramework->initialize = initEditor;
  outFramework->shutdown = shutdownEditor;
  outFramework->update = updateEditor;
  outFramework->draw = drawEditor;
  outFramework->processInput = processInputEditor;

  return TRUE;
}


#endif
