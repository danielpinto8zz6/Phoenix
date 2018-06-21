﻿#include "stdafx.h"

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
    removeClient(data, message.clientId);
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
    break;
  case KEYUP:
    break;
  case KEYLEFT:
	  movePlayer(data->gameData, message.clientId, LEFT);
    break;
  case KEYRIGHT:
	  movePlayer(data->gameData, message.clientId, RIGHT);
    break;
  case KEYSPACE:
	  movePlayer(data->gameData, message.clientId, RIGHT);

    break;
  }
}


