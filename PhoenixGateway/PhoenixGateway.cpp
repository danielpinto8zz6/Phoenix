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
    error(
        TEXT("There is an instance of gateway already running! Only 1 gateway "
             "at time"));
    system("pause");
    return FALSE;
  }

  if (!isServerRunning()) {
    error(TEXT("There is no server instance running! Start server first!"));
    system("pause");
    return FALSE;
  }

  if (!initGameZone(&gameData)) {
    error(TEXT("Can't connect game data with server. Exiting..."));
    system("pause");
  }

  gameData.gameUpdateEvent =
      OpenEventW(EVENT_ALL_ACCESS, FALSE, GAME_UPDATE_EVENT);
  if (gameData.gameUpdateEvent == NULL) {
    error(TEXT("OpenEvent failed"));
    return FALSE;
  }

  hThreadReceiveGameDataFromServer =
      CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)receiveGameDataFromServer,
                   &data, 0, &threadReceiveGameDataFromServerId);
  if (hThreadReceiveGameDataFromServer == NULL) {
    error(TEXT("Creating shared memory thread"));
    system("pause");
    return -1;
  }

  if (!initMessageZone(&messageData)) {
    error(TEXT("Can't connect message data with server. Exiting..."));
    system("pause");
  }

  messageData.serverMessageUpdateEvent =
      OpenEventW(EVENT_ALL_ACCESS, FALSE, MESSAGE_SERVER_UPDATE_EVENT);
  if (messageData.serverMessageUpdateEvent == NULL) {
    error(TEXT("OpenEvent failed"));
    return FALSE;
  }

  messageData.gatewayMessageUpdateEvent =
      OpenEventW(EVENT_ALL_ACCESS, FALSE, MESSAGE_GATEWAY_UPDATE_EVENT);
  if (messageData.gatewayMessageUpdateEvent == NULL) {
    error(TEXT("OpenEvent failed"));
    return FALSE;
  }

  hThreadReceiveMessagesFromServer =
      CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)receiveMessagesFromServer,
                   &messageData, 0, &threadReceiveMessagesFromServerId);
  if (hThreadReceiveMessagesFromServer == NULL) {
    error(TEXT("Creating thread to receive data from server"));
    return -1;
  }

  /**
   * Gateway thread to receive info from clients
   */
  hThreadReceiveDataFromClient =
      CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)manageClients, &data, 0,
                   &threadReceiveDataFromClientId);
  if (hThreadReceiveDataFromClient == NULL) {
    error(TEXT("Creating thread to manage clients"));
    return -1;
  }

  /**
   * Now that everything is set up, set control handler
   */
  SetConsoleCtrlHandler(CtrlHandler, TRUE);

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

/**
 * Used before app close
 */
BOOL WINAPI CtrlHandler(DWORD dwCtrlType) {
  switch (dwCtrlType) {
  case CTRL_SHUTDOWN_EVENT:
  case CTRL_CLOSE_EVENT:
  case CTRL_LOGOFF_EVENT:
  case CTRL_C_EVENT:
  case CTRL_BREAK_EVENT:
    // TODO
    return TRUE;
  default:
    // We don't care about this event
    // Default handler is used
    return FALSE;
  }
  return FALSE;
}