#include "stdafx.h"

#include "Clients.h"
#include "Game.h"
#include "MessageZone.h"

BOOL addClient(Data *data, TCHAR username[50], int id) {
  if (data->totalClients >= MAX_CLIENTS) {
    return FALSE;
  }

  _tcscpy_s(data->clients[data->totalClients].username, username);
  data->clients[data->totalClients].id = id;

  data->totalClients++;

  return TRUE;
}

int getClientIndex(Data *data, int id) {
  for (int i = 0; i < data->totalClients; i++) {
    if (data->clients[i].id == id) {
      return i;
    }
  }
  return -1;
}

BOOL removeClient(Data *data, int id) {
  int n = getClientIndex(data, id);

  if (n == -1) {
    return FALSE;
  }

  for (int i = n; i < data->totalClients; i++) {
    data->clients[i] = data->clients[i + 1];
  }

  data->totalClients--;

  removePlayer(&data->gameData->game, id);

  return TRUE;
}

void clientLogin(Data *data, Message message) {
  if (!addClient(data, message.text, message.clientId)) {
    errorGui(TEXT("Can't add client"));
  }

  message.cmd = LOGGED;

  /**
   * Player added : inform him
   */
  writeDataToSharedMemory(data->messageData->sharedMessage, &message,
                          sizeof(Message), data->messageData->hMutex,
                          data->messageData->gatewayMessageUpdateEvent);
}