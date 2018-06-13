#include "stdafx.h"

#include "Clients.h"
#include "GameZone.h"

DWORD WINAPI receiveGameDataFromServer(LPVOID lpParam) {
  Data *data = (Data *)lpParam;

  GameData *gameData = data->gameData;

  DWORD dwWaitResult;

  gameData->STOP = FALSE;

  while (!gameData->STOP) {
    dwWaitResult = WaitForSingleObject(gameData->gameUpdateEvent, INFINITE);
    if (dwWaitResult == WAIT_OBJECT_0) {
      readDataFromSharedMemory(gameData->sharedGame, &gameData->game,
                               sizeof(Game), &gameData->hMutex);
      debug(TEXT("%d Bytes received"), sizeof(Game));
      // broadcastGameToClients(data, &gameData->game);
    }
  }
  return 0;
}
