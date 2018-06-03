#include "stdafx.h"

#include "SharedMemory.h"
#include "structs.h"

DWORD peekData(GameData *data) {
  DWORD num;
  WaitForSingleObject(data->hMutex, INFINITE);
  num = data->game->num;
  ReleaseMutex(data->hMutex);
  return num;
}