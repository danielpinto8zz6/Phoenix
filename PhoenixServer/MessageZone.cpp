#include "stdafx.h"

#include "Clients.h"
#include "Game.h"
#include "MessageZone.h"
#include "GameZone.h"

DWORD WINAPI receiveMessagesFromGateway(LPVOID lpParam) {
  Data *data = (Data *)lpParam;

  MessageData *messageData = data->messageData;

  DWORD dwWaitResult;

  messageData->STOP = FALSE;

  while (!messageData->STOP) {
    dwWaitResult =
        WaitForSingleObject(messageData->serverMessageUpdateEvent, INFINITE);
    if (dwWaitResult == WAIT_OBJECT_0) {
      readDataFromSharedMemory(messageData->sharedMessage,
                               &messageData->message, sizeof(Message),
                               &messageData->hMutex);
      handleCommand(data, messageData->message);
    }
  }
  return 0;
}

void handleCommand(Data *data, Message message) {

  switch (message.cmd) {
  case LOGIN:
    clientLogin(data, message);
    break;
  case CLIENT_DISCONNECTED:
    if (removeClient(data, message.clientId)){
      sendGameToGateway(data->gameData, &data->gameData->game);
    }
    break;
  case JOIN_GAME:
    joinGame(data, message.clientId);
    break;
  case PLAYER_LOST:
    removePlayer(&data->gameData->game, message.clientId);
    break;
  case GATEWAY_DISCONNECTED:
    break;
  case KEYDOWN:
	  movePlayer(data->gameData, message.clientId, KEYDOWN);
    break;
  case KEYUP:
	  movePlayer(data->gameData, message.clientId, KEYUP);
    break;
  case KEYLEFT:
	  movePlayer(data->gameData, message.clientId, KEYLEFT);
    break;
  case KEYRIGHT:
	  movePlayer(data->gameData, message.clientId, KEYRIGHT);
    break;
  case KEYSPACE:
	  movePlayer(data->gameData, message.clientId, KEYSPACE);
    break;
  }
}


