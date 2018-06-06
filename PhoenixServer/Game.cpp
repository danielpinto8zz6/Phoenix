#include "stdafx.h"

#include "Game.h"
#include "GameZone.h"

Coordinates getFirstEmptyPosition(Game *game) {
  Coordinates coordinates;

  for (coordinates.y = 0; coordinates.y < HEIGHT; coordinates.y++) {
    for (coordinates.x = 0; coordinates.x < WIDTH; coordinates.x++) {
      if (!isPositionOccupied(game, coordinates)) {
        return coordinates;
      }
    }
  }
  coordinates.x = -1;
  coordinates.y = -1;

  return coordinates;
}

BOOL isPositionOccupied(Game *game, Coordinates coordinates) {
  for (int i = 0; i < game->totalEnemyShips; i++) {
    if (coordinatesEqual(coordinates, game->enemyShip[i].position)) {
      return TRUE;
    }
  }
  return FALSE;
}

BOOL coordinatesEqual(Coordinates c1, Coordinates c2) {
  return c1.x == c2.x && c1.y == c2.y ? TRUE : FALSE;
}

BOOL isCoordinatesValid(Coordinates coordinates) {
  return coordinates.x == -1 || coordinates.y == -1 ? FALSE : TRUE;
}

DWORD WINAPI threadManageEnemyShips(LPVOID lpParam) {
  GameData *gameData = (GameData *)lpParam;

  HANDLE aThread[ENEMYSHIPS];
  DWORD ThreadID = 0;

  HANDLE hMutexManageEnemyShips;

  debug(TEXT("[ManageEnemyShips] -> Thread-%d"), GetCurrentThreadId());

  // Create a mutex with no initial owner
  hMutexManageEnemyShips = CreateMutex(NULL, FALSE, ENEMYSHIPS_MUTEX);

  if (hMutexManageEnemyShips == NULL) {
    error(TEXT("Creating enemy ships mutex"));
    return 1;
  }

  // Create Enemy Ships threads
  for (int i = 0; i < ENEMYSHIPS; i++) {
    gameData->position = i;
    aThread[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadEnemyShip,
                              gameData, 0, &ThreadID);

    if (aThread[i] == NULL) {
      error(TEXT("Creating enemy ship thread"));
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
  Coordinates c = getFirstEmptyPosition(&gameData->game);
  if (!isCoordinatesValid(c)) {
    error(TEXT("Can't find an empty position"));
    return FALSE;
  }

  gameData->game.enemyShip[position].position = c;

  // debug(TEXT("(EnemyShips Coordinates): %d %d"),
  //       gameData->game.enemyShip[position].position.x,
  //       gameData->game.enemyShip[position].position.y);

  sendGameToGateway(gameData, &gameData->game);

  ReleaseMutex(hMutexManageEnemyShips);

  return TRUE;
}