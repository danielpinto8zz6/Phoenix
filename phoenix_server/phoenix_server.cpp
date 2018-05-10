#include "stdafx.h"

#include "../phoenix_dll/phoenix_dll.h"
#include "../phoenix_dll/structs.h"
#include "game.h"
#include "phoenix_server.h"

ControlData data;
HANDLE sem;

int _tmain(int argc, LPTSTR argv[]) {
  DWORD threadManageEnemyShipsId;
  HANDLE hThreadManageEnemyShips;



#ifdef UNICODE
  _setmode(_fileno(stdin), _O_WTEXT);
  _setmode(_fileno(stdout), _O_WTEXT);
#endif

  if (!initMemAndSync(&data)) {
    return -1;
  }

  data.game = (Game *)MapViewOfFile(data.hMapFile, FILE_MAP_ALL_ACCESS, 0, 0,
                                    sizeof(Game));

  data.game->num = 0;
 

  if (data.game == NULL) {
    _tprintf(TEXT("[Erro]Mapeamento da memória partilhada(%d)\n"),
             GetLastError());
    return -1;
  }


  hThreadManageEnemyShips =
      CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadManageEnemyShips,
                   NULL, 0, &threadManageEnemyShipsId);
  if (hThreadManageEnemyShips == NULL) {
    _tprintf(TEXT("CreateThread error: %d\n"), GetLastError());
    return -1;
  }


  WaitForSingleObject(hThreadManageEnemyShips, INFINITE);

  // DEBUG
  for (int i = 0; i < ENEMYSHIPS; i++) {
    _tprintf(TEXT("Position X Y: (%d , %d)\n"),
             data.game->enemy_ship[i].position.x,
             data.game->enemy_ship[i].position.y);
  }

  CloseHandle(PodeEscrever);
  CloseHandle(PodeLer);
  system("pause");

  return 0;
}

DWORD WINAPI ThreadManageEnemyShips(LPVOID lpParam) {
  HANDLE aThread[ENEMYSHIPS];
  DWORD ThreadID = 0;
  int i;
  int pos[ENEMYSHIPS];

  PodeEscrever = CreateSemaphore(NULL, Buffers, Buffers, NomeSemaforoPodeEscrever);
  PodeLer = CreateSemaphore(NULL, 0, Buffers, NomeSemaforoPodeLer);

  _tprintf(TEXT("[ManageEnemyShips] -> Thread-%d\n"), GetCurrentThreadId());

  // Create a mutex with no initial owner
  EnemyShipsMutex = CreateMutex(NULL, FALSE, NULL);

  if (EnemyShipsMutex == NULL) {
    _tprintf(TEXT("CreateMutex error: %d\n"), GetLastError());
    return 1;
  }
  // acho que é irrelevante  para começar  ojogo....
 // WaitForSingleObject(PodeEscrever, INFINITE);
  // Create Enemy Ships threads
  for (i = 0; i < ENEMYSHIPS; i++) {

    pos[i] = i;
    aThread[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadEnemyShip,
                              &pos[i], 0, &ThreadID);

    if (aThread[i] == NULL) {
      _tprintf(TEXT("CreateThread error: %d\n"), GetLastError());
      return 1;
    }
	Sleep(1000);
  }

  //ReleaseSemaphore(PodeLer, 1, NULL);

  // Wait for all threads to terminate
  WaitForMultipleObjects(ENEMYSHIPS, aThread, TRUE, INFINITE);
  WaitForSingleObject(PodeEscrever, INFINITE);


  // 
  // Close thread and mutex handles
  for (i = 0; i < ENEMYSHIPS; i++)
    CloseHandle(aThread[i]);

  CloseHandle(EnemyShipsMutex);
CloseHandle(data.semaphor);
  return 0;
}

DWORD WINAPI ThreadEnemyShip(LPVOID lpParam) {
  int position = *(int *)lpParam;

  //WaitForSingleObject(EnemyShipsMutex, INFINITE);

  WaitForSingleObject(PodeEscrever, INFINITE);
  _tprintf(TEXT("[EnemyShip] -> %i\n"), position);

  // Posicionar a nave...
  Coordinates *c = GetFirstEmptyPosition(data.game);
  if (c != NULL) {
    data.game->enemy_ship[position].position = *c;
    data.game->map[c->x][c->y] = '#';
  }

  writeData(&data, data.game);
  ReleaseSemaphore(PodeLer, 1, NULL);


 // ReleaseMutex(EnemyShipsMutex);

  return TRUE;
}