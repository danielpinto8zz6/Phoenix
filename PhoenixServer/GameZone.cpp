#include "stdafx.h"

#include "Game.h"
#include "GameZone.h"
#include "PhoenixServer.h"

HANDLE hMutexManageEnemyShips;

DWORD WINAPI threadManageEnemyShips(LPVOID lpParam) {
  GameData *gameData = (GameData *)lpParam;

  HANDLE aThread[ENEMYSHIPS];
  DWORD ThreadID = 0;

  _tprintf(TEXT("[ManageEnemyShips] -> Thread-%d\n"), GetCurrentThreadId());

  // Create a mutex with no initial owner
  hMutexManageEnemyShips = CreateMutex(NULL, FALSE, NULL);

  if (hMutexManageEnemyShips == NULL) {
    Error(TEXT("Creating enemy ships mutex"));
    return 1;
  }

  // Create Enemy Ships threads
  for (int i = 0; i < ENEMYSHIPS; i++) {
    gameData->position = i;
    aThread[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadEnemyShip,
                              gameData, 0, &ThreadID);

    if (aThread[i] == NULL) {
      Error(TEXT("Creating enemy ship thread"));
      return 1;
    }
    Sleep(500);
  }

  // Wait for all threads to terminate
  WaitForMultipleObjects(ENEMYSHIPS, aThread, TRUE, INFINITE);

  // Close thread and mutex handles
  for (int i = 0; i < ENEMYSHIPS; i++)
    CloseHandle(aThread[i]);

  CloseHandle(hMutexManageEnemyShips);

  return 0;
}

DWORD WINAPI threadEnemyShip(LPVOID lpParam) {
  GameData *gameData = (GameData *)lpParam;

  int position = gameData->position;

  WaitForSingleObject(hMutexManageEnemyShips, INFINITE);

  // Place ship...
  Coordinates *c = GetFirstEmptyPosition(&gameData->game);
  if (c != NULL) {
    gameData->game.enemyShip[position].position = *c;
    gameData->game.map[c->y][c->x] = '#';
  }

  writeDataToSharedMemory(gameData->sharedGame, &gameData->game, sizeof(Game),
                          &gameData->hMutex);
  if (!SetEvent(gameData->gameUpdateEvent)) {
    Error(TEXT("SetEvent failed"));
  }

  ReleaseMutex(hMutexManageEnemyShips);

  return TRUE;
}