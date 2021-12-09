#include <cstdio>

#include "memory_manager.h"

#include "fileio.h"

bool8 readWholeFile(const char* filename, uint32* outFileSize, char** outFileContent)
{
  FILE* file = fopen(filename, "r");
  if(file == NULL)
  {
    return FALSE;
  }
  
  fseek(file, 0, SEEK_END);
  uint32 fileSize = ftell(file);
  fseek(file, 0, SEEK_SET);

  char* fileContent = (char*)engineAllocMem(fileSize + 1, MEMORY_TYPE_GENERAL);
  fread(fileContent, fileSize, 1, file);

  *outFileSize = fileSize + 1;
  *outFileContent= fileContent;
  
  return TRUE;
}

void freeFileContent(uint32 fileSize, char* fileContent)
{
  engineFreeMem(fileContent, fileSize);
}
