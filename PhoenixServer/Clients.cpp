#include "stdafx.h"

#include "Clients.h"
#include "Game.h"
#include "MessageZone.h"

BOOL addClient(Data *data, TCHAR username[50], int id) {
  HANDLE hMutexClient;

  hMutexClient = OpenMutex(MUTEX_ALL_ACCESS, FALSE, CLIENTS_MUTEX);

  WaitForSingleObject(hMutexClient, INFINITE);

  int position = -1;

  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (data->clients[i].isEmpty) {
      position = i;
      break;
    }
  }

  if (position == -1) {
    errorGui(TEXT("Clients full"));
    ReleaseMutex(hMutexClient);
    return FALSE;
  }

  data->clients[position].isEmpty = FALSE;
  _tcscpy_s(data->clients[position].username, username);
  data->clients[position].id = id;

  ReleaseMutex(hMutexClient);

  return TRUE;
}

BOOL removeClient(Data *data, int id) {
  HANDLE hMutexClient;
  BOOL fSuccess = FALSE;

  hMutexClient = OpenMutex(MUTEX_ALL_ACCESS, FALSE, CLIENTS_MUTEX);

  WaitForSingleObject(hMutexClient, INFINITE);

  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (!data->clients[i].isEmpty) {
      if (data->clients[i].id == id) {
        data->clients[i] = {};
        data->clients[i].isEmpty = TRUE;
        fSuccess = TRUE;
        break;
      }
    }
  }

  ReleaseMutex(hMutexClient);

  if (!fSuccess) {
    return FALSE;
  }

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