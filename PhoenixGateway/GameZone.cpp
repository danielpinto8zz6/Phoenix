#include "stdafx.h"

#include "GameZone.h"

BOOL initGameZone(GameData *gameData) {
  if (!initMemAndSync(&gameData->hMapFile, GAMEDATA_SHARED_MEMORY_NAME,
                      &gameData->hMutex, GAMEDATA_MUTEX_NAME)) {
    return FALSE;
  }

  gameData->smWrite =
      CreateSemaphore(NULL, MAX_SEM_COUNT, MAX_SEM_COUNT, smWriteName);
  if (gameData->smWrite == NULL) {
    Error(TEXT("Initializing write semaphore"));
    return FALSE;
  }

  gameData->smRead = CreateSemaphore(NULL, 0, MAX_SEM_COUNT, smReadName);
  if (gameData->smRead == NULL) {
    Error(TEXT("Initializing read semaphore"));
    return FALSE;
  }

  gameData->game = (Game *)MapViewOfFile(
      gameData->hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Game));

  if (gameData->game == NULL) {
    Error(TEXT("Mapping shared memory"));
    return FALSE;
  }

  return TRUE;
}

DWORD WINAPI receiveGameDataFromServer(LPVOID lpParam) {
  GameData *gameData = (GameData *)lpParam;
  Game game;
  game.num = 0;

  DWORD current = peekGameData(gameData);

  while (gameData->ThreadMustConinue) {
    // Do not get data whitout permission
    WaitForSingleObject(gameData->smRead, INFINITE);

    if (peekGameData(gameData) > current) {
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
    ReleaseSemaphore(gameData->smWrite, 1, NULL);
  }
  return 0;
}

DWORD peekGameData(GameData *data) {
  DWORD num;
  WaitForSingleObject(data->hMutex, INFINITE);
  num = data->game->num;
  ReleaseMutex(data->hMutex);
  return num;
}
