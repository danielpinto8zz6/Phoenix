#include "stdafx.h"

#include "MessageZone.h"

DWORD WINAPI receiveMessagesFromServer(LPVOID lpParam) {
  MessageData *messageData = (MessageData *)lpParam;

  DWORD dwWaitResult;

  messageData->STOP = FALSE;

  while (!messageData->STOP) {
    dwWaitResult =
        WaitForSingleObject(messageData->gatewayMessageUpdateEvent, INFINITE);
    if (dwWaitResult == WAIT_OBJECT_0) {
      readDataFromSharedMemory(messageData->sharedMessage,
                               &messageData->message, sizeof(Message),
                               &messageData->hMutex);
      /**
       * TODO: Perform actions related to info received
       */
      debug(TEXT("%d Bytes received"), sizeof(Message));
    }
  }
  return 0;
}

BOOL sendMessageToServer(MessageData *messageData, Message *msg) {
  writeDataToSharedMemory(messageData->sharedMessage, msg, sizeof(Message),
                          &messageData->hMutex);
  if (!SetEvent(messageData->serverMessageUpdateEvent)) {
    error(TEXT("SetEvent failed"));
  }

  return TRUE;
}