// phoenix_gateway.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"

#include "../phoenix_dll/phoenix_dll.h"
#include "../phoenix_dll/structs.h"
#include "phoenix_gateway.h"
#include <Windows.h>
#include <fcntl.h>
#include <io.h>
#include <process.h>
#include <tchar.h>
#include <time.h>

int _tmain() {
  ControlData data;
  DWORD threadListenerId;
  HANDLE hThreadListener;
  DWORD threadDataReceiverId;
  HANDLE hThreadDataReceiver;

  Player client[50];

#ifdef UNICODE
  _setmode(_fileno(stdin), _O_WTEXT);
  _setmode(_fileno(stdout), _O_WTEXT);
#endif

  if (!initMemAndSync(&data)) {
    return -1;
  }

  if (!initSemaphores(&data)) {
    return -1;
  }

  data.game = (Game *)MapViewOfFile(data.hMapFile, FILE_MAP_ALL_ACCESS, 0, 0,
                                    sizeof(Game));

  if (data.game == NULL) {
    _tprintf(TEXT("[Erro] Mapeamento da memória partilhada(%d)\n"),
             GetLastError());
    return -1;
  }

  hThreadListener =
      CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadListener, &data, 0,
                   &threadListenerId);
  if (hThreadListener == NULL) {
    _tprintf(TEXT("[Erro] Criar thread: %d\n"), GetLastError());
    return -1;
  }

  /**
   * Gateway thread to receive info from clients
   */
  hThreadDataReceiver =
      CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)clientsDataReceiver, 0, 0,
                   &threadDataReceiverId);
  if (hThreadDataReceiver == NULL) {
    _tprintf(TEXT("[Erro] Criar thread"));
    return -1;
  }

  WaitForSingleObject(hThreadDataReceiver, INFINITE);
  WaitForSingleObject(hThreadListener, INFINITE);

  CloseHandle(data.hMapFile);
  CloseHandle(data.hMutex);
  CloseHandle(data.smWrite);
  CloseHandle(data.smRead);
  UnmapViewOfFile(data.game);

  system("pause");

  return 0;
}

unsigned int __stdcall threadListener(LPVOID lpParam) {
  ControlData *data = (ControlData *)lpParam;
  unsigned int current = peekData(data);

  while (data->ThreadMustConinue) {
    // Do not get data whitout permission
    WaitForSingleObject(data->smRead, INFINITE);

    if (peekData(data) > current) {
      readData(data, data->game);
      current = data->game->num;

      // Clear the console
      system("cls");

      // Show the actual map of the game
      for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
          _tprintf(TEXT("%c"), data->game->map[y][x]);
        }
        _tprintf(TEXT("\n"));
      }
    }

    // We can send data now
    ReleaseSemaphore(data->smWrite, 1, NULL);
  }
  return 0;
}

// Receive data from server
LPVOID clientsDataReceiver() {
  TCHAR buf[256];
  HANDLE hGatewayPipe;
  BOOL fSuccess = FALSE;
  DWORD n;
  // Temporary
  BOOL STOP = FALSE;

  if (!WaitNamedPipe(GATEWAY_PIPE_NAME, NMPWAIT_WAIT_FOREVER)) {
    _tprintf(TEXT("[ERRO] Ligar ao pipe '%s'! (WaitNamedPipe)\n"),
             GATEWAY_PIPE_NAME);
    system("pause");
    exit(-1);
  }

  hGatewayPipe = CreateFile(GATEWAY_PIPE_NAME, GENERIC_READ, 0, NULL,
                            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hGatewayPipe == NULL) {
    _tprintf(TEXT("[ERRO] Ligar ao pipe '%s'! (CreateFile)\n"),
             GATEWAY_PIPE_NAME);
    system("pause");
    exit(-1);
  }

  // Connected
  do {
    fSuccess = ReadFile(hGatewayPipe, buf, sizeof(buf), &n, NULL);
    buf[n / sizeof(TCHAR)] = '\0';

    if (!fSuccess || !n) {
      _tprintf(TEXT("[ERRO] %d %d... (ReadFile)\n"), fSuccess, n);
      break;
    }

    // Do what you gotta do...
  } while (!STOP);

  CloseHandle(hGatewayPipe);

  return 0;
}

/**
 * To send data we specificate the client pipe by parameter
 */
LPVOID sendDataToClient(int clientId, LPVOID data) {
  DWORD n;
  HANDLE hClientPipe;

  hClientPipe =
      CreateNamedPipe(TEXT("PHOENIX-CLIENT-%d",clientId), PIPE_ACCESS_OUTBOUND,
                      PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, 1,
                      sizeof(data), sizeof(data), 1000, NULL);
  if (hClientPipe == INVALID_HANDLE_VALUE) {
    _tprintf(TEXT("[ERRO] Criar Named Pipe! %d\n"), GetLastError());
    system("pause");
    exit(-1);
  }

  if (!ConnectNamedPipe(hClientPipe, NULL)) {
    _tprintf(TEXT("[ERRO] Ligação ao leitor! %d\n"), GetLastError());
    system("pause");
    exit(-1);
  }

  /**
   * Here we send the data specified by parameter trought the pipe
   */
  if (!WriteFile(hClientPipe, &data, sizeof(data), &n, NULL)) {
    _tprintf(TEXT("[ERRO] Escrever no pipe! (WriteFile)\n"));
    exit(-1);
  }

  if (!DisconnectNamedPipe(hClientPipe)) {
    _tprintf(TEXT("[ERRO] Desligar o pipe! %d"), GetLastError());
    system("pause");
    exit(-1);
  }
  CloseHandle(hClientPipe);
}