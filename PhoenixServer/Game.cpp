#include "stdafx.h"

#include "Clients.h"
#include "Game.h"
#include "GameZone.h"
#include <time.h>

#define ENEMYSHIP_WIDTH 50
#define ENEMYSHIP_HEIGHT 50

#define TOTAL_SHOTS 50;

Coordinates getFirstEmptyPosition(Game *game) {
  Coordinates coordinates;

  coordinates.y = 0;
  coordinates.x = 0;

  int j = 0;

  for (int i = 0; i < game->maxEnemyShips; i++, j++) {
    if (!game->enemyShip[i].isEmpty) {
      coordinates.x = ENEMYSHIP_WIDTH * j;
      if (coordinates.x == 1000) {
        j = 0;
        coordinates.x = 0;
        coordinates.y += ENEMYSHIP_HEIGHT + 5;
      }
    }
  }

  return coordinates;
}

BOOL isPositionOccupied(Game *game, Coordinates coordinates) {
  for (int i = 0; i < game->maxEnemyShips; i++) {
    if (!game->enemyShip[i].isEmpty) {
      if (coordinatesEqual(coordinates, game->enemyShip[i].position)) {
        return TRUE;
      }
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

  HANDLE startEnemyShipsEvent;

  startEnemyShipsEvent =
      CreateEvent(NULL, TRUE, FALSE, TEXT("phoenix_start_enemy_ships_event"));

  if (startEnemyShipsEvent == NULL) {
    errorGui(TEXT("Error creating enemy ships start event"));
    return FALSE;
  }

  // Create a mutex with no initial owner
  hMutexManageEnemyShips = CreateMutex(NULL, FALSE, ENEMY_SHIPS_MUTEX);

  if (hMutexManageEnemyShips == NULL) {
    errorGui(TEXT("Creating enemy ships mutex"));
    return 1;
  }

  // Create Enemy Ships threads
  for (int i = 0; i < game->maxEnemyShips; i++) {
    aThread[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadEnemyShip,
                              gameData, 0, &ThreadID);

    if (aThread[i] == NULL) {
      errorGui(TEXT("Creating enemy ship thread"));
      return 1;
    }
  }

  // Wait a bit
  Sleep(500);

  /**
   * Enemy ships positioned, show to clients
   */
  sendGameToGateway(gameData, &gameData->game);

  /**
   * Now that threads are all up, let enemy ships move
   */
  Sleep(500);
  SetEvent(startEnemyShipsEvent);

  while (TRUE) {
    Sleep(100);
    sendGameToGateway(gameData, &gameData->game);
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

  HANDLE hThreadDropBombs;

  hMutexManageEnemyShips =
      OpenMutex(MUTEX_ALL_ACCESS, FALSE, ENEMY_SHIPS_MUTEX);

  HANDLE startEnemyShipsEvent;

  BOOL isOverlapping;

  Coordinates c1;

  int position = addEnemyShip(&gameData->game);

  if (position == -1) {
    // Can't add more enemy ships
    return FALSE;
  }

  startEnemyShipsEvent = OpenEventW(EVENT_ALL_ACCESS, FALSE,
                                    TEXT("phoenix_start_enemy_ships_event"));
  if (startEnemyShipsEvent == NULL) {
    errorGui(TEXT("Failed to open enemy ship start event"));
    return FALSE;
  }

  gameData->game.enemyShip[position].gameData = gameData;

  hThreadDropBombs = CreateThread(NULL, 0, dropBombs,
                                  &gameData->game.enemyShip[position], 0, NULL);
  if (hThreadDropBombs == NULL) {
    errorGui(TEXT("Creating bomb moving thread"));
    return FALSE;
  } else {
    CloseHandle(hThreadDropBombs);
  }

  WaitForSingleObject(startEnemyShipsEvent, INFINITE);

  // TODO: WIP
  while (TRUE) {
    WaitForSingleObject(hMutexManageEnemyShips, INFINITE);

    gameData->game.enemyShip[position].position.y +=
        gameData->game.enemyShip[position].velocity;

    c1 = gameData->game.enemyShip[position].position;
    c1.y += SCORE_BOARD_HEIGHT;

    for (int i = 0; i < gameData->game.maxPlayers + 1; i++) {
      if (!gameData->game.player[i].isEmpty) {
        isOverlapping =
            isRectangleOverlapping(c1, gameData->game.enemyShip[position].size,
                                   gameData->game.player[i].ship.position,
                                   gameData->game.player[i].ship.size);

        if (isOverlapping) {
          ReleaseMutex(hMutexManageEnemyShips);
          removeEnemyShip(&gameData->game, position);
          removePlayer(&gameData->game, gameData->game.player[i].id);
          return FALSE;
        }

        for (int j = 0; j < 50; j++) {
          if (!gameData->game.player[i].ship.shots[j].isEmpty) {

            c1 = gameData->game.enemyShip[position].position;

            Size sz;
            sz.height = 10;
            sz.width = 5;

            isOverlapping = isRectangleOverlapping(
                c1, gameData->game.enemyShip[position].size,
                gameData->game.player[i].ship.shots[j].position,sz);

            if (isOverlapping) {
              /** 
               * TODO : Fix bug here
               */
              gameData->game.enemyShip[position].strength--;
              if (gameData->game.enemyShip[position].strength < 1) {
                ReleaseMutex(hMutexManageEnemyShips);
                removeEnemyShip(&gameData->game, position);
                removeShot(&gameData->game.player[i].ship, j);
                gameData->game.player[i].score++;
                return FALSE;
              }
              removeShot(&gameData->game.player[i].ship, j);
              break;
            }
          }
        }
      }
    }
    // sendGameToGateway(gameData, &gameData->game);

    ReleaseMutex(hMutexManageEnemyShips);

    Sleep(300);
  }

  removeEnemyShip(&gameData->game, position);

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
  HANDLE hMutexPlayer;

  hMutexPlayer = OpenMutex(MUTEX_ALL_ACCESS, FALSE, PLAYERS_MUTEX);

  WaitForSingleObject(hMutexPlayer, INFINITE);

  int position = -1;

  for (int i = 0; i < game->maxPlayers; i++) {
    if (game->player[i].isEmpty) {
      position = i;
      break;
    }
  }

  if (position == -1) {
    errorGui(TEXT("Can't add more players"));
    return FALSE;
  }

  game->player[position].isEmpty = FALSE;

  _tcscpy_s(game->player[position].username, username);

  game->player[position].id = id;

  game->player[position].ship.size.width = 50;
  game->player[position].ship.size.height = 50;

  game->player[position].ship.velocity = game->velocityDefenderShips;

  for (int j = 0; j < 50; j++) {
    game->player[position].ship.shots[j].isEmpty = TRUE;
  }

  game->player[position].ship.shotVelocity = 10;

  ReleaseMutex(hMutexPlayer);

  return TRUE;
}

BOOL removePlayer(Game *game, int id) {
  HANDLE hMutexPlayer;
  BOOL fSuccess = FALSE;

  hMutexPlayer = OpenMutex(MUTEX_ALL_ACCESS, FALSE, PLAYERS_MUTEX);

  WaitForSingleObject(hMutexPlayer, INFINITE);

  for (int i = 0; i < game->maxPlayers; i++) {
    if (game->player[i].id == id) {
      game->player[i] = {};
      game->player[i].isEmpty = TRUE;
      fSuccess = TRUE;
      break;
    }
  }

  ReleaseMutex(hMutexPlayer);

  if (!fSuccess) {
    return FALSE;
  }

  return TRUE;
}

BOOL joinGame(Data *data, int id) {
  Message message;

  int position = -1;

  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (!data->clients[i].isEmpty) {
      if (data->clients[i].id == id) {
        position = i;
        break;
      }
    }
  }

  if (position == -1) {
    errorGui(TEXT("Client not found!"));
    return FALSE;
  }

  if (!addPlayer(&data->gameData->game, data->clients[position].username,
                 data->clients[position].id)) {
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
  for (int i = 0; i < game->maxPlayers; i++) {
    if (!game->player[i].isEmpty) {
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
}

void setUpPlayers(GameData *data) {
  int dist;

  int totalPlayers = 0;

  for (int i = 0; i < data->game.maxPlayers; i++) {
    if (!data->game.player[i].isEmpty) {
      totalPlayers++;
    }
  }

  dist = 950 / (totalPlayers + 1);
  for (int i = 0; i < data->game.maxPlayers; i++) {
    if (!data->game.player[i].isEmpty) {
      data->game.player[i].ship.position.x = dist * (i + 1);
      data->game.player[i].ship.position.y = 550;
      data->game.player[i].score = 0;
      data->game.player[i].lives = data->game.earlyLives;
      data->game.player[i].ship.velocity = data->game.velocityDefenderShips;
    }
  }
}

BOOL startGame(Data *data) {
  DWORD threadManageEnemyShipsId;
  HANDLE hThreadManageEnemyShips;

  HANDLE hMutexShot;
  HANDLE hMutexPlayer;
  HANDLE hMutexBomb;

  GameData *gameData = data->gameData;
  setUpPlayers(gameData);

  hMutexShot = CreateMutex(NULL, FALSE, SHOTS_MUTEX);
  if (hMutexShot == NULL) {
    error(TEXT("Creating shots mutex"));
    return FALSE;
  }

  hMutexPlayer = CreateMutex(NULL, FALSE, PLAYERS_MUTEX);
  if (hMutexPlayer == NULL) {
    error(TEXT("Creating players mutex"));
    return FALSE;
  }

  hMutexBomb = CreateMutex(NULL, FALSE, BOMBS_MUTEX);
  if (hMutexBomb == NULL) {
    error(TEXT("Creating players mutex"));
    return FALSE;
  }

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

  HANDLE hThreadShot;

  s.height = 50;
  s.width = 50;

  num = -1;

  for (int i = 0; i < game->maxPlayers; i++) {
    if (!game->player[i].isEmpty) {
      if (game->player[i].id == id) {
        num = i;
      }
    }
  }

  if (num == -1) {
    return;
  }

  c1 = game->player[num].ship.position;

  switch (m) {
  case KEYLEFT:

    c1.x -= game->player[num].ship.velocity;

    if (c1.x < 1)
      return;

    for (i = 0; i < game->maxPlayers; i++) {
      if (!game->player[i].isEmpty) {
        if (i != num) {

          overlaping =
              isRectangleOverlapping(c1, s, game->player[i].ship.position, s);

          if (overlaping)
            return;
        }
      }
    }

    game->player[num].ship.position.x -= game->player[num].ship.velocity;
    sendGameToGateway(gameData, game);

    break;
  case KEYRIGHT:
    c1.x += game->player[num].ship.velocity;

    if (c1.x > GAME_WIDTH - (game->player[num].ship.size.width + 1))
      return;

    for (i = 0; i < game->maxPlayers; i++) {
      if (!game->player[i].isEmpty) {
        if (i != num) {

          overlaping =
              isRectangleOverlapping(c1, s, game->player[i].ship.position, s);

          if (overlaping)
            return;
        }
      }
    }

    game->player[num].ship.position.x += game->player[num].ship.velocity;
    sendGameToGateway(gameData, game);
    break;
  case KEYUP:
    c1.y -= game->player[num].ship.velocity;
    if (c1.y < (GAME_HEIGHT - (GAME_HEIGHT * 0.2)) - SCORE_BOARD_HEIGHT)
      return;

    for (i = 0; i < game->maxPlayers; i++) {
      if (!game->player[i].isEmpty) {
        if (i != num) {

          overlaping =
              isRectangleOverlapping(c1, s, game->player[i].ship.position, s);

          if (overlaping)
            return;
        }
      }
    }

    game->player[num].ship.position.y -= game->player[num].ship.velocity;
    sendGameToGateway(gameData, game);
    break;
  case KEYDOWN:
    c1.y += game->player[num].ship.velocity;
    if (c1.y > (GAME_HEIGHT -
                (game->player[num].ship.size.height + SCORE_BOARD_HEIGHT + 1)))
      return;

    for (i = 0; i < game->maxPlayers; i++) {
      if (!game->player[i].isEmpty) {
        if (i != num) {

          overlaping =
              isRectangleOverlapping(c1, s, game->player[i].ship.position, s);

          if (overlaping)
            return;
        }
      }
    }

    game->player[num].ship.position.y += game->player[num].ship.velocity;
    sendGameToGateway(gameData, game);
    break;
  case KEYSPACE:
    hThreadShot =
        CreateThread(NULL, 0, manageShot, &game->player[num].ship, 0, NULL);
    if (hThreadShot == NULL) {
      errorGui(TEXT("Creating shot moving thread"));
      return;
    } else {
      CloseHandle(hThreadShot);
    }
    break;
  default:

    break;
  }
}

int addShot(DefenderShip *defenderShip) {
  HANDLE hMutexShot;

  hMutexShot = OpenMutex(MUTEX_ALL_ACCESS, FALSE, SHOTS_MUTEX);

  WaitForSingleObject(hMutexShot, INFINITE);

  int position = -1;

  for (int i = 0; i < 50; i++) {
    if (defenderShip->shots[i].isEmpty) {
      position = i;
      break;
    }
  }

  if (position == -1) {
    return position;
  }

  defenderShip->shots[position].position.x = defenderShip->position.x + 24;
  defenderShip->shots[position].position.y = defenderShip->position.y + 1;
  defenderShip->shots[position].isEmpty = FALSE;

  ReleaseMutex(hMutexShot);

  return position;
}

BOOL removeShot(DefenderShip *defenderShip, int position) {
  HANDLE hMutexShot;

  hMutexShot = OpenMutex(MUTEX_ALL_ACCESS, FALSE, SHOTS_MUTEX);

  WaitForSingleObject(hMutexShot, INFINITE);

  defenderShip->shots[position] = {};
  defenderShip->shots[position].isEmpty = TRUE;

  ReleaseMutex(hMutexShot);

  return TRUE;
}

DWORD WINAPI manageShot(LPVOID lParam) {
  DefenderShip *defenderShip = (DefenderShip *)lParam;

  int position;

  position = addShot(defenderShip);

  if (position == -1) {
    return FALSE;
  }

  /**
   * Perform shot move
   */
  while (defenderShip->shots[position].position.y > 20) {
    defenderShip->shots[position].position.y -= defenderShip->shotVelocity;
    Sleep(100);
  }

  removeShot(defenderShip, position);

  return TRUE;
}

int addEnemyShip(Game *game) {
  HANDLE hMutexEnemyShip;

  hMutexEnemyShip = OpenMutex(MUTEX_ALL_ACCESS, FALSE, ENEMY_SHIPS_MUTEX);

  WaitForSingleObject(hMutexEnemyShip, INFINITE);

  int position = -1;

  for (int i = 0; i < game->maxEnemyShips; i++) {
    if (game->enemyShip[i].isEmpty) {
      position = i;
      break;
    }
  }

  if (position == -1) {
    return position;
  }

  game->enemyShip[position].isEmpty = FALSE;

  Coordinates c = getFirstEmptyPosition(game);
  if (!isCoordinatesValid(c)) {
    errorGui(TEXT("Can't find an empty position"));
    return FALSE;
  }

  game->enemyShip[position].position = c;

  game->enemyShip[position].size.height = 50;
  game->enemyShip[position].size.width = 50;

  if (position < 7) {
    game->enemyShip[position].type = BASIC;
    game->enemyShip[position].strength = 1;
    game->enemyShip[position].velocity = game->velocityEnemyShips;
    game->enemyShip[position].fireRate = 1 * game->difficulty;
  } else if (position >= 7 && position < 14) {
    game->enemyShip[position].type = DODGE;
    game->enemyShip[position].strength = 3;
    game->enemyShip[position].velocity = game->velocityEnemyShips;
    game->enemyShip[position].fireRate = 1 * game->difficulty;

  } else {
    game->enemyShip[position].type = SUPERBAD;
    game->enemyShip[position].strength = 4;
    game->enemyShip[position].velocity = game->velocityEnemyShips;
    game->enemyShip[position].fireRate = 1 * game->difficulty;
  }

  for (int j = 0; j < 50; j++) {
    game->enemyShip[position].bombs[j].isEmpty = TRUE;
  }

  game->enemyShip[position].BombVelocity = 8;

  ReleaseMutex(hMutexEnemyShip);

  return position;
}

BOOL removeEnemyShip(Game *game, int position) {
  HANDLE hMutexEnemyShip;

  hMutexEnemyShip = OpenMutex(MUTEX_ALL_ACCESS, FALSE, ENEMY_SHIPS_MUTEX);

  WaitForSingleObject(hMutexEnemyShip, INFINITE);

  game->enemyShip[position] = {};
  game->enemyShip[position].isEmpty = TRUE;

  ReleaseMutex(hMutexEnemyShip);

  return TRUE;
}

int addBomb(EnemyShip *enemyShip) {
  HANDLE hMutexBomb;

  hMutexBomb = OpenMutex(MUTEX_ALL_ACCESS, FALSE, BOMBS_MUTEX);

  WaitForSingleObject(hMutexBomb, INFINITE);

  int position = -1;

  for (int i = 0; i < 50; i++) {
    if (enemyShip->bombs[i].isEmpty) {
      position = i;
      break;
    }
  }

  if (position == -1) {
    return position;
  }

  enemyShip->bombs[position].position.x = enemyShip->position.x + 24;
  enemyShip->bombs[position].position.y = enemyShip->position.y + 51;
  enemyShip->bombs[position].isEmpty = FALSE;

  ReleaseMutex(hMutexBomb);

  return position;
}

BOOL removeBomb(EnemyShip *enemyShip, int position) {
  HANDLE hMutexBomb;

  hMutexBomb = OpenMutex(MUTEX_ALL_ACCESS, FALSE, BOMBS_MUTEX);

  WaitForSingleObject(hMutexBomb, INFINITE);

  enemyShip->bombs[position] = {};
  enemyShip->bombs[position].isEmpty = TRUE;

  ReleaseMutex(hMutexBomb);

  return TRUE;
}

DWORD WINAPI manageBomb(LPVOID lParam) {
  EnemyShip *enemyShip = (EnemyShip *)lParam;

  GameData *gameData = (GameData *)enemyShip->gameData;

  int position;

  position = addBomb(enemyShip);

  BOOL isOverlapping;
  Coordinates c1;
  Size size;

  size.width = 5;
  size.height = 10;

  if (position == -1) {
    return FALSE;
  }

  /**
   * Perform bomb move
   */
  while (enemyShip->bombs[position].position.y <
             GAME_HEIGHT - SCORE_BOARD_HEIGHT &&
         enemyShip->bombs[position].position.x > 1 &&
         enemyShip->bombs[position].position.x < GAME_WIDTH) {
    enemyShip->bombs[position].position.y += enemyShip->BombVelocity;
    enemyShip->bombs[position].position.x += enemyShip->BombVelocity;

    c1 = enemyShip->bombs[position].position;
    c1.y += 20;

    // for (int i = 0; i < gameData->game.maxPlayers; i++) {
    //   if (!gameData->game.player[i].isEmpty) {
    //     isOverlapping = isRectangleOverlapping(
    //         c1, size, gameData->game.player[i].ship.position,
    //         gameData->game.player[i].ship.size);

    //     if (isOverlapping) {
    //       gameData->game.player[i].lives--;
    //       if (gameData->game.player[i].lives < 1) {
    //         removePlayer(&gameData->game, gameData->game.player[i].id);
    //         removeBomb(enemyShip, position);
    //         return FALSE;
    //       }
    //       removeBomb(enemyShip, position);
    //       break;
    //     }
    //   }
    // }
    Sleep(100);
  }

  removeBomb(enemyShip, position);

  return TRUE;
}

DWORD WINAPI dropBombs(LPVOID lParam) {
  EnemyShip *enemyShip = (EnemyShip *)lParam;

  HANDLE hThreadBomb;

  while (TRUE) {
    hThreadBomb = CreateThread(NULL, 0, manageBomb, enemyShip, 0, NULL);
    if (hThreadBomb == NULL) {
      return FALSE;
    } else {
      CloseHandle(hThreadBomb);
    }

    switch (enemyShip->type) {
    case DODGE:
      Sleep(3000);
      break;
    case BASIC:
      Sleep(4000);
      break;
    case SUPERBAD:
      Sleep(5000);
      break;
    }
  }
  return TRUE;
}

void nextLevel(Game *game) {
  game->level++;
  game->velocityDefenderShips += 2;
  game->difficulty++;
}