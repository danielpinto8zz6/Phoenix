// PhoenixServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "Game.h"
#include "GameZone.h"
#include "MessageZone.h"
#include "PhoenixServer.h"

int _tmain(int argc, LPTSTR argv[]) {
  MessageData messageData;
  GameData gameData;

  DWORD threadReceiveMessagesFromServerId;
  HANDLE hThreadReceiveMessagesFromGateway;

#ifdef UNICODE
  _setmode(_fileno(stdin), _O_WTEXT);
  _setmode(_fileno(stdout), _O_WTEXT);
#endif

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

  messageData.sharedMessage->num = 0;

  hThreadReceiveMessagesFromGateway =
      CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)receiveMessagesFromGateway,
                   &messageData, 0, &threadReceiveMessagesFromServerId);
  if (hThreadReceiveMessagesFromGateway == NULL) {
    Error(TEXT("Creating thread to receive data from server"));
    system("pause");
    return -1;
  }

  hThreadManageEnemyShips =
      CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadManageEnemyShips,
                   &gameData, 0, &threadManageEnemyShipsId);
  if (hThreadManageEnemyShips == NULL) {
    Error(TEXT("Creating thread to manage enemy ships"));
    system("pause");
    return -1;
  }

  WaitForSingleObject(hThreadManageEnemyShips, INFINITE);
  WaitForSingleObject(hThreadReceiveMessagesFromGateway, INFINITE);

  CloseHandle(hThreadManageEnemyShips);

  CloseHandle(gameData.smRead);
  CloseHandle(gameData.smWrite);
  CloseHandle(gameData.hMutex);
  CloseHandle(gameData.hMapFile);
  UnmapViewOfFile(gameData.sharedGame);

  CloseHandle(messageData.smRead);
  CloseHandle(messageData.smWrite);
  CloseHandle(messageData.hMutex);
  CloseHandle(messageData.hMapFile);
  CloseHandle(hThreadReceiveMessagesFromGateway);

  UnmapViewOfFile(messageData.sharedMessage);

  system("pause");

  return 0;
}