// phoenix_client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "phoenix_client.h"
#include "../phoenix_dll/phoenix_dll.h"
#include "../phoenix_dll/structs.h"
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
  HANDLE hPipe;
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
  hPipe = CreateFile(SERVER_PIPE_NAME, GENERIC_READ, 0, NULL, OPEN_EXISTING,
                     FILE_ATTRIBUTE_NORMAL, NULL);
  if (hPipe == NULL) {
    _tprintf(TEXT("[ERRO] Ligar ao pipe '%s'! (CreateFile)\n"),
             SERVER_PIPE_NAME);
    system("pause");
    exit(-1);
  }

  // Connected
  do {
    fSuccess = ReadFile(hPipe, buf, sizeof(buf), &n, NULL);
    buf[n / sizeof(TCHAR)] = '\0';

    if (!fSuccess || !n) {
      _tprintf(TEXT("[ERRO] %d %d... (ReadFile)\n"), fSuccess, n);
      break;
    }

    // Do what you gotta do...
  } while (!STOP);

  CloseHandle(hPipe);

  return 0;
}