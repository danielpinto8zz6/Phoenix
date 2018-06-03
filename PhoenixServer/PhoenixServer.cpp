// PhoenixServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "Game.h"
#include "GameZone.h"
#include "MessageZone.h"
#include "PhoenixServer.h"

int _tmain(int argc, LPTSTR argv[]) {

#ifdef UNICODE
  _setmode(_fileno(stdin), _O_WTEXT);
  _setmode(_fileno(stdout), _O_WTEXT);
#endif

  MessageData messageData;
  GameData gameData;

  gameData.game.num = 0;
  // Temporary: map will be abandoned
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      gameData.game.map[y][x] = ' ';
    }
  }

  DWORD threadManageEnemyShipsId;
  HANDLE hThreadManageEnemyShips;

  if (!initGameZone(&gameData)) {
    Error(TEXT("Can't connect game data with server. Exiting..."));
    system("pause");
  }

  if (!initMessageZone(&messageData)) {
    Error(TEXT("Can't connect message data with server. Exiting..."));
    system("pause");
  }

  hThreadManageEnemyShips =
      CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadManageEnemyShips,
                   &gameData, 0, &threadManageEnemyShipsId);
  if (hThreadManageEnemyShips == NULL) {
    Error(TEXT("Creating thread to manage enemy ships"));
    return -1;
  }

  WaitForSingleObject(hThreadManageEnemyShips, INFINITE);

  CloseHandle(hThreadManageEnemyShips);

  CloseHandle(gameData.smRead);
  CloseHandle(gameData.smWrite);
  CloseHandle(gameData.hMutex);
  CloseHandle(gameData.hMapFile);
  UnmapViewOfFile(gameData.sharedGame);

  system("pause");

  return 0;
}