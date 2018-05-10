#include "structs.h"
#include <tchar.h>
#include <windows.h>

#define EOF (-1)

#ifdef __cplusplus // If used by C++ code,
extern "C" {       // we need to export the C interface
#endif

__declspec(dllexport) BOOL initMemAndSync(ControlData *data) {
  data->hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
                                     0, sizeof(Game), TEXT("phoenixShm"));
  if (data->hMapFile == NULL) {
    _tprintf(TEXT("Erro na memoria partilhada (%d).\n"), GetLastError());
    return FALSE;
  }

  data->hMutex = CreateMutex(NULL, FALSE, TEXT("phoenixMutex"));
  if (data->hMutex == NULL) {
    _tprintf(TEXT("Erro no mutex (%d).\n"), GetLastError());
    return FALSE;
  }

  return TRUE;
}

__declspec(dllexport) void writeData(ControlData *data, Game *game) {
  WaitForSingleObject(data->hMutex, INFINITE);
  CopyMemory(data->game, game, sizeof(Game));
  ReleaseMutex(data->hMutex);
}

__declspec(dllexport) void readData(ControlData *data, Game *game) {
  WaitForSingleObject(data->hMutex, INFINITE);
  CopyMemory(game, data->game, sizeof(Game));
  ReleaseMutex(data->hMutex);
}

#ifdef __cplusplus
}
#endif
