#include "stdafx.h"

#include "Clients.h"
#include "GameZone.h"

DWORD WINAPI receiveGameDataFromServer(LPVOID lpParam) {
  Data *data = (Data *)lpParam;

  GameData *gameData = data->gameData;
  MessageData *messageData = data->messageData;

  DWORD dwWaitResult;

  Message message;
  message.cmd = UPDATE_GAME;

  while (gameData->ThreadMustConinue) {
    dwWaitResult = WaitForSingleObject(gameData->gameUpdateEvent, INFINITE);
    if (dwWaitResult == WAIT_OBJECT_0) {
      readDataFromSharedMemory(gameData->sharedGame, &gameData->game,
                               sizeof(Game), &gameData->hMutex);
      Debug(TEXT("%d Bytes received\n"), sizeof(Game));
      sendMessageToAllClients(data, &message);
    }
  }
  return 0;
}