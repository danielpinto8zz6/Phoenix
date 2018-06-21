#include "stdafx.h"

#include "Clients.h"
#include "Game.h"
#include "GameZone.h"
#include <time.h>

#define ENEMYSHIP_WIDTH 50
#define ENEMYSHIP_HEIGHT 50

Coordinates getFirstEmptyPosition(Game *game) {
  Coordinates coordinates;

  coordinates.y = 0;
  coordinates.x = 0;

  int j = 0;

  for (int i = 0; i < game->totalEnemyShips; i++, j++) {
    coordinates.x = ENEMYSHIP_WIDTH * j;
    if (coordinates.x == 1000) {
      j = 0;
      coordinates.x = 0;
      coordinates.y += ENEMYSHIP_HEIGHT + 5;
    }
  }

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

  Game *game = &gameData->game;

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
  for (int i = 0; i < game->maxEnemyShips; i++) {
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

  gameData->game.totalEnemyShips = position + 1;

  WaitForSingleObject(hMutexManageEnemyShips, INFINITE);

  // Place ship...hNaveBasichNaveDodge
  Coordinates c = getFirstEmptyPosition(&gameData->game);
  if (!isCoordinatesValid(c)) {
    errorGui(TEXT("Can't find an empty position"));
    return FALSE;
  }

  gameData->game.enemyShip[position].position = c;

  gameData->game.enemyShip[position].size.height = 50;
  gameData->game.enemyShip[position].size.width = 50;

  if (position < 7) {
    gameData->game.enemyShip[position].type = BASIC;
    gameData->game.enemyShip[position].strength = 1;
    gameData->game.enemyShip[position].velocity =
        gameData->game.velocityEnemyShips;
    gameData->game.enemyShip[position].fireRate = 1 * gameData->game.difficulty;
  } else if (position >= 7 && position < 14) {
    gameData->game.enemyShip[position].type = DODGE;
    gameData->game.enemyShip[position].strength = 3;
    gameData->game.enemyShip[position].velocity =
        gameData->game.velocityEnemyShips;
    gameData->game.enemyShip[position].fireRate = 1 * gameData->game.difficulty;

  } else {
    gameData->game.enemyShip[position].type = SUPERBAD;
    gameData->game.enemyShip[position].strength = 4;
    gameData->game.enemyShip[position].velocity =
        gameData->game.velocityEnemyShips;
    gameData->game.enemyShip[position].fireRate = 1 * gameData->game.difficulty;
  }

  sendGameToGateway(gameData, &gameData->game);

  ReleaseMutex(hMutexManageEnemyShips);

  // TODO: WIP
  while (TRUE) {
    WaitForSingleObject(hMutexManageEnemyShips, INFINITE);

    gameData->game.enemyShip[position].position.y++;

    sendGameToGateway(gameData, &gameData->game);

    ReleaseMutex(hMutexManageEnemyShips);

    Sleep(100);
  }

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

  int i = game->totalPlayers;

  _tcscpy_s(game->player[i].username, username);
  game->player[i].id = id;

  game->player[i].ship.size.width = 50;
  game->player[i].ship.size.height = 50;

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
  Message message;

  int i = getClientIndex(data, id);

  if (i == -1) {
    return FALSE;
  }

  if (!addPlayer(&data->gameData->game, data->clients[i].username,
                 data->clients[i].id)) {
    return FALSE;
  }

  message.clientId = id;
  message.cmd = IN_GAME;

  writeDataToSharedMemory(data->messageData->sharedMessage, &message,
                          sizeof(Message), data->messageData->hMutex,
                          data->messageData->gatewayMessageUpdateEvent);

  return TRUE;
}

int compare(const void *a, const void *b) {
  return (*(int *)a > *(int *)b) - (*(int *)a < *(int *)b);
}

void sort(int *arr, size_t len) { qsort(arr, len, sizeof(int), compare); }

void setupTopTen(Game *game) {
  for (int i = 0; i < game->totalPlayers; i++) {
    for (int j = 0; j < 10; j++) {
      if (game->player[i].score > game->topTen[j].score) {
        for (int k = j; k < 10; k++) {
          game->topTen[k] = game->topTen[k + 1];
        }
        game->topTen[j].score = game->player[i].score;
        _tcscpy_s(game->topTen[j].username, game->player[i].username);
      }
    }
  }
}

void setUpPlayers(GameData *data) {
  int dist;

  dist = 950 / (data->game.totalPlayers + 1);
  for (int i = 0; i < data->game.totalPlayers; i++) {

    data->game.player[i].ship.position.x = dist * (i + 1);
    data->game.player[i].ship.position.y = 550;
    data->game.player[i].score = 0;
    data->game.player[i].lives = data->game.earlyLives;
  }
}

BOOL startGame(Data *data) {
  DWORD threadManageEnemyShipsId;
  HANDLE hThreadManageEnemyShips;

  GameData *gameData = data->gameData;
  setUpPlayers(gameData);
  hThreadManageEnemyShips =
      CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadManageEnemyShips,
                   gameData, 0, &threadManageEnemyShipsId);
  if (hThreadManageEnemyShips == NULL) {
    errorGui(TEXT("Creating thread to manage enemy ships"));
    return FALSE;
  }

  return TRUE;
}

void movePlayer(GameData *gameData, int id, Command m) {
  Game *game = &gameData->game;
  int num, i;
  Coordinates c1;
  BOOL overlaping;
  Size s;
  s.height = 50;
  s.width = 50;
  num = getPlayerIndex(game, id);
  c1 = game->player[num].ship.position;

  switch (m) {
  case KEYLEFT:

    c1.x -= 2;

    if (c1.x < 1)
      return;

    if (game->totalPlayers > 1) {
      for (i = 0; i < game->totalPlayers; i++) {
        if (i != num) {

          overlaping =
              isRectangleOverlapping(c1, s, game->player[i].ship.position, s);

          if (overlaping)
            return;
        }
      }
    }
    game->player[num].ship.position.x -= 2;
    sendGameToGateway(gameData, game);

    break;
  case KEYRIGHT:
    c1.x += 2;

    if (c1.x > WINDOW_WIDTH - 1)
      return;

    if (game->totalPlayers > 1) {
      for (i = 0; i < game->totalPlayers; i++) {
        if (i != num) {

          overlaping =
              isRectangleOverlapping(c1, s, game->player[i].ship.position, s);

          if (overlaping)
            return;
        }
      }
    }

    game->player[num].ship.position.x += 2;
    sendGameToGateway(gameData, game);
    break;
  case KEYUP:
    c1.y -= 2;
    if (c1.y < 530)
      return;

    if (game->totalPlayers > 1) {
      for (i = 0; i < game->totalPlayers; i++) {
        if (i != num) {

          overlaping =
              isRectangleOverlapping(c1, s, game->player[i].ship.position, s);

          if (overlaping)
            return;
        }
      }
    }

    game->player[num].ship.position.y -= 2;
    sendGameToGateway(gameData, game);
    break;
  case KEYDOWN:
    c1.y += 2;
    if (c1.y > (WINDOW_HEIGHT - 115 + 2))
      return;

    if (game->totalPlayers > 1) {
      for (i = 0; i < game->totalPlayers; i++) {
        if (i != num) {

          overlaping =
              isRectangleOverlapping(c1, s, game->player[i].ship.position, s);

          if (overlaping)
            return;
        }
      }
    }

    game->player[num].ship.position.y += 2;
    sendGameToGateway(gameData, game);
    break;
  case KEYSPACE:
    num = getPlayerIndex(game, id);
    break;
  default:

    break;
  }
}