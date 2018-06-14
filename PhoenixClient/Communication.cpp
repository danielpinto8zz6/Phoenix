#include "stdafx.h"

#include "Communication.h"
#include "Game.h"
#include "PhoenixClient.h"
#include "resource.h"
#include <process.h>

BOOL connectPipes(Client *client) {
  DWORD dwMode;
  BOOL fSuccess;

  while (TRUE) {
    client->hPipeMessage = CreateFile(
        PIPE_MESSAGE_NAME, GENERIC_READ | GENERIC_WRITE,
        0 | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

    if (client->hPipeMessage != INVALID_HANDLE_VALUE) {
      break;
    }

    if (GetLastError() != ERROR_PIPE_BUSY) {
      errorGui(TEXT("Can't create file"));
      return FALSE;
    }

    /**
     * Wait 30 sec
     */
    if (!WaitNamedPipe(PIPE_MESSAGE_NAME, 30000)) {
      errorGui(TEXT("Timeout! Exiting..."));
      return FALSE;
    }
  }

  while (TRUE) {
    client->hPipeGame = CreateFile(
        PIPE_GAME_NAME, GENERIC_READ, 0 | FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, OPEN_EXISTING, 0, NULL);

    if (client->hPipeGame != INVALID_HANDLE_VALUE) {
      break;
    }

    if (GetLastError() != ERROR_PIPE_BUSY) {
      errorGui(TEXT("Can't create file"));
      return FALSE;
    }

    if (!WaitNamedPipe(PIPE_GAME_NAME, 30000)) {
      errorGui(TEXT("Timeout! Exiting..."));
      return FALSE;
    }
  }

  debug(TEXT("Pipes connected!"));

  /**
   * Connected! Change pipe mode to read
   */
  dwMode = PIPE_READMODE_MESSAGE;
  fSuccess = SetNamedPipeHandleState(client->hPipeMessage, &dwMode, NULL, NULL);

  if (!fSuccess) {
    errorGui(TEXT("Can't set message named pipe handle state"));
    return FALSE;
  }

  return TRUE;
}

DWORD WINAPI gameReceiver(LPVOID lpParam) {
  Client *client = (Client *)lpParam;

  DWORD nBytes = 0;
  BOOL fSuccess = FALSE;

  Game game;

  if (client->hPipeGame == NULL) {
    errorGui(TEXT("Game pipe is NULL"));
    return FALSE;
  }

  while (client->threadContinue) {
    fSuccess = readDataFromPipe(client->hPipeGame, &game, sizeof(Game));

    if (!fSuccess) {
      error(TEXT("Can't read message data"));
      break;
    }

    // TODO
  }

  return FALSE;
}

DWORD WINAPI messageReceiver(LPVOID lpParam) {
  Client *client = (Client *)lpParam;

  DWORD nBytes = 0;
  BOOL fSuccess = FALSE;

  Message message;

  if (client->hPipeMessage == NULL) {
    errorGui(TEXT("Message pipe is NULL"));
    return FALSE;
  }

  while (client->threadContinue) {
    fSuccess =
        readDataFromPipe(client->hPipeMessage, &message, sizeof(Message));

    if (!fSuccess) {
      error(TEXT("Can't read message data"));
      break;
    }

    // TODO
  }

  return FALSE;
}