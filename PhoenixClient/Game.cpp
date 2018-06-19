#include "stdafx.h"

#include "Communication.h"
#include "Game.h"

BOOL startGame(Client *client) {
  Game game;

  HANDLE hThreadGameReceiver;
  DWORD threadGameReceiverId = 0;

  client->game = &game;
  client->gameStarted = TRUE;

  /**
   * Create thread to receive game from gateway
   */
  hThreadGameReceiver =
      CreateThread(NULL, 0, gameReceiver, &client, 0, &threadGameReceiverId);
  if (hThreadGameReceiver == NULL) {
    errorGui(TEXT("Creating data receiver thread"));
    return FALSE;
  }

  WaitForSingleObject(hThreadGameReceiver, INFINITE);

  CloseHandle(hThreadGameReceiver);

  return TRUE;
}