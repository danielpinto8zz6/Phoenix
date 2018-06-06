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

  HANDLE runningEvent;

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
                   &data, 0, &threadReceiveMessagesFromServerId);
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

  /**
   * Wait running event to be released to proceed
   */
  runningEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, GATEWAY_RUNNING_EVENT);
  if (runningEvent != NULL) {
    WaitForSingleObject(runningEvent, INFINITE);
  }

  /**
   * If we got here, runningEvent is released
   * Handle close
   */
  handleClose(&data);

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

  return 0;
}

BOOL WINAPI CtrlHandler(DWORD dwCtrlType) {
  HANDLE serverRunningEvent;

  serverRunningEvent =
      OpenEvent(EVENT_ALL_ACCESS, FALSE, GATEWAY_RUNNING_EVENT);
  if (serverRunningEvent == NULL) {
    error(TEXT("Can't set up close event! Gateway will not exit "
               "properly"));
    ExitThread(0);
  }

  switch (dwCtrlType) {
  case CTRL_SHUTDOWN_EVENT:
  case CTRL_CLOSE_EVENT:
  case CTRL_LOGOFF_EVENT:
  case CTRL_C_EVENT:
  case CTRL_BREAK_EVENT:
    if (!SetEvent(serverRunningEvent)) {
      error(TEXT("Can't close event! Gateway will not exit "
                 "properly"));
    }
    /**
     * Force exit after 10 sec
     */
    Sleep(10000);
    return TRUE;
  default:
    return FALSE;
  }
  return FALSE;
}

VOID handleClose(Data *data) {
  Message msg;
  msg.cmd = CLOSING;
  writeDataToSharedMemory(data->messageData->sharedMessage, &msg, sizeof(Message),
                          data->messageData->hMutex,
                          data->messageData->serverMessageUpdateEvent);
  sendMessageToAllClients(data, &msg);
}