// PhoenixGateway.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "Clients.h"
#include "PhoenixGateway.h"
#include "SharedMemory.h"

Game game;

int _tmain() {
  GameData gameData;
  DWORD threadListenerId;
  HANDLE hThreadListener;
  DWORD threadDataReceiverId;
  HANDLE hThreadDataReceiver;

#ifdef UNICODE
  _setmode(_fileno(stdin), _O_WTEXT);
  _setmode(_fileno(stdout), _O_WTEXT);
#endif

  if (!initMemAndSync(&gameData.hMapFile, GAMEDATA_SHARED_MEMORY_NAME,
                      &gameData.hMutex, GAMEDATA_MUTEX_NAME)) {
    return -1;
  }

  gameData.smWrite =
      CreateSemaphore(NULL, MAX_SEM_COUNT, MAX_SEM_COUNT, smWriteName);
  if (gameData.smWrite == NULL) {
    Error(TEXT("Initializing write semaphore"));
    return -1;
  }

  gameData.smRead = CreateSemaphore(NULL, 0, MAX_SEM_COUNT, smReadName);
  if (gameData.smRead == NULL) {
    Error(TEXT("Initializing read semaphore"));
    return -1;
  }

  gameData.game = (Game *)MapViewOfFile(gameData.hMapFile, FILE_MAP_ALL_ACCESS,
                                        0, 0, sizeof(Game));

  if (gameData.game == NULL) {
    Error(TEXT("Mapping shared memory"));
    return -1;
  }

  hThreadListener =
      CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadListener, &gameData,
                   0, &threadListenerId);
  if (hThreadListener == NULL) {
    Error(TEXT("Creating shared memory thread"));
    return -1;
  }

  /**
   * Gateway thread to receive info from clients
   */
  hThreadDataReceiver =
      CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)manageClients, &gameData, 0,
                   &threadDataReceiverId);
  if (hThreadDataReceiver == NULL) {
    Error(TEXT("Creating thread to manage clients"));
    return -1;
  }

  WaitForSingleObject(hThreadDataReceiver, INFINITE);
  WaitForSingleObject(hThreadListener, INFINITE);

  CloseHandle(gameData.hMapFile);
  CloseHandle(gameData.hMutex);
  CloseHandle(gameData.smWrite);
  CloseHandle(gameData.smRead);
  UnmapViewOfFile(gameData.game);

  system("pause");

  return 0;
}

DWORD WINAPI threadListener(LPVOID lpParam) {
  GameData *gameData = (GameData *)lpParam;
  DWORD current = peekData(gameData);

  while (gameData->ThreadMustConinue) {
    // Do not get data whitout permission
    // WaitForSingleObject(gameData->smRead, INFINITE);

    if (peekData(gameData) > current) {
      readDataFromSharedMemory(gameData->game, &game, sizeof(Game),
                               &gameData->hMutex);
      current = game.num;

      // Clear the console
      system("cls");

      // Show the actual map of the game
      for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
          _tprintf(TEXT("%c"), game.map[y][x]);
        }
        _tprintf(TEXT("\n"));
      }
    }

    // We can send data now
    // ReleaseSemaphore(gameData->smWrite, 1, NULL);
  }
  return 0;
}
