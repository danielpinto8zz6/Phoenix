// phoenix_client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "../phoenix_dll/phoenix_dll.h"
#include "../phoenix_dll/structs.h"
#include "phoenix_client.h"
#include <fcntl.h>
#include <io.h>
#include <tchar.h>
#include <windows.h>

int _tmain() {
  DWORD threadDataReceiverId;
  HANDLE hThreadDataReceiver;

#ifdef UNICODE
  _setmode(_fileno(stdin), _O_WTEXT);
  _setmode(_fileno(stdout), _O_WTEXT);
#endif

  /**
   * In this thread we receive info from the gateway trought the specific client
   * pipe
   */
  hThreadDataReceiver =
      CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)dataReceiver, 0, 0,
                   &threadDataReceiverId);
  if (hThreadDataReceiver == NULL) {
    _tprintf(TEXT("[Erro] Criar thread"));
    return -1;
  }

  WaitForSingleObject(hThreadDataReceiver, INFINITE);

  system("pause");

  return 0;
}

/**
 *  Receive data from gateway
 */
LPVOID dataReceiver() {
  HANDLE hServerPipe;
  BOOL fSuccess = FALSE;
  DWORD n;
  LPVOID data;
  // Temporary
  BOOL STOP = FALSE;
  int clientId = 0;
  TCHAR clientPipeName[50];

  _stprintf_s(clientPipeName, 50, TEXT("PHOENIX-CLIENT-%d"), 0);

  if (!WaitNamedPipe(clientPipeName, NMPWAIT_WAIT_FOREVER)) {
    _tprintf(TEXT("[ERRO] Ligar ao pipe '%s'! (WaitNamedPipe)\n"),
             clientPipeName);
    system("pause");
    exit(-1);
  }

  hServerPipe = CreateFile(clientPipeName, GENERIC_READ, 0, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hServerPipe == NULL) {
    _tprintf(TEXT("[ERRO] Ligar ao pipe '%s'! (CreateFile)\n"), clientPipeName);
    system("pause");
    exit(-1);
  }

  // Connected
  do {
    fSuccess = ReadFile(hServerPipe, &data, sizeof(data), &n, NULL);

    if (!fSuccess || !n) {
      _tprintf(TEXT("[ERRO] %d %d... (ReadFile)\n"), fSuccess, n);
      break;
    }

    // Do what you gotta do...
  } while (!STOP);

  CloseHandle(hServerPipe);

  return 0;
}

/**
 * Send specified data to gateway
 */
LPVOID sendDataToGateway(LPVOID data) {
  DWORD n;
  HANDLE hGateWayPipe;

  hGateWayPipe =
      CreateNamedPipe(GATEWAY_PIPE_NAME, PIPE_ACCESS_OUTBOUND,
                      PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, 1,
                      sizeof(data), sizeof(data), 1000, NULL);
  if (hGateWayPipe == INVALID_HANDLE_VALUE) {
    _tprintf(TEXT("[ERRO] Criar Named Pipe! %d\n"), GetLastError());
    system("pause");
    exit(-1);
  }

  while (1) {
    if (!ConnectNamedPipe(hGateWayPipe, NULL)) {
      _tprintf(TEXT("[ERRO] Ligação ao leitor! %d\n"), GetLastError());
      system("pause");
      exit(-1);
    }
    if (!WriteFile(hGateWayPipe, &data, sizeof(data), &n, NULL)) {
      _tprintf(TEXT("[ERRO] Escrever no pipe! (WriteFile)\n"));
      exit(-1);
    }

    if (!DisconnectNamedPipe(hGateWayPipe)) {
      _tprintf(TEXT("[ERRO] Desligar o pipe! %d"), GetLastError());
      system("pause");
      exit(-1);
    }
  }
  CloseHandle(hGateWayPipe);
}