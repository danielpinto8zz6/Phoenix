#include "stdafx.h"

#include "MessageZone.h"

BOOL initMessageZone(MessageData *messageData) {
  if (!initMemAndSync(&messageData->hMapFile, GAMEDATA_SHARED_MEMORY_NAME,
                      &messageData->hMutex, GAMEDATA_MUTEX_NAME)) {
    return FALSE;
  }

  messageData->smWrite =
      CreateSemaphore(NULL, MAX_SEM_COUNT, MAX_SEM_COUNT, smWriteName);
  if (messageData->smWrite == NULL) {
    Error(TEXT("Initializing write semaphore"));
    return FALSE;
  }

  messageData->smRead = CreateSemaphore(NULL, 0, MAX_SEM_COUNT, smReadName);
  if (messageData->smRead == NULL) {
    Error(TEXT("Initializing read semaphore"));
    return FALSE;
  }

  messageData->message = (Message *)MapViewOfFile(
      messageData->hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Message));

  if (messageData->message == NULL) {
    Error(TEXT("Mapping shared memory"));
    return FALSE;
  }

  return TRUE;
}

DWORD WINAPI receiveMessagesFromServer(LPVOID lpParam) {
  MessageData *messageData = (MessageData *)lpParam;

  Message msg;
  msg.num = 0;

  DWORD current = peekMessageData(messageData);

  while (!messageData->STOP) {
    // Do not get data whitout permission
    WaitForSingleObject(messageData->smRead, INFINITE);

    if (peekMessageData(messageData) > current) {
      readDataFromSharedMemory(messageData->message, &msg, sizeof(Message),
                               &messageData->hMutex);
      current = msg.num;

      /**
       * TODO: Perform actions related to info received
       */
      _tprintf(TEXT("DEBUG : Received -> %s"), msg.text);

      /**
       * TODO: Send message to client
       */
    }

    // We can send data now
    ReleaseSemaphore(messageData->smWrite, 1, NULL);
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

BOOL sendMessageToServer(MessageData *messageData, Message *msg) {

  WaitForSingleObject(messageData->smWrite, INFINITE);

  msg->num++;

  writeDataToSharedMemory(messageData->message, msg, sizeof(Message),
                          &messageData->hMutex);

  ReleaseSemaphore(messageData->smRead, 1, NULL);

  return TRUE;
}