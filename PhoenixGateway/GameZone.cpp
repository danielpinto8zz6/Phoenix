#include "stdafx.h"

#include "GameZone.h"

DWORD WINAPI receiveGameDataFromServer(LPVOID lpParam) {
  GameData *gameData = (GameData *)lpParam;

  DWORD dwWaitResult;

  while (gameData->ThreadMustConinue) {
    dwWaitResult = WaitForSingleObject(gameData->gameUpdateEvent, INFINITE);
    switch (dwWaitResult) {
    case WAIT_OBJECT_0:
      _tprintf(TEXT("Nice we can read data\n"));
      readDataFromSharedMemory(gameData->sharedGame, &gameData->game,
                               sizeof(Game), &gameData->hMutex);
      break;
    default:
      Error(TEXT("Wait error"));
      return 0;
    }

    // system("cls");

    // // Show the actual map of the game
    // for (int y = 0; y < HEIGHT; y++) {
    //   for (int x = 0; x < WIDTH; x++) {
    //     _tprintf(TEXT("%c"), gameData->game.map[y][x]);
    //   }
    //   _tprintf(TEXT("\n"));
    // }
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