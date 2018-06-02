#pragma once

#include "structs.h"
#include <tchar.h>
#include <windows.h>

#define smReadName TEXT("smReadName")
#define smWriteName TEXT("smWriteName")
#define mReadName TEXT("mRead")
#define mWriteName TEXT("mWrite")

#define MAX_SEM_COUNT 10

#define Buffers 10

#ifdef __cplusplus // If used by C++ code,
extern "C" {       // we need to export the C interface
#endif

__declspec(dllexport) BOOL initMemAndSync(ControlData *data) {
  data->hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
                                     0, sizeof(Game), TEXT("phoenixShm"));
  if (data->hMapFile == NULL) {
    _tprintf(TEXT("[Erro] Criar memÃ³ria partilhada: %d\n"), GetLastError());
    return FALSE;
  }

  data->hMutex = CreateMutex(NULL, FALSE, TEXT("phoenixMutex"));
  if (data->hMutex == NULL) {
    _tprintf(TEXT("[Erro] Criar mutex: %d\n"), GetLastError());
    return FALSE;
  }

  return TRUE;
}

__declspec(dllexport) void writeData(ControlData *data, Game *game) {
  WaitForSingleObject(data->hMutex, INFINITE);
  data->game->num++;
  CopyMemory(data->game, game, sizeof(Game));
  ReleaseMutex(data->hMutex);
}

__declspec(dllexport) void readData(ControlData *data, Game *game) {
  WaitForSingleObject(data->hMutex, INFINITE);
  CopyMemory(game, data->game, sizeof(Game));
  ReleaseMutex(data->hMutex);
}

__declspec(dllexport) unsigned peekData(ControlData *data) {
  unsigned num;
  WaitForSingleObject(data->hMutex, INFINITE);
  num = data->game->num;
  ReleaseMutex(data->hMutex);
  return num;
}

__declspec(dllexport) BOOL initSemaphores(ControlData *data) {
  data->smWrite =
      CreateSemaphore(NULL, MAX_SEM_COUNT, MAX_SEM_COUNT, smWriteName);
  if (data->smWrite == NULL) {
    _tprintf(TEXT("[Erro] Criar semÃ¡foro: %d\n"), GetLastError());
    return FALSE;
  }

  data->smRead = CreateSemaphore(NULL, 0, MAX_SEM_COUNT, smReadName);
  if (data->smRead == NULL) {
    _tprintf(TEXT("[Erro] ao criar semÃ¡foro: %d\n"), GetLastError());
    return FALSE;
  }

  return TRUE;
}

__declspec(dllexport) void Error(const TCHAR *text) {
  _tprintf(TEXT("[ERROR] %s. (%d)"), text, GetLastError());
}

#ifdef __cplusplus
}
#endif
