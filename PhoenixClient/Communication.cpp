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
    client->hPipeMessage =
        CreateFile(PIPE_MESSAGE_NAME, GENERIC_READ | GENERIC_WRITE,
                   0 | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
                   0 | FILE_FLAG_OVERLAPPED, NULL);

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
    client->hPipeGame = CreateFile(PIPE_GAME_NAME, GENERIC_READ,
                                   0 | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                                   OPEN_EXISTING, 0, NULL);

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

DWORD WINAPI messageReceiver(LPVOID lpParam) {
  Client *client = (Client *)lpParam;

  DWORD nBytes = 0;
  BOOL fSuccess = FALSE;

  Message message;

  HANDLE readReady;
  OVERLAPPED OverlRr = {0};

  if (client->hPipeMessage == NULL) {
    errorGui(TEXT("Message pipe is NULL"));
    return FALSE;
  }

  readReady = CreateEvent(NULL, TRUE, FALSE, NULL);
  if (readReady == NULL) {
    errorGui(TEXT("Can't create reader event!"));
    return FALSE;
  }

  client->readerAlive = TRUE;

  while (client->threadContinue) {
    fSuccess = readDataFromPipeAsync(client->hPipeMessage, readReady, &message,
                                     sizeof(Message));

    if (!fSuccess) {
      if (GetLastError() == ERROR_BROKEN_PIPE) {
        errorGui(TEXT("Gateway disconnected! Can't obtain data!"));
      } else {
        errorGui(TEXT("Can't read message data"));
      }
      break;
    }

    handleCommand(client, message);
  }

  client->readerAlive = FALSE;

  return TRUE;
}

void handleCommand(Client *client, Message message) {
  switch (message.cmd) {
  // case GAME_STARTED:
  //   client->gameStarted = TRUE;
  //   InvalidateRect(client->hWnd, NULL, TRUE);
  //   break;
  case LOGGED:
    MessageBox(NULL, message.text, TEXT("Login succeed"),
               MB_OK | MB_ICONINFORMATION);
    client->logged = TRUE;
    client->id = message.clientId;
    break;

  case IN_GAME:
    MessageBox(NULL, TEXT("Joined to game!"), TEXT("Joined"),
               MB_OK | MB_ICONINFORMATION);
    client->inGame = TRUE;
    InvalidateRect(client->hWnd, NULL, TRUE);
    break;

  case CANT_JOIN:
    MessageBox(NULL, TEXT("Can't join to game!"), TEXT("Error"),
               MB_OK | MB_ICONINFORMATION);
    client->inGame = TRUE;
    break;
  }
}

BOOL joinGame(Client *client) {
  Message message;

  message.cmd = JOIN_GAME;

  _tcscpy_s(message.text, client->username);

  return writeDataToPipeAsync(client->hPipeMessage, client->hEvent, &message,
                              sizeof(Message));
}

DWORD WINAPI gameReceiver(LPVOID lpParam) {
  Client *client = (Client *)lpParam;
  Game *game = &client->game;

  DWORD nBytes = 0;
  BOOL fSuccess = FALSE;

  if (client->hPipeGame == NULL) {
    errorGui(TEXT("Game pipe is NULL"));
    return FALSE;
  }

  while (client->threadContinue) {
    if (client->logged) {
      fSuccess = readDataFromPipe(client->hPipeGame, game, sizeof(Game));

      if (!fSuccess) {
        // Ignore it because messagereceiver already handles broken pipe
        // error... if (GetLastError() == ERROR_BROKEN_PIPE) {
        //   errorGui(TEXT("Gateway disconnected! Can't obtain data!"));
        // }

        errorGui(TEXT("Can't read game data"));
        break;
      }

      InvalidateRect(client->hWnd, NULL, FALSE);
    }
  }

  return FALSE;
}