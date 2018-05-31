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

  // Temporary
  DWORD data;

  hThreadDataReceiver =
      CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)dataReceiver, &data, 0,
                   &threadDataReceiverId);
  if (hThreadDataReceiver == NULL) {
    _tprintf(TEXT("[Erro] Criar thread"));
    return -1;
  }

  WaitForSingleObject(hThreadDataReceiver, INFINITE);

  system("pause");

  return 0;
}

// Receive data from server
unsigned int __stdcall dataReceiver() {
  TCHAR buf[256];
  HANDLE hServerPipe;
  BOOL fSuccess = FALSE;
  DWORD n;
  // Temporary
  BOOL STOP = FALSE;

  if (!WaitNamedPipe(SERVER_PIPE_NAME, NMPWAIT_WAIT_FOREVER)) {
    _tprintf(TEXT("[ERRO] Ligar ao pipe '%s'! (WaitNamedPipe)\n"),
             SERVER_PIPE_NAME);
    system("pause");
    exit(-1);
  }
  hServerPipe = CreateFile(SERVER_PIPE_NAME, GENERIC_READ, 0, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hServerPipe == NULL) {
    _tprintf(TEXT("[ERRO] Ligar ao pipe '%s'! (CreateFile)\n"),
             SERVER_PIPE_NAME);
    system("pause");
    exit(-1);
  }

  // Connected
  do {
    fSuccess = ReadFile(hServerPipe, buf, sizeof(buf), &n, NULL);
    buf[n / sizeof(TCHAR)] = '\0';

    if (!fSuccess || !n) {
      _tprintf(TEXT("[ERRO] %d %d... (ReadFile)\n"), fSuccess, n);
      break;
    }

    // Do what you gotta do...
  } while (!STOP);

  CloseHandle(hServerPipe);

  return 0;
}

void dataSender(int data) {
  DWORD n;
  HANDLE hClientPipe;
  TCHAR buf[256];

  hClientPipe =
      CreateNamedPipe(TEXT("NONE"), PIPE_ACCESS_OUTBOUND,
                      PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, 1,
                      sizeof(buf), sizeof(buf), 1000, NULL);
  if (hClientPipe == INVALID_HANDLE_VALUE) {
    _tprintf(TEXT("[ERRO] Criar Named Pipe! %d\n"), GetLastError());
    system("pause");
    exit(-1);
  }

  while (1) {
    if (!ConnectNamedPipe(hClientPipe, NULL)) {
      _tprintf(TEXT("[ERRO] Ligação ao leitor! %d\n"), GetLastError());
      system("pause");
      exit(-1);
    }
    if (!WriteFile(hClientPipe, &data, sizeof(data), &n, NULL)) {
      _tprintf(TEXT("[ERRO] Escrever no pipe! (WriteFile)\n"));
      exit(-1);
    }

    if (!DisconnectNamedPipe(hClientPipe)) {
      _tprintf(TEXT("[ERRO] Desligar o pipe! %d"), GetLastError());
      system("pause");
      exit(-1);
    }
  }
  CloseHandle(hClientPipe);
}