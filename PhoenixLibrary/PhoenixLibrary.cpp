﻿// PhoenixLibrary.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include "PhoenixLibrary.h"

VOID error(LPCWSTR text, ...) {
  TCHAR msg[1024];
  va_list argptr;
  va_start(argptr, text);
  _stprintf_s(msg, TEXT("[ERROR] %s (%d)\n"), text, GetLastError());
  _vftprintf(stderr, msg, argptr);
  va_end(argptr);
}

VOID debug(LPCWSTR text, ...) {
  TCHAR msg[1024];

  va_list argptr;
  va_start(argptr, text);
  _stprintf_s (msg, TEXT("[DEBUG] %s\n"), text);
  _vftprintf(stdout, msg, argptr);
  va_end(argptr);
}

BOOL writeDataToPipe(LPVOID data, SIZE_T size, HANDLE hPipe, LPDWORD nBytes) {
  BOOL success;

  success = WriteFile(hPipe, data, size, nBytes, NULL);
  if (!success) {
    error(TEXT("Sending data"));
  }
  return success;
}

BOOL receiveDataFromPipe(LPVOID data, SIZE_T size, HANDLE hPipe,
                         LPDWORD nBytes) {
  BOOL success;

  success = ReadFile(hPipe, data, size, nBytes, NULL);
  if (!success) {
    error(TEXT("Receiving data"));
  }
  return success;
}

BOOL initMemAndSync(HANDLE *hMapFile, const LPCWSTR sharedMemoryName,
                    HANDLE *hMutex, LPCWSTR mutexName) {
  *hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0,
                                sizeof(Game), sharedMemoryName);
  if (*hMapFile == NULL) {
    error(TEXT("Initializing shared memory"));
    return FALSE;
  }

  *hMutex = CreateMutex(NULL, FALSE, mutexName);
  if (*hMutex == NULL) {
    error(TEXT("Creating phoenix mutex"));
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
    error(TEXT("Mapping shared memory"));
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
    error(TEXT("Mapping shared memory"));
    return FALSE;
  }

  return TRUE;
}

BOOL isGatewayRunning() {
  HANDLE m_hStartEvent =
      CreateEventW(NULL, FALSE, FALSE, GATEWAY_RUNNING_EVENT);

  if (m_hStartEvent == NULL) {
    CloseHandle(m_hStartEvent);
    return TRUE;
  }

  if (GetLastError() == ERROR_ALREADY_EXISTS) {

    CloseHandle(m_hStartEvent);
    m_hStartEvent = NULL;
    return TRUE;
  }
  return FALSE;
}

BOOL isServerRunning() {
  HANDLE m_hStartEvent = CreateEventW(NULL, FALSE, FALSE, SERVER_RUNNING_EVENT);

  if (m_hStartEvent == NULL) {
    CloseHandle(m_hStartEvent);
    return TRUE;
  }

  if (GetLastError() == ERROR_ALREADY_EXISTS) {

    CloseHandle(m_hStartEvent);
    m_hStartEvent = NULL;
    return TRUE;
  }
  return FALSE;
}