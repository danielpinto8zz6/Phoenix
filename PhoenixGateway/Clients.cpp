#include "stdafx.h"

#include "Clients.h"
#include "MessageZone.h"

#define MAXPLAYERS 10

DWORD WINAPI manageClients(LPVOID lpParam) {
  Data *data = (Data *)lpParam;
  /**
   * TOTAL : total of clients connected
   */
  int TOTAL = 0;
  BOOL STOP = FALSE;
  BOOL result;
  HANDLE hGatewayPipe;
  HANDLE clientPipe[MAXPLAYERS];
  HANDLE hThreadManageClient[MAXPLAYERS];
  Pipes pipes;

  while (!STOP && TOTAL < MAXPLAYERS) {

    _tprintf(TEXT("Creating an instance of a named pipe...\n"));
    // outbound server->client
    clientPipe[TOTAL] = CreateNamedPipe(
        PIPE_NAME_INBOUND, PIPE_ACCESS_OUTBOUND,
        PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, MAXPLAYERS,
        50 * sizeof(TCHAR), 50 * sizeof(TCHAR), 1000, NULL);

    if (clientPipe[TOTAL] == NULL ||
        clientPipe[TOTAL] == INVALID_HANDLE_VALUE) {
      Error(TEXT("Failed to create outbound pipe instance.\n"));
      // look up error code here using GetLastError()
      system("pause");
      return -1;
    }

    // inbound server<-client
    hGatewayPipe = CreateNamedPipe(
        PIPE_NAME_OUTBOUND, PIPE_ACCESS_INBOUND,
        PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, MAXPLAYERS,
        50 * sizeof(TCHAR), 50 * sizeof(TCHAR), 1000, NULL);
    if (hGatewayPipe == NULL || hGatewayPipe == INVALID_HANDLE_VALUE) {
      Error(TEXT("Failed to create inbound pipe instance.\n"));
      system("pause");
      return -1;
    }

    _tprintf(TEXT("Waiting for a client to connect...\n"));

    // This call blocks until a client process connects to the pipe
    result = ConnectNamedPipe(clientPipe[TOTAL], NULL);
    if (!result) {
      Error(TEXT("Failed to make connection on named pipe.\n"));
      CloseHandle(clientPipe[TOTAL]); // close the pipe
      system("pause");
      return -1;
    }

    pipes.inboundPipe = hGatewayPipe;
    pipes.outboundPipe = clientPipe[TOTAL];

    Client client;
    client.pipes = &pipes;

    data->messageData->message.client = &client;

    /**
     * Client connected, create thread
     */
    hThreadManageClient[TOTAL] = CreateThread(
        NULL, 0, (LPTHREAD_START_ROUTINE)manageClient, (LPVOID)data, 0, NULL);
    if (hThreadManageClient[TOTAL] == NULL) {
      Error(TEXT("Creating client thread"));
      system("pause");
      return -1;
    }
    TOTAL++;
  }

  WaitForMultipleObjects(TOTAL, hThreadManageClient, TRUE, INFINITE);

  // Shutdown each named pipe
  for (int i = 0; i < TOTAL; i++) {
    DisconnectNamedPipe(clientPipe[i]);
    _tprintf(TEXT("Closing pipe (CloseHandle)\n"));
    CloseHandle(clientPipe[i]);
  }

  return 0;
}

DWORD WINAPI manageClient(LPVOID lpParam) {
  int currentMsg = 0;
  Data *data = (Data *)lpParam;

  Client *client;
  client = data->messageData->message.client;

  Message msg;
  msg.client = client;

  BOOL result;
  DWORD nBytes;
  BOOL STOP = FALSE;

  do {
    result = ReadFile(client->pipes->inboundPipe, (LPVOID)&msg, sizeof(msg),
                      &nBytes, NULL);
    if (nBytes > 0) {
      // msg.num = data->messageData->currrentMessage ++;
      // sendMessageToServer(data->messageData, &msg);
      switch (msg.cmd) {
      case LOGIN:
        _tcscpy_s(client->username, _tcslen(msg.text) + 1, msg.text);
        clientLogged(client);
        _tprintf(TEXT("Client Login : %s\n"), client->username);

        /**
         * Tell client he's logged
         */
        msg.cmd = LOGGED;
        result = WriteFile(client->pipes->outboundPipe, (LPCVOID)&msg,
                           sizeof(msg), &nBytes, NULL);
        if (!result) {
          Error(TEXT("Failed to send data to client."));
        }

        break;
      }
    }
  } while (!STOP);

  return 0;
}

VOID clientLogged(Client *client) {
  _tprintf(TEXT("%s logged...\n"), client->username);
}