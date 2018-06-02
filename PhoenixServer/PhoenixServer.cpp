// PhoenixServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "PhoenixServer.h"
#include "Game.h"

ControlData data;

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

  if (!initSemaphores(&data)) {
    return -1;
  }

  data.game = (Game *)MapViewOfFile(data.hMapFile, FILE_MAP_ALL_ACCESS, 0, 0,
                                    sizeof(Game));

  data.game->num = 0;

  if (data.game == NULL) {
    Error(TEXT("Mapping shared memory"));
    return -1;
  }

  hThreadManageEnemyShips =
      CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadManageEnemyShips,
                   NULL, 0, &threadManageEnemyShipsId);
  if (hThreadManageEnemyShips == NULL) {
    Error(TEXT("Creating thread to manage enemy ships"));
    return -1;
  }

  WaitForSingleObject(hThreadManageEnemyShips, INFINITE);

  // DEBUG
  for (int i = 0; i < ENEMYSHIPS; i++) {
    _tprintf(TEXT("Position X Y: (%d , %d)\n"),
             data.game->enemy_ship[i].position.x,
             data.game->enemy_ship[i].position.y);
  }

  CloseHandle(data.smRead);
  CloseHandle(data.smWrite);
  CloseHandle(hThreadManageEnemyShips);
  CloseHandle(data.hMutex);
  CloseHandle(data.hMapFile);
  UnmapViewOfFile(data.game);

  system("pause");

  return 0;
}

DWORD WINAPI threadManageEnemyShips(LPVOID lpParam) {
  HANDLE aThread[ENEMYSHIPS];
  DWORD ThreadID = 0;
  int i;
  int pos[ENEMYSHIPS];

  _tprintf(TEXT("[ManageEnemyShips] -> Thread-%d\n"), GetCurrentThreadId());

  // Create a mutex with no initial owner
  EnemyShipsMutex = CreateMutex(NULL, FALSE, NULL);

  if (EnemyShipsMutex == NULL) {
    Error(TEXT("Creating enemy ships mutex"));
    return 1;
  }

  // Create Enemy Ships threads
  for (i = 0; i < ENEMYSHIPS; i++) {

    pos[i] = i;
    aThread[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadEnemyShip,
                              &pos[i], 0, &ThreadID);

    if (aThread[i] == NULL) {
      Error(TEXT("Creating enemy ship thread"));
      return 1;
    }
    Sleep(500);
  }

  // Wait for all threads to terminate
  WaitForMultipleObjects(ENEMYSHIPS, aThread, TRUE, INFINITE);

  // Close thread and mutex handles
  for (i = 0; i < ENEMYSHIPS; i++)
    CloseHandle(aThread[i]);

  CloseHandle(EnemyShipsMutex);

  return 0;
}

DWORD WINAPI threadEnemyShip(LPVOID lpParam) {
  int position = *(int *)lpParam;

  WaitForSingleObject(EnemyShipsMutex, INFINITE);

  WaitForSingleObject(data.smWrite, INFINITE);

  _tprintf(TEXT("[EnemyShip] -> %i\n"), position);

  // Place ship...
  Coordinates *c = GetFirstEmptyPosition(data.game);
  if (c != NULL) {
    data.game->enemy_ship[position].position = *c;
    data.game->map[c->y][c->x] = '#';
  }

  writeData(&data, data.game);

  ReleaseSemaphore(data.smRead, 1, NULL);

  ReleaseMutex(EnemyShipsMutex);

  return TRUE;
}
