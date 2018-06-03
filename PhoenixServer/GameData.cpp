#include "stdafx.h"

#include "Game.h"
#include "GameData.h"
#include "PhoenixServer.h"

HANDLE EnemyShipsMutex;

GameData gameData;
Game game;

int initGameData() {
  DWORD threadManageEnemyShipsId;
  HANDLE hThreadManageEnemyShips;

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

  game.num = 0;

  if (gameData.game == NULL) {
    Error(TEXT("Mapping shared memory"));
    return -1;
  }

  hThreadManageEnemyShips =
      CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadManageEnemyShips,
                   NULL, 0, &threadManageEnemyShipsId);
  if (hThreadManageEnemyShips == NULL) {
    Error(TEXT("Creating thread to manage enemy ships"));
    return -1;
  }

  WaitForSingleObject(hThreadManageEnemyShips, INFINITE);

  CloseHandle(gameData.smRead);
  CloseHandle(gameData.smWrite);
  CloseHandle(hThreadManageEnemyShips);
  CloseHandle(gameData.hMutex);
  CloseHandle(gameData.hMapFile);
  UnmapViewOfFile(gameData.game);

  return 0;
}

DWORD WINAPI threadManageEnemyShips(LPVOID lpParam) {
  HANDLE aThread[ENEMYSHIPS];
  DWORD ThreadID = 0;
  int i;

  _tprintf(TEXT("[ManageEnemyShips] -> Thread-%d\n"), GetCurrentThreadId());

  // Create a mutex with no initial owner
  EnemyShipsMutex = CreateMutex(NULL, FALSE, NULL);

  if (EnemyShipsMutex == NULL) {
    Error(TEXT("Creating enemy ships mutex"));
    return 1;
  }

  // Create Enemy Ships threads
  for (int i = 0; i < ENEMYSHIPS; i++) {
    int x = i;
    aThread[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadEnemyShip,
                              &x, 0, &ThreadID);

    if (aThread[i] == NULL) {
      Error(TEXT("Creating enemy ship thread"));
      return 1;
    }
    Sleep(500);
  }

  // Wait for all threads to terminate
  WaitForMultipleObjects(ENEMYSHIPS, aThread, TRUE, INFINITE);

  // Close thread and mutex handles
  for (i = 0; i < ENEMYSHIPS; i++)
    CloseHandle(aThread[i]);

  CloseHandle(EnemyShipsMutex);

  return 0;
}

DWORD WINAPI threadEnemyShip(LPVOID lpParam) {
  int position = *(int *)lpParam;
  
  WaitForSingleObject(EnemyShipsMutex, INFINITE);

  WaitForSingleObject(gameData.smWrite, INFINITE);

  _tprintf(TEXT("[EnemyShip] -> %i\n"), position);

  // Place ship...
  Coordinates *c = GetFirstEmptyPosition(&game);
  if (c != NULL) {
    game.enemyShip[position].position = *c;
    game.map[c->y][c->x] = '#';
  }

  game.num++;

  writeDataToSharedMemory(gameData.game, &game, sizeof(Game), &gameData.hMutex);

  ReleaseSemaphore(gameData.smRead, 1, NULL);

  ReleaseMutex(EnemyShipsMutex);

  return TRUE;
}