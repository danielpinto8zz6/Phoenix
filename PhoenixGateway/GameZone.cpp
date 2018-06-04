#include "stdafx.h"

#include "GameZone.h"

DWORD WINAPI receiveGameDataFromServer(LPVOID lpParam) {
  GameData *gameData = (GameData *)lpParam;

  DWORD dwWaitResult;

  while (gameData->ThreadMustConinue) {
    dwWaitResult = WaitForSingleObject(gameData->gameUpdateEvent, INFINITE);
    if (dwWaitResult == WAIT_OBJECT_0) {
      readDataFromSharedMemory(gameData->sharedGame, &gameData->game,
                               sizeof(Game), &gameData->hMutex);
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