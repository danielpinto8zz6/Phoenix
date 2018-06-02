// PhoenixGateway.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "structs.h"
#include "PhoenixGateway.h"
#include "Clients.h"

int _tmain() {
  ControlData data;
  DWORD threadListenerId;
  HANDLE hThreadListener;
  DWORD threadDataReceiverId;
  HANDLE hThreadDataReceiver;

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
    Error(TEXT("Mapping shared memory"));
    return -1;
  }

  hThreadListener =
      CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadListener, &data, 0,
                   &threadListenerId);
  if (hThreadListener == NULL) {
    Error(TEXT("Creating shared memory thread"));
    return -1;
  }

  /**
   * Gateway thread to receive info from clients
   */
  hThreadDataReceiver =
      CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)manageClients, &data, 0,
                   &threadDataReceiverId);
  if (hThreadDataReceiver == NULL) {
    Error(TEXT("Creating thread to manage clients"));
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

DWORD WINAPI threadListener(LPVOID lpParam) {
  ControlData *data = (ControlData *)lpParam;
  unsigned int current = peekData(data);

  while (data->ThreadMustConinue) {
    // Do not get data whitout permission
    WaitForSingleObject(data->smRead, INFINITE);

    if (peekData(data) > current) {
      readDataFromSharedMemory(data, data->game);
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
