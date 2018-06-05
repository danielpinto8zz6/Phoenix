#include "stdafx.h"

#include "Game.h"
#include "GameZone.h"
#include "PhoenixServer.h"

DWORD WINAPI threadManageEnemyShips(LPVOID lpParam) {
  GameData *gameData = (GameData *)lpParam;

  HANDLE aThread[ENEMYSHIPS];
  DWORD ThreadID = 0;

  HANDLE hMutexManageEnemyShips;

  Debug(TEXT("[ManageEnemyShips] -> Thread-%d"), GetCurrentThreadId());

  // Create a mutex with no initial owner
  hMutexManageEnemyShips = CreateMutex(NULL, FALSE, ENEMYSHIPS_MUTEX);

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

  HANDLE hMutexManageEnemyShips;

  hMutexManageEnemyShips = OpenMutex(MUTEX_ALL_ACCESS, FALSE, ENEMYSHIPS_MUTEX);

  int position = gameData->position;

  WaitForSingleObject(hMutexManageEnemyShips, INFINITE);

  // Place ship...
  Coordinates *c = GetFirstEmptyPosition(&gameData->game);
  if (c != NULL) {
    gameData->game.enemyShip[position].position = *c;
    gameData->game.map[c->y][c->x] = '#';
  }

  Debug(TEXT("(EnemyShips Coordinates): %d %d"),
        gameData->game.enemyShip[position].position.x,
        gameData->game.enemyShip[position].position.y);

  sendGameToGateway(gameData, &gameData->game);

  ReleaseMutex(hMutexManageEnemyShips);

  return TRUE;
}

BOOL sendGameToGateway(GameData *gameData, Game *game) {
  writeDataToSharedMemory(gameData->sharedGame, game, sizeof(Game),
                          &gameData->hMutex);
  if (!SetEvent(gameData->gameUpdateEvent)) {
    Error(TEXT("SetEvent failed"));
  }
  Debug(TEXT("%d Bytes sent to gateway"), sizeof(Game));
  return TRUE;
}