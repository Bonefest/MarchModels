#include <memory_manager.h>

#include "view.h"

void destroyView(View* view)
{
  engineFreeMem(view, MEMORY_TYPE_GENERAL);
}
