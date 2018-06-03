#include "stdafx.h"

#include "MessageZone.h"

MessageData messageData;

int initMessageZone() {
  HANDLE hThreadReceiveFromGateway;
  DWORD threadReceiveFromGatewayId;

  if (!initMemAndSync(&messageData.hMapFile, GAMEDATA_SHARED_MEMORY_NAME,
                      &messageData.hMutex, GAMEDATA_MUTEX_NAME)) {
    return -1;
  }

  messageData.smWrite =
      CreateSemaphore(NULL, MAX_SEM_COUNT, MAX_SEM_COUNT, smWriteName);
  if (messageData.smWrite == NULL) {
    Error(TEXT("Initializing write semaphore"));
    return -1;
  }

  messageData.smRead = CreateSemaphore(NULL, 0, MAX_SEM_COUNT, smReadName);
  if (messageData.smRead == NULL) {
    Error(TEXT("Initializing read semaphore"));
    return -1;
  }

  messageData.message = (Message *)MapViewOfFile(
      messageData.hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Message));

  if (messageData.message == NULL) {
    Error(TEXT("Mapping shared memory"));
    return -1;
  }

  messageData.message->num = 0;

  hThreadReceiveFromGateway =
      CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)receiveFromGateway,
                   NULL, 0, &threadReceiveFromGatewayId);
  if (hThreadReceiveFromGateway == NULL) {
    Error(TEXT("Creating thread to receive data from gateway"));
    return -1;
  }

  WaitForSingleObject(hThreadReceiveFromGateway, INFINITE);

  CloseHandle(messageData.smRead);
  CloseHandle(messageData.smWrite);
  CloseHandle(hThreadReceiveFromGateway);
  CloseHandle(messageData.hMutex);
  CloseHandle(messageData.hMapFile);
  UnmapViewOfFile(messageData.message);

  return 0;
}

DWORD WINAPI receiveFromGateway(LPVOID lpParam) {
  DWORD current = peekMessageData(&messageData);
  Message msg;

  while (!messageData.STOP) {
    // Do not get data whitout permission
    WaitForSingleObject(messageData.smRead, INFINITE);

    if (peekMessageData(&messageData) > current) {
      readDataFromSharedMemory(messageData.message, &msg, sizeof(Message),
                               &messageData.hMutex);
      current = msg.num;

      /**
       * TODO: Perform actions related to info received
       */
      _tprintf(TEXT("DEBUG : Received -> %s"), msg.client->username);
    }

    // We can send data now
    ReleaseSemaphore(messageData.smWrite, 1, NULL);
  }
  return 0;
}

DWORD peekMessageData(MessageData *data) {
  DWORD num;
  WaitForSingleObject(data->hMutex, INFINITE);
  num = data->message->num;
  ReleaseMutex(data->hMutex);
  return num;
}
