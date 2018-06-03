// PhoenixGateway.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "Clients.h"
#include "GameZone.h"
#include "MessageZone.h"
#include "PhoenixGateway.h"

typedef struct {
  MessageData *messageData;
  GameData *gameData;
} Data;

int _tmain() {
  DWORD threadDataReceiverId;
  HANDLE hThreadDataReceiver;

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

  if (!initGameZone(&gameData)) {
    Error(TEXT("Can't connect game data with server. Exiting..."));
    system("pause");
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
  hThreadDataReceiver =
      CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)manageClients, &gameData, 0,
                   &threadDataReceiverId);
  if (hThreadDataReceiver == NULL) {
    Error(TEXT("Creating thread to manage clients"));
    return -1;
  }

  WaitForSingleObject(hThreadDataReceiver, INFINITE);
  WaitForSingleObject(hThreadReceiveGameDataFromServer, INFINITE);
  WaitForSingleObject(hThreadReceiveMessagesFromServer, INFINITE);

  CloseHandle(gameData.hMapFile);
  CloseHandle(gameData.hMutex);
  CloseHandle(gameData.smWrite);
  CloseHandle(gameData.smRead);
  CloseHandle(hThreadReceiveGameDataFromServer);

  UnmapViewOfFile(gameData.sharedGame);

  CloseHandle(messageData.smRead);
  CloseHandle(messageData.smWrite);
  CloseHandle(messageData.hMutex);
  CloseHandle(messageData.hMapFile);
  CloseHandle(hThreadReceiveMessagesFromServer);

  UnmapViewOfFile(messageData.message);

  system("pause");

  return 0;
}