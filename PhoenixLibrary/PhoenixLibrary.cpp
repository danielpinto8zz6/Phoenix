// PhoenixLibrary.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include "PhoenixLibrary.h"

VOID Error(CONST TCHAR *text) {
  _tprintf(TEXT("[ERROR] %s. (%d)\n"), text, GetLastError());
}

BOOL writeDataToPipe(LPVOID data, SIZE_T size, HANDLE hPipe, LPDWORD nBytes) {
  BOOL success;

  success = WriteFile(hPipe, data, size, nBytes, NULL);
  if (!success) {
    Error(TEXT("Sending data"));
  }
  return success;
}

BOOL receiveDataFromPipe(LPVOID data, SIZE_T size, HANDLE hPipe,
                         LPDWORD nBytes) {
  BOOL success;

  success = ReadFile(hPipe, data, size, nBytes, NULL);
  if (!success) {
    Error(TEXT("Receiving data"));
  }
  return success;
}

BOOL initMemAndSync(HANDLE *hMapFile, const TCHAR *sharedMemoryName,
                    HANDLE *hMutex, const TCHAR *mutexName) {
  *hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0,
                                sizeof(Game), sharedMemoryName);
  if (*hMapFile == NULL) {
    Error(TEXT("Inicializing shared memory"));
    return FALSE;
  }

  *hMutex = CreateMutex(NULL, FALSE, mutexName);
  if (*hMutex == NULL) {
    Error(TEXT("Creating phoenix mutex"));
    return FALSE;
  }

  return TRUE;
}

VOID writeDataToSharedMemory(LPVOID sharedMemory, LPVOID data, SIZE_T size,
                             HANDLE *hMutex) {
  WaitForSingleObject(*hMutex, INFINITE);
  CopyMemory(sharedMemory, data, size);
  ReleaseMutex(*hMutex);
}

VOID readDataFromSharedMemory(LPVOID sharedMemory, LPVOID data, SIZE_T size,
                              HANDLE *hMutex) {
  WaitForSingleObject(*hMutex, INFINITE);
  CopyMemory(data, sharedMemory, size);
  ReleaseMutex(*hMutex);
}

BOOL initMessageZone(MessageData *messageData) {
  if (!initMemAndSync(&messageData->hMapFile, MESSAGES_SHARED_MEMORY_NAME,
                      &messageData->hMutex, MESSAGES_MUTEX_NAME)) {
    return FALSE;
  }

  messageData->sharedMessage = (Message *)MapViewOfFile(
      messageData->hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Message));

  if (messageData->sharedMessage == NULL) {
    Error(TEXT("Mapping shared memory"));
    return FALSE;
  }

  return TRUE;
}

BOOL initGameZone(GameData *gameData) {
  if (!initMemAndSync(&gameData->hMapFile, GAMEDATA_SHARED_MEMORY_NAME,
                      &gameData->hMutex, GAMEDATA_MUTEX_NAME)) {
    return FALSE;
  }

  gameData->sharedGame = (Game *)MapViewOfFile(
      gameData->hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Game));

  if (gameData->sharedGame == NULL) {
    Error(TEXT("Mapping shared memory"));
    return FALSE;
  }

  return TRUE;
}