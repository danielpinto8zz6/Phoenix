#include "stdafx.h"

#include "Clients.h"
#include "MessageZone.h"

BOOL addClient(Data *data, TCHAR username[50], int id) {
  Game *game = &data->gameData->game;

  if (game->totalPlayers >= MAX_PLAYERS) {
    return FALSE;
  }

  _tcscpy_s(game->player[game->totalPlayers].username, username);
  game->player[game->totalPlayers].id = id;

  game->totalPlayers++;

  return TRUE;
}

int getClientIndex(Game *game, int id) {
  for (int i = 0; i < game->totalPlayers; i++) {
    if (game->player[i].id == id) {
      return i;
    }
  }
  return -1;
}

BOOL removeClient(Game *game, int id) {
  int n = getClientIndex(game, id);

  if (n == -1) {
    return FALSE;
  }

  MessageBox(NULL, game->player[n].username, TEXT("Client disconnected"),
             MB_OK | MB_ICONINFORMATION);

  for (int i = n; i < game->totalPlayers; i++) {
    game->player[i] = game->player[i + 1];
  }

  game->totalPlayers--;

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