﻿#include "stdafx.h"

#include "Clients.h"
#include "MessageZone.h"

int getClientIndex(Data *data, HANDLE client) {
  for (int i = 0; i < data->totalClients; i++) {
    if (data->client[i].hPipe == client) {
      return i;
    }
  }
  return -1;
}

BOOL removeClient(Data *data, HANDLE client) {
  int n = getClientIndex(data, client);

  if (n == -1) {
    return FALSE;
  }

  ZeroMemory(&data->client[n], sizeof(Client));

  for (int i = n; i < data->totalClients; i++) {
    data->client[i] = data->client[i + 1];
  }

  data->totalClients--;

  return TRUE;
}

BOOL addClient(Data *data, HANDLE client) {
  if (data->totalClients >= MAXCLIENTS - 1) {
    error(TEXT("Can't add more clients. Max exceed!"));
    return FALSE;
  }

  int n = data->totalClients;

  data->client[n].hPipe = client;
  data->totalClients++;

  return TRUE;
}

BOOL writeGameToClientAsync(HANDLE hPipe, Game *game, HANDLE writeReady) {
  DWORD nBytes;
  BOOL fSuccess = FALSE;

  OVERLAPPED OverlWr = {0};

  if (hPipe == NULL) {
    error(TEXT("Pipe is NULL"));
    return FALSE;
  }

  if (game == NULL) {
    error(TEXT("Game is NULL"));
    return FALSE;
  }

  ZeroMemory(&OverlWr, sizeof(OverlWr));

  ResetEvent(writeReady);
  OverlWr.hEvent = writeReady;

  fSuccess = WriteFile(hPipe, game, sizeof(Game), &nBytes, &OverlWr);

  WaitForSingleObject(writeReady, INFINITE);

  debug(TEXT("PIPES : %d bytes sent"), nBytes);

  GetOverlappedResult(hPipe, &OverlWr, &nBytes, FALSE);

  if (nBytes < sizeof(Game)) {
    error(TEXT("Write file didn't wrote all the info"));
  }

  return TRUE;
}

int broadcastGameToClients(Data *data, Game *game, HANDLE writeReady) {
  int nWrites = 0;
  for (int i = 0; i < data->totalClients; i++) {
    if (writeGameToClientAsync(data->client[i].hPipe, game, writeReady))
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

  BOOL fConnected = FALSE;
  DWORD dwThreadId = 0;
  HANDLE hPipe = INVALID_HANDLE_VALUE;
  HANDLE hThread = NULL;

  data->writeReady = CreateEvent(NULL, TRUE, FALSE, NULL);
  if (data->writeReady == NULL) {
    error(TEXT("Can't create write event"));
    return FALSE;
  }

  while (TRUE) {
    hPipe = CreateNamedPipe(
        PIPE_NAME, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES, sizeof(Game), sizeof(Game), 5000, NULL);

    if (hPipe == INVALID_HANDLE_VALUE) {
      error(TEXT("Failed creating named pipe"));
      return FALSE;
    }

    debug(TEXT("Waiting for a client to connect..."));

    /**
     * != 0 means success
     * ERROR_PIPE_CONNECTED means success too
     */
    fConnected = ConnectNamedPipe(hPipe, NULL)
                     ? TRUE
                     : (GetLastError() == ERROR_PIPE_CONNECTED);

    if (fConnected) {
      data->tmpPipe = hPipe;
      hThread =
          CreateThread(NULL, 0, manageClient, (LPVOID)data, 0, &dwThreadId);

      if (hThread == NULL) {
        error(TEXT("Failed to create thread for client"));
        return FALSE;
      } else {
        /**
         * Don't need the handler
         */
        CloseHandle(hThread);
      }
    } else {
      /**
       * Client can't connect. Close pipe instance
       */
      CloseHandle(hPipe);
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

  DWORD nBytes = 0;
  BOOL fSuccess = FALSE;
  HANDLE hPipe = data->tmpPipe;

  Message message;

  HANDLE readReady;
  OVERLAPPED OverlRd = {0};

  int clientId = GetCurrentThreadId();

  if (hPipe == NULL) {
    error(TEXT("Can't access client pipe"));
    return FALSE;
  }

  readReady = CreateEvent(NULL, TRUE, FALSE, NULL);

  if (readReady == NULL) {
    error(TEXT("Can't create read event"));
    return FALSE;
  }

  fSuccess = addClient(data, hPipe);

  if (!fSuccess) {
    error(TEXT("Can't add client to gateway client list"));
  }

  while (TRUE) {
    ZeroMemory(&OverlRd, sizeof(OverlRd));
    ResetEvent(readReady);
    OverlRd.hEvent = readReady;

    fSuccess = ReadFile(hPipe, &message, sizeof(Message), &nBytes, &OverlRd);

    WaitForSingleObject(readReady, INFINITE);

    GetOverlappedResult(hPipe, &OverlRd, &nBytes, FALSE);

    if (nBytes < sizeof(Message)) {
      error(TEXT("ReadFile can't read all data"));
    } else {
      message.clientId = clientId;
      writeDataToSharedMemory(data->messageData->sharedMessage, &message,
                              sizeof(Message), &data->messageData->hMutex,
                              data->messageData->serverMessageUpdateEvent);

      debug(TEXT("%d"), message.clientId);
      if (message.cmd == CLIENT_CLOSING) {
        break;
      }
    }
  }

  removeClient(data, hPipe);

  FlushFileBuffers(hPipe);
  DisconnectNamedPipe(hPipe);
  CloseHandle(hPipe);

  debug(TEXT("Client thread exiting..."));

  return TRUE;
}