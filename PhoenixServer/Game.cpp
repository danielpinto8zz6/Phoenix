#include "stdafx.h"

#include "Game.h"
#include "GameZone.h"
#include "Clients.h"

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

  // Create a mutex with no initial owner
  hMutexManageEnemyShips = CreateMutex(NULL, FALSE, ENEMYSHIPS_MUTEX);

  if (hMutexManageEnemyShips == NULL) {
    errorGui(TEXT("Creating enemy ships mutex"));
    return 1;
  }

  // Create Enemy Ships threads
  for (int i = 0; i < ENEMYSHIPS; i++) {
    gameData->position = i;
    aThread[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadEnemyShip,
                              gameData, 0, &ThreadID);

    if (aThread[i] == NULL) {
      errorGui(TEXT("Creating enemy ship thread"));
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
    errorGui(TEXT("Can't find an empty position"));
    return FALSE;
  }

  gameData->game.enemyShip[position].position = c;

  // debug(TEXT("(EnemyShips Coordinates): %d %d"),
  //       gameData->game.enemyShip[position].position.x,
  //       gameData->game.enemyShip[position].position.y);

  while (TRUE) {
  }

  sendGameToGateway(gameData, &gameData->game);

  ReleaseMutex(hMutexManageEnemyShips);

  return TRUE;
}

BOOL isRectangleOverlapping(Coordinates rect1Coord, Size rect1Sz,
                            Coordinates rect2Coord, Size rect2Sz) {
  Coordinates totalRect1, totalRect2;

  totalRect1.x = rect1Coord.x + rect1Sz.width;
  totalRect1.y = rect1Coord.y + rect1Sz.height;

  totalRect2.x = rect2Coord.x + rect2Sz.width;
  totalRect2.y = rect2Coord.y + rect2Sz.height;

  /**
   * Check overlap on x & y
   */
  if (rect1Coord.x < rect2Coord.x) {
    if (rect2Coord.x <= totalRect1.x) {
      if (rect1Coord.y < rect2Coord.y) {
        if (rect2Coord.y <= totalRect1.y) {
          return TRUE;
        }
      } else {
        if (rect1Coord.y <= totalRect2.y) {
          return TRUE;
        }
      }
    }
  } else {
    if (rect1Coord.x <= totalRect2.x) {
      if (rect1Coord.y < rect2Coord.y) {
        if (rect2Coord.y <= totalRect1.y) {
          return TRUE;
        }
      } else {
        if (rect1Coord.y <= totalRect2.y) {
          return TRUE;
        }
      }
    }
  }

  return FALSE;
}

BOOL isPointInsideRect(Coordinates rect1Coord, Size rect1Sz,
                       Coordinates point) {
  Coordinates totalRect1;

  totalRect1.x = rect1Coord.x + rect1Sz.width;
  totalRect1.y = rect1Coord.y + rect1Sz.height;

  /**
   * Check on x
   */
  if (point.x >= rect1Coord.x && point.x <= rect1Coord.x) {
    if (point.y >= rect1Coord.y && point.y <= rect1Coord.y) {
      return TRUE;
    }
  }
  return FALSE;
}

BOOL addPlayer(Game *game, TCHAR username[50], int id) {
  if (game->totalPlayers >= MAX_PLAYERS) {
    return FALSE;
  }

  _tcscpy_s(game->player[game->totalPlayers].username, username);
  game->player[game->totalPlayers].id = id;

  game->totalPlayers++;

  return TRUE;
}

int getPlayerIndex(Game *game, int id) {
  for (int i = 0; i < game->totalPlayers; i++) {
    if (game->player[i].id == id) {
      return i;
    }
  }
  return -1;
}

BOOL removePlayer(Game *game, int id) {
  int n = getPlayerIndex(game, id);

  if (n == -1) {
    return FALSE;
  }

  for (int i = n; i < game->totalPlayers; i++) {
    game->player[i] = game->player[i + 1];
  }

  game->totalPlayers--;

  return TRUE;
}

BOOL joinGame(Data *data, int id) {
  int i = getClientIndex(data, id);

  if (i == -1) {
    return FALSE;
  }

  return addPlayer(&data->gameData->game, data->clients[i].username,
            data->clients[i].id);
}