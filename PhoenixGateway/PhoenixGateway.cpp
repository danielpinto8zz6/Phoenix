// PhoenixGateway.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "Clients.h"
#include "GameZone.h"
#include "MessageZone.h"
#include "PhoenixGateway.h"

int _tmain() {
  DWORD threadReceiveDataFromClientId;
  HANDLE hThreadReceiveDataFromClient;

  HANDLE hThreadReceiveGameDataFromServer;
  DWORD threadReceiveGameDataFromServerId;

  HANDLE hThreadReceiveMessagesFromServer;
  DWORD threadReceiveMessagesFromServerId;

  GameData gameData;
  MessageData messageData;

  Data data;
  data.gameData = &gameData;
  data.messageData = &messageData;

#ifdef UNICODE
  _setmode(_fileno(stdin), _O_WTEXT);
  _setmode(_fileno(stdout), _O_WTEXT);
#endif

  if (isGatewayRunning()) {
    Error(
        TEXT("There is an instance of gateway already running! Only 1 gateway "
             "at time"));
    system("pause");
    return FALSE;
  }

  if (!isServerRunning()) {
    Error(TEXT("There's no server instance running! Start server first!"));
    system("pause");
    return FALSE;
  }

  if (!initGameZone(&gameData)) {
    Error(TEXT("Can't connect game data with server. Exiting..."));
    system("pause");
  }

  gameData.gameUpdateEvent =
      OpenEventW(EVENT_ALL_ACCESS, FALSE, GAME_UPDATE_EVENT);
  if (gameData.gameUpdateEvent == NULL) {
    Error(TEXT("OpenEvent failed"));
    return FALSE;
  }

  hThreadReceiveGameDataFromServer =
      CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)receiveGameDataFromServer,
                   &gameData, 0, &threadReceiveGameDataFromServerId);
  if (hThreadReceiveGameDataFromServer == NULL) {
    Error(TEXT("Creating shared memory thread"));
    system("pause");
    return -1;
  }

  if (!initMessageZone(&messageData)) {
    Error(TEXT("Can't connect message data with server. Exiting..."));
    system("pause");
  }

  messageData.serverMessageUpdateEvent =
      OpenEventW(EVENT_ALL_ACCESS, FALSE, MESSAGE_SERVER_UPDATE_EVENT);
  if (messageData.serverMessageUpdateEvent == NULL) {
    Error(TEXT("OpenEvent failed"));
    return FALSE;
  }

  messageData.gatewayMessageUpdateEvent =
      OpenEventW(EVENT_ALL_ACCESS, FALSE, MESSAGE_GATEWAY_UPDATE_EVENT);
  if (messageData.gatewayMessageUpdateEvent == NULL) {
    Error(TEXT("OpenEvent failed"));
    return FALSE;
  }

  hThreadReceiveMessagesFromServer =
      CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)receiveMessagesFromServer,
                   &messageData, 0, &threadReceiveMessagesFromServerId);
  if (hThreadReceiveMessagesFromServer == NULL) {
    Error(TEXT("Creating thread to receive data from server"));
    return -1;
  }

  /**
   * Gateway thread to receive info from clients
   */
  hThreadReceiveDataFromClient =
      CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)manageClients, &data, 0,
                   &threadReceiveDataFromClientId);
  if (hThreadReceiveDataFromClient == NULL) {
    Error(TEXT("Creating thread to manage clients"));
    return -1;
  }

  WaitForSingleObject(hThreadReceiveDataFromClient, INFINITE);
  WaitForSingleObject(hThreadReceiveGameDataFromServer, INFINITE);
  WaitForSingleObject(hThreadReceiveMessagesFromServer, INFINITE);

  CloseHandle(gameData.hMapFile);
  CloseHandle(gameData.hMutex);
  CloseHandle(gameData.gameUpdateEvent);

  CloseHandle(hThreadReceiveGameDataFromServer);

  UnmapViewOfFile(gameData.sharedGame);

  CloseHandle(messageData.hMutex);
  CloseHandle(messageData.hMapFile);
  CloseHandle(messageData.gatewayMessageUpdateEvent);
  CloseHandle(messageData.serverMessageUpdateEvent);
  CloseHandle(hThreadReceiveMessagesFromServer);

  UnmapViewOfFile(messageData.sharedMessage);

  system("pause");

  return 0;
}