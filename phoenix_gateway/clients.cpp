#include "stdafx.h"

#include "../phoenix_dll/structs.h"
#include "clients.h"

DWORD WINAPI manageClients(LPVOID lpParam) {
  /**
   * Temporary : Max players
   */
  const int MAXPLAYERS = 10;
  /**
   * TOTAL : total of clients connected
   */
  int TOTAL = 0;
  BOOL STOP = FALSE;
  BOOL result;
  HANDLE hGatewayPipe;
  HANDLE clientPipe[MAXPLAYERS];
  HANDLE hThreadManageClient[MAXPLAYERS];

  while (!STOP && TOTAL < MAXPLAYERS) {

    _tprintf(TEXT("Creating an instance of a named pipe...\n"));

    // outbound server->client
    clientPipe[TOTAL] = CreateNamedPipe(
        PIPE_NAME_INBOUND, PIPE_ACCESS_OUTBOUND,
        PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, MAXPLAYERS,
        50 * sizeof(TCHAR), 50 * sizeof(TCHAR), 1000, NULL);

    if (clientPipe[TOTAL] == NULL ||
        clientPipe[TOTAL] == INVALID_HANDLE_VALUE) {
      _tprintf(TEXT("Failed to create outbound pipe instance.\n"));
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
      _tprintf(TEXT("Failed to create inbound pipe instance.\n"));
      system("pause");
      return -1;
    }

    _tprintf(TEXT("Waiting for a client to connect...\n"));

    // This call blocks until a client process connects to the pipe
    result = ConnectNamedPipe(clientPipe[TOTAL], NULL);
    if (!result) {
      _tprintf(TEXT("Failed to make connection on named pipe.\n"));
      CloseHandle(clientPipe[TOTAL]); // close the pipe
      system("pause");
      return -1;
    }

    ClientPipes clientPipes;
    clientPipes.inboundPipe = hGatewayPipe;
    clientPipes.outboundPipe = clientPipe[TOTAL];

    /**
     * Client connected, create thread
     */
    hThreadManageClient[TOTAL] =
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)manageClient,
                     (LPVOID)&clientPipes, 0, NULL);
    if (hThreadManageClient[TOTAL] == NULL) {
      _tprintf(TEXT("[Erro] Criar thread"));
      return -1;
    }
    TOTAL++;
  }

  WaitForMultipleObjects(TOTAL, hThreadManageClient, TRUE, INFINITE);

  // Shutdown each named pipe
  for (int i = 0; i < TOTAL; i++) {
    DisconnectNamedPipe(clientPipe[i]);
    _tprintf(TEXT("Desligar o pipe... (CloseHandle)\n"));
    CloseHandle(clientPipe[i]);
  }

  return 0;
}

DWORD WINAPI manageClient(LPVOID lpParam) {
  ClientPipes *clientPipes;
  clientPipes = (ClientPipes *)lpParam;
  Message msg;
  BOOL result;
  DWORD nBytes;
  // Fase 1 - o servidor aguardo o login do jogador
  do {
    result = ReadFile(clientPipes->inboundPipe, (LPVOID)&msg, sizeof(msg),
                      &nBytes, NULL);
    if (nBytes > 0) {
      switch (msg.cmd) {
      case LOGIN:
        _tprintf(TEXT("CLIENT LOGIN"));
        break;
      }
      result = WriteFile(clientPipes->outboundPipe, (LPCVOID)&msg, sizeof(msg),
                         &nBytes, NULL);
      if (!result) {
        _tprintf(TEXT("Failed to send data."));
      }
    }
  } while (!msg.Stop);

  return 0;
}