#include "stdafx.h"

#include "Clients.h"
#include "MessageZone.h"

VOID startClients(HANDLE listClients[MAXCLIENTS]) {
  for (int i = 0; i < MAXCLIENTS; i++) {
    listClients[i] = NULL;
  }
}

BOOL addClient(HANDLE listClients[MAXCLIENTS], HANDLE client) {
  for (int i = 0; i < MAXCLIENTS; i++) {
    if (listClients[i] == NULL) {
      listClients[i] = client;
      return TRUE;
    }
  }
  return FALSE;
}

BOOL removeClient(HANDLE clients[MAXCLIENTS], HANDLE client) {
  for (int i = 0; i < MAXCLIENTS; i++) {
    if (clients[i] == client) {
      clients[i] = NULL;
      return TRUE;
    }
  }
  return FALSE;
}

BOOL writeClientAsync(HANDLE hPipe, Game *game, HANDLE writeReady) {
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

int broadcastClients(HANDLE clients[MAXCLIENTS], Game *game,
                     HANDLE writeReady) {
  int nWrites = 0;
  for (int i = 0; i < MAXCLIENTS; i++) {
    if (clients[i] != 0) {
      if (writeClientAsync(clients[i], game, writeReady))
        nWrites++;
    }
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

  startClients(data->clients);

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

  if (hPipe == NULL) {
    error(TEXT("Can't access client pipe"));
    return FALSE;
  }

  readReady = CreateEvent(NULL, TRUE, FALSE, NULL);

  if (readReady == NULL) {
    error(TEXT("Can't create read event"));
    return FALSE;
  }

  addClient(data->clients, hPipe);

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
      writeDataToSharedMemory(data->messageData->sharedMessage, &message,
                              sizeof(Message), &data->messageData->hMutex,
                              data->messageData->serverMessageUpdateEvent);
      if (message.cmd == CLIENT_CLOSING) {
        break;
      }
    }
  }

  removeClient(data->clients, hPipe);

  FlushFileBuffers(hPipe);
  DisconnectNamedPipe(hPipe);
  CloseHandle(hPipe);

  debug(TEXT("Client thread exiting..."));

  return TRUE;
}