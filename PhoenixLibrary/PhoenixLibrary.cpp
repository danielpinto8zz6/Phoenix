// PhoenixLibrary.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include "PhoenixLibrary.h"

BOOL initMemAndSync(ControlData *data) {
  data->hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
                                     0, sizeof(Game), TEXT("phoenixShm"));
  if (data->hMapFile == NULL) {
    Error(TEXT("Inicializing shared memory"));
    return FALSE;
  }

  data->hMutex = CreateMutex(NULL, FALSE, TEXT("phoenixMutex"));
  if (data->hMutex == NULL) {
    Error(TEXT("Creating phoenix mutex"));
    return FALSE;
  }

  return TRUE;
}

void writeData(ControlData *data, Game *game) {
  WaitForSingleObject(data->hMutex, INFINITE);
  data->game->num++;
  CopyMemory(data->game, game, sizeof(Game));
  ReleaseMutex(data->hMutex);
}

void readData(ControlData *data, Game *game) {
  WaitForSingleObject(data->hMutex, INFINITE);
  CopyMemory(game, data->game, sizeof(Game));
  ReleaseMutex(data->hMutex);
}

unsigned peekData(ControlData *data) {
  unsigned num;
  WaitForSingleObject(data->hMutex, INFINITE);
  num = data->game->num;
  ReleaseMutex(data->hMutex);
  return num;
}

BOOL initSemaphores(ControlData *data) {
  data->smWrite =
      CreateSemaphore(NULL, MAX_SEM_COUNT, MAX_SEM_COUNT, smWriteName);
  if (data->smWrite == NULL) {
    Error(TEXT("Initializing write semaphore"));
    return FALSE;
  }

  data->smRead = CreateSemaphore(NULL, 0, MAX_SEM_COUNT, smReadName);
  if (data->smRead == NULL) {
    Error(TEXT("Initializing read semaphore"));
    return FALSE;
  }

  return TRUE;
}

void Error(const TCHAR *text) {
  _tprintf(TEXT("[ERROR] %s. (%d)"), text, GetLastError());
}
