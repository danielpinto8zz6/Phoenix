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

  /**
   * Use an event to check if the program is running
   * Start event at instance start
   */
  if (isServerRunning()) {
    Error(TEXT("There is an instance of server already running! Only 1 server "
               "at time"));
    system("pause");
    return FALSE;
  }

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

  gameData.gameUpdateEvent =
      CreateEventW(NULL, FALSE, FALSE, GAME_UPDATE_EVENT);

  if (gameData.gameUpdateEvent == NULL) {
    Error(TEXT("CreateEvent failed"));
    return FALSE;
  }

  if (!initMessageZone(&messageData)) {
    Error(TEXT("Can't connect message data with server. Exiting..."));
    system("pause");
  }

  messageData.gatewayMessageUpdateEvent =
      CreateEventW(NULL, FALSE, FALSE, MESSAGE_GATEWAY_UPDATE_EVENT);

  if (messageData.gatewayMessageUpdateEvent == NULL) {
    Error(TEXT("CreateEvent failed"));
    return FALSE;
  }

  messageData.serverMessageUpdateEvent =
      CreateEventW(NULL, FALSE, FALSE, MESSAGE_SERVER_UPDATE_EVENT);

  if (messageData.serverMessageUpdateEvent == NULL) {
    Error(TEXT("CreateEvent failed"));
    return FALSE;
  }

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

  CloseHandle(gameData.hMutex);
  CloseHandle(gameData.hMapFile);
  CloseHandle(gameData.gameUpdateEvent);
  UnmapViewOfFile(gameData.sharedGame);

  CloseHandle(messageData.hMutex);
  CloseHandle(messageData.hMapFile);
  CloseHandle(messageData.serverMessageUpdateEvent);
  CloseHandle(messageData.gatewayMessageUpdateEvent);
  CloseHandle(hThreadReceiveMessagesFromGateway);

  UnmapViewOfFile(messageData.sharedMessage);

  system("pause");

  return 0;
}