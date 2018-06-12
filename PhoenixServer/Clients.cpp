#include "stdafx.h"

#include "Clients.h"
#include "MessageZone.h"

BOOL addClient(Data *data, TCHAR username[50], int n) {
  Game *game = &data->gameData->game;

  if (game->totalPlayers >= MAX_PLAYERS) {
    return FALSE;
  }

  _tcscpy_s(game->player[game->totalPlayers].username, username);
  game->player[game->totalPlayers].id = n;

  game->totalPlayers++;

  return TRUE;
}

void clientLogin(Data *data, Message message) {
  debug(TEXT("Login : %s %d"), message.text, message.number);
  if (!addClient(data, message.text, message.number)) {
    error(TEXT("Can't add client"));
  }

  message.cmd = LOGGED;

  /**
   * Player added : inform him
   */
  writeDataToSharedMemory(data->messageData->sharedMessage, &message,
                          sizeof(Message), data->messageData->hMutex,
                          data->messageData->gatewayMessageUpdateEvent);
}