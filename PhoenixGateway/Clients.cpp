#include "stdafx.h"

#include "Clients.h"
#include "MessageZone.h"

DWORD WINAPI manageClients(LPVOID lpParam) {
  Data *data = (Data *)lpParam;
  /**
   * TOTAL : total of clients connected
   */
  BOOL STOP = FALSE;
  BOOL result;
  HANDLE hThreadManageClient[PLAYERS];
  data->totalClients = 0;

  while (!STOP && data->totalClients < PLAYERS) {

    Debug(TEXT("Creating an instance of a named pipe..."));
    // outbound server->client
    data->hClientPipe[data->totalClients] = CreateNamedPipe(
        PIPE_NAME_INBOUND, PIPE_ACCESS_OUTBOUND,
        PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, PLAYERS,
        50 * sizeof(TCHAR), 50 * sizeof(TCHAR), 1000, NULL);

    if (data->hClientPipe[data->totalClients] == NULL ||
        data->hClientPipe[data->totalClients] == INVALID_HANDLE_VALUE) {
      Error(TEXT("Failed to create outbound pipe instance.\n"));
      // look up error code here using GetLastError()
      system("pause");
      return -1;
    }

    // inbound server<-client
    data->hGatewayPipe = CreateNamedPipe(
        PIPE_NAME_OUTBOUND, PIPE_ACCESS_INBOUND,
        PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, PLAYERS,
        50 * sizeof(TCHAR), 50 * sizeof(TCHAR), 1000, NULL);
    if (data->hGatewayPipe == NULL ||
        data->hGatewayPipe == INVALID_HANDLE_VALUE) {
      Error(TEXT("Failed to create inbound pipe instance.\n"));
      system("pause");
      return -1;
    }

    Debug(TEXT("Waiting for a client to connect..."));

    // This call blocks until a client process connects to the pipe
    result = ConnectNamedPipe(data->hClientPipe[data->totalClients], NULL);
    if (!result) {
      Error(TEXT("Failed to make connection on named pipe.\n"));
      CloseHandle(data->hClientPipe[data->totalClients]); // close the pipe
      system("pause");
      return -1;
    }

    /**
     * Client connected, create thread
     */
    hThreadManageClient[data->totalClients] = CreateThread(
        NULL, 0, (LPTHREAD_START_ROUTINE)manageClient, (LPVOID)data, 0, NULL);
    if (hThreadManageClient[data->totalClients] == NULL) {
      Error(TEXT("Creating client thread"));
      system("pause");
      return -1;
    }
    data->totalClients++;
  }

  WaitForMultipleObjects(data->totalClients, hThreadManageClient, TRUE,
                         INFINITE);

  // Shutdown each named pipe
  for (int i = 0; i < data->totalClients; i++) {
    DisconnectNamedPipe(data->hClientPipe[i]);
    Debug(TEXT("Closing client %d pipe"), i);
    CloseHandle(data->hClientPipe[i]);
  }

  return 0;
}

DWORD WINAPI manageClient(LPVOID lpParam) {
  Data *data = (Data *)lpParam;

  MessageData *messageData = data->messageData;

  BOOL result;
  DWORD nBytes;
  BOOL STOP = FALSE;

  do {
    result = ReadFile(data->hGatewayPipe, (LPVOID)&messageData->message,
                      sizeof(Message), &nBytes, NULL);
    if (nBytes > 0) {
      sendMessageToServer(data->messageData, &messageData->message);
    }
  } while (!STOP);

  return 0;
}

BOOL sendMessageToAllClients(Data *data, Message *message) {
  BOOL result = FALSE;

  for (int i = 0; i < data->totalClients; i++) {
    if (data->hClientPipe[i] != NULL)
      result = sendMessageToClient(data->hClientPipe[i], message);
  }
  return result;
}

BOOL sendMessageToClient(HANDLE hClientPipe, Message *message) {
  BOOL result;
  DWORD nBytes;

  result =
      WriteFile(hClientPipe, (LPCVOID)message, sizeof(Message), &nBytes, NULL);
  if (!result) {
    Error(TEXT("Failed to send data to client."));
    return FALSE;
  }
  Debug(TEXT("%d Bytes sent to client"), nBytes);
  return TRUE;
}