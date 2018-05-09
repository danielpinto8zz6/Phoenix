#include "stdafx.h"

#include "../phoenix_dll/phoenix_dll.h"
#include "../phoenix_dll/structs.h"
#include "game.h"
#include "phoenix_server.h"

Game game;

int _tmain(int argc, LPTSTR argv[]) {
  DWORD threadManageEnemyShipsId;
  HANDLE hThreadManageEnemyShips;
  DWORD threadManageSharedMemory;
  HANDLE hThreadManageSharedMemory;

#ifdef UNICODE
  _setmode(_fileno(stdin), _O_WTEXT);
  _setmode(_fileno(stdout), _O_WTEXT);
#endif

  FillMap();

  int Ret = 1;

  Ret = myPuts((LPWSTR)TEXT("Message sent to the DLL function\n"));

  hThreadManageEnemyShips =
      CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadManageEnemyShips,
                   NULL, 0, &threadManageEnemyShipsId);
  if (hThreadManageEnemyShips == NULL) {
    _tprintf(TEXT("CreateThread error: %d\n"), GetLastError());
    return -1;
  }

  hThreadManageSharedMemory =
	  CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadManageSharedMemory,
		  NULL, 0, &threadManageEnemyShipsId);
  if (hThreadManageSharedMemory == NULL) {
	  _tprintf(TEXT("CreateThread error: %d\n"), GetLastError());
	  return -1;
  }

  WaitForSingleObject(hThreadManageEnemyShips, INFINITE);
  WaitForSingleObject(hThreadManageSharedMemory, INFINITE);

  // DEBUG
  for (int i = 0; i < ENEMYSHIPS; i++) {
	  _tprintf(TEXT("Position X Y: (%d , %d)\n"), game.enemy_ship[i].position.x, game.enemy_ship[i].position.y);
  }

  system("pause");

  return 0;
}

DWORD WINAPI ThreadManageEnemyShips(LPVOID lpParam) {
  HANDLE aThread[ENEMYSHIPS];
  DWORD ThreadID = 0;
  int i;
  int pos[ENEMYSHIPS];

  _tprintf(TEXT("[ManageEnemyShips] -> Thread-%d\n"), GetCurrentThreadId());

  // Create a mutex with no initial owner
  EnemyShipsMutex = CreateMutex(NULL, FALSE, NULL);

  if (EnemyShipsMutex == NULL) {
    _tprintf(TEXT("CreateMutex error: %d\n"), GetLastError());
    return 1;
  }

  // Create Enemy Ships threads
  for (i = 0; i < ENEMYSHIPS; i++) {
	pos[i] = i;
    aThread[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadEnemyShip,
                              &pos[i], 0, &ThreadID);

    if (aThread[i] == NULL) {
      _tprintf(TEXT("CreateThread error: %d\n"), GetLastError());
      return 1;
    }
  }

  // Wait for all threads to terminate
  WaitForMultipleObjects(ENEMYSHIPS, aThread, TRUE, INFINITE);

  // Close thread and mutex handles
  for (i = 0; i < ENEMYSHIPS; i++)
    CloseHandle(aThread[i]);

  CloseHandle(EnemyShipsMutex);

  return 0;
}

DWORD WINAPI ThreadEnemyShip(LPVOID lpParam) {
  int position = *(int *)lpParam;

  WaitForSingleObject(EnemyShipsMutex, INFINITE);

  _tprintf(TEXT("[EnemyShip] -> %i\n"), position);

  // Posicionar a nave...
  Coordinates *c = GetFirstEmptyPosition(game);
  if (c != NULL) {
	  game.enemy_ship[position].position = *c;
	  game.map[c->x][c->y] = 'E';
  }

  // _tprintf(TEXT("[Position] -> (%i,%i)\n"), game.enemy_ship[position].position.x, game.enemy_ship[position].position.y);

  Sleep(200);

  ReleaseMutex(EnemyShipsMutex);

  return TRUE;
}

DWORD WINAPI ThreadManageSharedMemory(LPVOID lpParam) {
	_tprintf(TEXT("MANAGING SHARED MEMORY\n"));
	return TRUE;
}

void FillMap() {
	for (int x = 0; x < HEIGHT; x++) {
		for (int y = 0; y < WIDTH; y++) {
			game.map[x][y] = ' ';
		}
	}
}