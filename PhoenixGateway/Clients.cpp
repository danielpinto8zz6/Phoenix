#include "stdafx.h"

#include "Clients.h"
#include "MessageZone.h"

int getClientIndex(Data *data, int id) {
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (!data->client[i].isEmpty) {
      if (data->client[i].id == id) {
        return i;
      }
    }
  }
  return -1;
}

BOOL removeClient(Data *data, int id) {
  HANDLE hMutexClient;
  BOOL fSuccess = FALSE;
  Message message;

  hMutexClient = OpenMutex(MUTEX_ALL_ACCESS, FALSE, GATEWAY_CLIENTS_MUTEX);

  WaitForSingleObject(hMutexClient, INFINITE);

  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (!data->client[i].isEmpty) {
      if (data->client[i].id == id) {
        data->client[i] = {};
        data->client[i].isEmpty = TRUE;
        fSuccess = TRUE;
        break;
      }
    }
  }

  ReleaseMutex(hMutexClient);

  if (!fSuccess) {
    return FALSE;
  }

  /**
   * Inform server
   */
  message.cmd = CLIENT_DISCONNECTED;
  message.clientId = id;

  writeDataToSharedMemory(data->messageData->sharedMessage, &message,
                          sizeof(Message), data->messageData->hMutex,
                          data->messageData->serverMessageUpdateEvent);

  return TRUE;
}

int broadcastGameToClients(Data *data, Game *game) {
  int nWrites = 0;
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (!data->client[i].isEmpty) {
      if (writeDataToPipeAsync(data->client[i].hPipeGame, data->hEvent, game,
                               sizeof(Game)))
        nWrites++;
    }
  }
  return nWrites;
}

int broadcastMessageToClients(Data *data, Message *message) {
  int nWrites = 0;
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (!data->client[i].isEmpty) {

      if (writeDataToPipeAsync(data->client[i].hPipeMessage, data->hEvent,
                               message, sizeof(Message)))
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

  BOOL fConnectedGame = FALSE;
  BOOL fConnectedMessage = FALSE;
  DWORD dwThreadId = 0;
  HANDLE hPipeGame = INVALID_HANDLE_VALUE;
  HANDLE hPipeMessage = INVALID_HANDLE_VALUE;

  HANDLE hThread = NULL;

  data->hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
  if (data->hEvent == NULL) {
    error(TEXT("Can't create write event"));
  }

  while (TRUE) {
    hPipeMessage = CreateNamedPipe(
        PIPE_MESSAGE_NAME, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES, sizeof(Message), sizeof(Message), 5000, NULL);

    hPipeGame = CreateNamedPipe(
        PIPE_GAME_NAME, PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES, sizeof(Game), sizeof(Game), 5000, NULL);

    if (hPipeMessage == INVALID_HANDLE_VALUE ||
        hPipeGame == INVALID_HANDLE_VALUE) {
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

    fConnectedGame = ConnectNamedPipe(hPipeGame, NULL)
                         ? TRUE
                         : (GetLastError() == ERROR_PIPE_CONNECTED);

    if (fConnectedGame && fConnectedMessage) {
      data->tmpPipeMessage = hPipeMessage;
      data->tmpPipeGame = hPipeGame;

      hThread = CreateThread(NULL, 0, manageClient, data, 0, &dwThreadId);

      if (hThread == NULL) {
        error(TEXT("Failed to create thread for client"));
        return FALSE;
      } else {
        CloseHandle(hThread);
      }
    } else {
      CloseHandle(hPipeMessage);
      CloseHandle(hPipeGame);
    }
  }
  return TRUE;
}

DWORD WINAPI manageClient(LPVOID lpParam) {
  Data *data = (Data *)lpParam;

  HANDLE readReady;
  OVERLAPPED OverlRd = {0};

  if (data == NULL) {
    error(TEXT("Can't receive data"));
    return FALSE;
  }

  int id = GetCurrentThreadId();

  int position = addClient(data, id, data->tmpPipeMessage, data->tmpPipeGame);

  if (position == -1) {
    error(TEXT("Can't find a valid slot"));
    return FALSE;
  }

  DWORD nBytes = 0;
  BOOL fSuccess = FALSE;

  Message message;

  if (data->client[position].hPipeGame == NULL ||
      data->client[position].hPipeGame == NULL) {
    error(TEXT("Can't access client pipe"));
    return FALSE;
  }

  readReady = CreateEvent(NULL, TRUE, FALSE, NULL);
  if (readReady == NULL) {
    error(TEXT("Can't create read event!"));
    return FALSE;
  }

  while (TRUE) {
    fSuccess = readDataFromPipeAsync(data->client[position].hPipeMessage,
                                     readReady, &message, sizeof(Message));

    if (!fSuccess) {
      if (GetLastError() == ERROR_BROKEN_PIPE) {
        error(TEXT("Client %d disconnected! Removing him..."), id);
      } else {
        error(TEXT("Can't read message data"));
      }
      break;
    }

    /**
     * We identify each client with unique id, to generate that id we use the
     * gateway client thread id. Each client thread has different thread id
     */
    message.clientId = id;

    writeDataToSharedMemory(data->messageData->sharedMessage, &message,
                            sizeof(Message), data->messageData->hMutex,
                            data->messageData->serverMessageUpdateEvent);

    if (message.cmd == CLIENT_DISCONNECTED) {
      break;
    }
  }

  FlushFileBuffers(data->client[position].hPipeMessage);
  DisconnectNamedPipe(data->client[position].hPipeMessage);
  CloseHandle(data->client[position].hPipeMessage);
  FlushFileBuffers(data->client[position].hPipeGame);
  DisconnectNamedPipe(data->client[position].hPipeGame);
  CloseHandle(data->client[position].hPipeGame);
  removeClient(data, id);

  return TRUE;
}

int addClient(Data *data, int id, HANDLE hPipeMessage, HANDLE hPipeGame) {
  HANDLE hMutexClient;

  hMutexClient = OpenMutex(MUTEX_ALL_ACCESS, FALSE, GATEWAY_CLIENTS_MUTEX);

  WaitForSingleObject(hMutexClient, INFINITE);

  int position = -1;

  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (data->client[i].isEmpty) {
      position = i;
      break;
    }
  }

  if (position == -1) {
    error(TEXT("Can't connect client! Clients full"));
    ReleaseMutex(hMutexClient);
    return -1;
  }

  data->client[position].isEmpty = FALSE;
  data->client[position].id = id;
  data->client[position].hPipeMessage = hPipeMessage;
  data->client[position].hPipeGame = hPipeGame;

  ReleaseMutex(hMutexClient);

  return position;
}
