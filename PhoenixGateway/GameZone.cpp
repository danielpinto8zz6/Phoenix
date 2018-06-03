#include "stdafx.h"

#include "GameZone.h"

DWORD WINAPI receiveGameDataFromServer(LPVOID lpParam) {
  GameData *gameData = (GameData *)lpParam;
  gameData->game.num = 0;

  DWORD current = peekGameData(gameData);

  while (gameData->ThreadMustConinue) {
    // Do not get data whitout permission
    WaitForSingleObject(gameData->smRead, INFINITE);

    if (peekGameData(gameData) > current) {
      readDataFromSharedMemory(gameData->sharedGame, &gameData->game, sizeof(Game),
                               &gameData->hMutex);
      current = gameData->game.num;

      // Clear the console
      system("cls");

      // Show the actual map of the game
      for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
          _tprintf(TEXT("%c"), gameData->game.map[y][x]);
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
  num = data->sharedGame->num;
  ReleaseMutex(data->hMutex);
  return num;
}
