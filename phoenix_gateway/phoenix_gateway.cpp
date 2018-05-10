// phoenix_gateway.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"

#include "../phoenix_dll/phoenix_dll.h"
#include "../phoenix_dll/structs.h"

int main() {
  ControlData data;
  DWORD threadReceiveId;
  HANDLE hthreadReceive;

#ifdef UNICODE
  _setmode(_fileno(stdin), _O_WTEXT);
  _setmode(_fileno(stdout), _O_WTEXT);
#endif

  if (!initSemaphores(&data)) {
    return -1;
  }

  if (!initMemAndSync(&data)) {
    return -1;
  }

  data.game =
      (Game *)MapViewOfFile(data.hMapFile, FILE_MAP_WRITE, 0, 0, sizeof(Game));

  if (data.game == NULL) {
    _tprintf(TEXT("[Erro]Mapeamento da memória partilhada(%d)\n"),
             GetLastError());
    return -1;
  }

  data.hMutex = CreateMutex(NULL, FALSE, mReadName);

  for (int i = 0;; ++i) {
    WaitForSingleObject(data.smRead, INFINITE);

    WaitForSingleObject(data.hMutex, INFINITE);

    system("cls");
    for (int x = 0; x < HEIGHT; x++) {
      for (int y = 0; y < WIDTH; y++) {
        _tprintf(TEXT("%c "), data.game->map[x][y]);
      }
    }

    // Libertar Mutex... com nome "mE"
    ReleaseMutex(data.hMutex);

    ReleaseSemaphore(data.smRead, 1, NULL);
  }

  CloseHandle(data.smRead);
  CloseHandle(data.smWrite);
  CloseHandle(data.hMapFile);
  CloseHandle(data.hMutex);

  return 0;
}
