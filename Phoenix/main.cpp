#include "stdafx.h"

#define ENEMYSHIPS 20

HANDLE EnemyShipsMutex;

DWORD WINAPI ThreadEnemyShip(LPVOID);
DWORD WINAPI ThreadManageEnemyShips(LPVOID lpParam);

typedef void(__cdecl *DLLFUNC)(LPTSTR);

void load_dll();

int _tmain(int argc, LPTSTR argv[]) {
  DWORD threadManageEnemyShipsId;
  HANDLE hThreadManageEnemyShips;

#ifdef UNICODE
  _setmode(_fileno(stdin), _O_WTEXT);
  _setmode(_fileno(stdout), _O_WTEXT);
#endif

  load_dll();

  hThreadManageEnemyShips =
      CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadManageEnemyShips,
                   NULL, 0, &threadManageEnemyShipsId);
  if (hThreadManageEnemyShips == NULL) {
    _tprintf(TEXT("CreateThread error: %d\n"), GetLastError());
    return -1;
  }

  WaitForSingleObject(hThreadManageEnemyShips, INFINITE);

  system("pause");

  return 0;
}

DWORD WINAPI ThreadManageEnemyShips(LPVOID lpParam) {
  HANDLE aThread[ENEMYSHIPS];
  DWORD ThreadID;
  int i;

  _tprintf(TEXT("[ManageEnemyShips] -> Thread-%d\n"), GetCurrentThreadId());

  // Create a mutex with no initial owner
  EnemyShipsMutex = CreateMutex(NULL, FALSE, NULL);

  if (EnemyShipsMutex == NULL) {
    _tprintf(TEXT("CreateMutex error: %d\n"), GetLastError());
    return 1;
  }

  // Create Enemy Ships threads
  for (i = 0; i < ENEMYSHIPS; i++) {
    aThread[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadEnemyShip,
                              NULL, 0, &ThreadID);

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
  // lpParam not used
  UNREFERENCED_PARAMETER(lpParam);

  WaitForSingleObject(EnemyShipsMutex, INFINITE);

  _tprintf(TEXT("[EnemyShip] -> Thread-%d\n"), GetCurrentThreadId());

  Sleep(1000);

  ReleaseMutex(EnemyShipsMutex);

  return TRUE;
}

void load_dll() {
  HINSTANCE hinstLib;
  DLLFUNC MessageHandler;

  // Get a handle to the DLL module.
  hinstLib = LoadLibrary(TEXT("PhoenixLibrary.dll"));

  // If the handle is valid, try to get the function address.
  if (hinstLib != NULL) {
    MessageHandler = (DLLFUNC)GetProcAddress(hinstLib, "MessageHandler");

    // If the function address is valid, call the function.
    if (NULL != MessageHandler) {
      MessageHandler((LPTSTR)TEXT("DLL LOADED"));
    } else {
      _tprintf(TEXT("Could not load DLL\n"));
    }

    // Free the DLL module.
    FreeLibrary(hinstLib);
  }
}