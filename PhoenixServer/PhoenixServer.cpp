// PhoenixServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "Game.h"
#include "GameZone.h"
#include "MessageZone.h"
#include "PhoenixServer.h"

BOOL done;

int _tmain(int argc, LPTSTR argv[]) {
  MessageData messageData;
  GameData gameData;

  DWORD threadReceiveMessagesFromServerId;
  HANDLE hThreadReceiveMessagesFromGateway;

  HANDLE runningEvent;

#ifdef UNICODE
  _setmode(_fileno(stdin), _O_WTEXT);
  _setmode(_fileno(stdout), _O_WTEXT);
#endif

  /**
   * Use an event to check if the program is running
   * Start event at instance start
   */
  if (isServerRunning()) {
    error(TEXT("There is an instance of server already running! Only 1 server "
               "at time"));
    system("pause");
    return FALSE;
  }

  DWORD threadManageEnemyShipsId;
  HANDLE hThreadManageEnemyShips;

  if (!initGameZone(&gameData)) {
    error(TEXT("Can't connect game data with server. Exiting..."));
    system("pause");
  }

  gameData.gameUpdateEvent =
      CreateEventW(NULL, FALSE, FALSE, GAME_UPDATE_EVENT);

  if (gameData.gameUpdateEvent == NULL) {
    error(TEXT("CreateEvent failed"));
    return FALSE;
  }

  if (!initMessageZone(&messageData)) {
    error(TEXT("Can't connect message data with server. Exiting..."));
    system("pause");
  }

  messageData.gatewayMessageUpdateEvent =
      CreateEventW(NULL, FALSE, FALSE, MESSAGE_GATEWAY_UPDATE_EVENT);

  if (messageData.gatewayMessageUpdateEvent == NULL) {
    error(TEXT("CreateEvent failed"));
    return FALSE;
  }

  messageData.serverMessageUpdateEvent =
      CreateEventW(NULL, FALSE, FALSE, MESSAGE_SERVER_UPDATE_EVENT);

  if (messageData.serverMessageUpdateEvent == NULL) {
    error(TEXT("CreateEvent failed"));
    return FALSE;
  }

  hThreadReceiveMessagesFromGateway =
      CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)receiveMessagesFromGateway,
                   &messageData, 0, &threadReceiveMessagesFromServerId);
  if (hThreadReceiveMessagesFromGateway == NULL) {
    error(TEXT("Creating thread to receive data from server"));
    system("pause");
    return -1;
  }

  hThreadManageEnemyShips =
      CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadManageEnemyShips,
                   &gameData, 0, &threadManageEnemyShipsId);
  if (hThreadManageEnemyShips == NULL) {
    error(TEXT("Creating thread to manage enemy ships"));
    system("pause");
    return -1;
  }

  debug(TEXT("Server started successfully"));

  /**
   * Now that everything is set up, set control handler
   */
  SetConsoleCtrlHandler(CtrlHandler, TRUE);

  /**
   * Wait running event to be released to proceed
   */
  runningEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, SERVER_RUNNING_EVENT);
  if (runningEvent != NULL) {
    WaitForSingleObject(runningEvent, INFINITE);
  }

  /**
   * If we got here, runningEvent is released
   * Handle close
   */
  handleClose(&messageData);

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

  return 0;
}

/**
 * Used before app close
 */
BOOL WINAPI CtrlHandler(DWORD dwCtrlType) {
  HANDLE serverRunningEvent;

  serverRunningEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, SERVER_RUNNING_EVENT);
  if (serverRunningEvent == NULL) {
    error(TEXT("Can't set up close event! Server will not exit "
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
      error(TEXT("Can't close event! Server will not exit "
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

VOID handleClose(MessageData *messageData) {
  Message msg;
  msg.cmd = SERVER_CLOSING;
  writeDataToSharedMemory(messageData->sharedMessage, &msg, sizeof(Message),
                          messageData->hMutex,
                          messageData->gatewayMessageUpdateEvent);
}