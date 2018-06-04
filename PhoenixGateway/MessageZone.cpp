#include "stdafx.h"

#include "MessageZone.h"

DWORD WINAPI receiveMessagesFromServer(LPVOID lpParam) {
  MessageData *messageData = (MessageData *)lpParam;

  DWORD dwWaitResult;

  messageData->STOP = FALSE;

  while (!messageData->STOP) {
    dwWaitResult = WaitForSingleObject(messageData->gatewayMessageUpdateEvent, INFINITE);
    if (dwWaitResult == WAIT_OBJECT_0) {
      readDataFromSharedMemory(messageData->sharedMessage, &messageData->message,
                               sizeof(Message), &messageData->hMutex);
    }

    /**
     * TODO: Perform actions related to info received
     */
    _tprintf(TEXT("DEBUG : Received -> %s\n"), messageData->message.text);
  }
  return 0;
}

BOOL sendMessageToServer(MessageData *messageData, Message *msg) {
  writeDataToSharedMemory(messageData->sharedMessage, msg, sizeof(Message),
                          &messageData->hMutex);
  if (!SetEvent(messageData->serverMessageUpdateEvent)) {
    Error(TEXT("SetEvent failed"));
  }

  return TRUE;
}