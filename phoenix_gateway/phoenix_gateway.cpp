// phoenix_gateway.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"

#include "../phoenix_dll/phoenix_dll.h"
#include "../phoenix_dll/structs.h"



Game game;

int main() {
	_tprintf(TEXT("inicio\n"));
  ControlData data;
  DWORD threadReceiveId;
  HANDLE hthreadReceive;

  PodeEscrever = CreateSemaphore(NULL, Buffers, Buffers, NomeSemaforoPodeEscrever);
  PodeLer = CreateSemaphore(NULL, 0, Buffers, NomeSemaforoPodeLer);

#ifdef UNICODE
  _setmode(_fileno(stdin), _O_WTEXT);
  _setmode(_fileno(stdout), _O_WTEXT);
#endif

  if (!initMemAndSync(&data)) {
    return -1;
  }

  data.game = (Game *)MapViewOfFile(data.hMapFile, FILE_MAP_ALL_ACCESS, 0, 0,
                                    sizeof(Game));

  if (data.game == NULL) {
    _tprintf(TEXT("[Erro]Mapeamento da memória partilhada(%d)\n"),
             GetLastError());
    return -1;
  }

  data.hMutex = CreateMutex(NULL, FALSE, mReadName);

  //unsigned int current = peekData(&data);
  int i = 0;

  while (TRUE) {
    //Sleep(500);
	_tprintf(TEXT("ciclo\n"));

	WaitForSingleObject(PodeLer, INFINITE);
    //if (peekData(&data) > current) {
      readData(&data, &game);
      //current = game.num;
      system("cls");
      for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {
          _tprintf(TEXT("%c "), game.map[x][y]);
        }
        _tprintf(TEXT("\n"));
      }
    //}
	  _tprintf(TEXT("passou\n"));
	ReleaseSemaphore(PodeEscrever, 1, NULL);
	i++;
	if (i == 20)break;
  }
  _tprintf(TEXT("TheEnd\n"));
  CloseHandle(data.hMapFile);
  CloseHandle(data.hMutex);
  CloseHandle(PodeEscrever);
  CloseHandle(PodeLer);  system("pause");
  return 0;
}
