﻿#include "stdafx.h"

#include "Clients.h"
#include "MessageZone.h"

int getClientIndex(Data *data, int clientId) {
  for (int i = 0; i < data->totalClients; i++) {
    if (data->client[i].id == clientId) {
      return i;
    }
  }
  return -1;
}

BOOL removeClient(Data *data, int clientId) {
  int n = getClientIndex(data, clientId);

  if (n == -1) {
    return FALSE;
  }

  for (int i = n; i < data->totalClients; i++) {
    data->client[i] = data->client[i + 1];
  }

  data->totalClients--;

  return TRUE;
}

int broadcastGameToClients(Data *data, Game *game) {
  int nWrites = 0;
  for (int i = 0; i < data->totalClients; i++) {
    if (writeDataToPipe(data->client[i].hPipeGame, (LPVOID)game, sizeof(Game)))
      nWrites++;
  }
  return nWrites;
}

int broadcastMessageToClients(Data *data, Message *message) {
  int nWrites = 0;
  for (int i = 0; i < data->totalClients; i++) {
    if (writeDataToPipe(data->client[i].hPipeMessage, (LPVOID)message,
                        sizeof(Message)))
      nWrites++;
  }
  return nWrites;
}

DWORD WINAPI manageClients(LPVOID lpParam) {
  Data *data = (Data *)lpParam;

  if (data == NULL) {
    error(TEXT("Can't receive data"));
    return FALSE;
  }

  data->totalClients = 0;

  BOOL fConnectedGame = FALSE;
  BOOL fConnectedMessage = FALSE;
  DWORD dwThreadId = 0;
  // HANDLE hPipeGame = INVALID_HANDLE_VALUE;
  HANDLE hPipeMessage = INVALID_HANDLE_VALUE;

  HANDLE hThread = NULL;

  while (TRUE) {
    hPipeMessage = CreateNamedPipe(
        PIPE_MESSAGE_NAME, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES, sizeof(Message), sizeof(Message), 5000, NULL);

    // hPipeGame = CreateNamedPipe(
    //     PIPE_GAME_NAME, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
    //     PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
    //     PIPE_UNLIMITED_INSTANCES, sizeof(Game), sizeof(Game), 5000, NULL);

    if (hPipeMessage == INVALID_HANDLE_VALUE /**||
        hPipeGame == INVALID_HANDLE_VALUE*/) {
      error(TEXT("Failed creating named pipe"));
      return FALSE;
    }

    debug(TEXT("Waiting for a client to connect..."));

    /**
     * != 0 means success
     * ERROR_PIPE_CONNECTED means success too
     */
    fConnectedMessage = ConnectNamedPipe(hPipeMessage, NULL)
                            ? TRUE
                            : (GetLastError() == ERROR_PIPE_CONNECTED);

    // fConnectedGame = ConnectNamedPipe(hPipeGame, NULL)
    //                      ? TRUE
    //                      : (GetLastError() == ERROR_PIPE_CONNECTED);

    if (/**fConnectedGame && */ fConnectedMessage) {
      data->tmpPipeMessage = hPipeMessage;
      // data->tmpPipeGame = hPipeGame;
      hThread =
          CreateThread(NULL, 0, manageClient, (LPVOID)data, 0, &dwThreadId);

      if (hThread == NULL) {
        error(TEXT("Failed to create thread for client"));
        return FALSE;
      } else {
        CloseHandle(hThread);
      }
    } else {
      CloseHandle(hPipeMessage);
      // CloseHandle(hPipeGame);
    }
  }
  return TRUE;
}

DWORD WINAPI manageClient(LPVOID lpParam) {
  Data *data = (Data *)lpParam;

  if (data == NULL) {
    error(TEXT("Can't receive data"));
    return FALSE;
  }

  int i = data->totalClients;
  data->totalClients++;

  int clientId;
  clientId = GetCurrentThreadId();
  data->client[i].id = clientId;

  HANDLE hPipeGame = data->tmpPipeGame;
  HANDLE hPipeMessage = data->tmpPipeMessage;

  DWORD nBytes = 0;
  BOOL fSuccess = FALSE;

  Message message;

  if (hPipeGame == NULL || hPipeMessage == NULL) {
    error(TEXT("Can't access client pipe"));
    return FALSE;
  }

  while (TRUE) {
    fSuccess =
        readDataFromPipe(hPipeMessage, (LPVOID)&message, sizeof(Message));

    if (!fSuccess) {
      error(TEXT("Can't read message data"));
      break;
    }

    message.clientId = clientId;

    writeDataToSharedMemory(data->messageData->sharedMessage, &message,
                            sizeof(Message), &data->messageData->hMutex,
                            data->messageData->serverMessageUpdateEvent);

    if (message.cmd == CLIENT_CLOSING) {
      break;
    }
  }

  removeClient(data, clientId);
  FlushFileBuffers(hPipeGame);
  DisconnectNamedPipe(hPipeGame);
  CloseHandle(hPipeGame);
  FlushFileBuffers(hPipeMessage);
  DisconnectNamedPipe(hPipeMessage);
  // FlushFileBuffers(hPipeGame);
  // DisconnectNamedPipe(hPipeGame);
  // CloseHandle(hPipeGame);

  return TRUE;
  CloseHandle(hPipeMessage);
}