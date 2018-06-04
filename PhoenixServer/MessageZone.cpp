#include "stdafx.h"

#include "MessageZone.h"

DWORD WINAPI receiveMessagesFromGateway(LPVOID lpParam) {
  MessageData *messageData = (MessageData *)lpParam;

  Message msg;
  DWORD dwWaitResult;

  messageData->STOP = FALSE;

  while (!messageData->STOP) {
    dwWaitResult = WaitForSingleObject(messageData->serverMessageUpdateEvent, INFINITE);
    if (dwWaitResult == WAIT_OBJECT_0) {
      readDataFromSharedMemory(messageData->sharedMessage, &msg,
                               sizeof(Message), &messageData->hMutex);
    }

    /**
     * TODO: Perform actions related to info received
     */
    _tprintf(TEXT("DEBUG : Received -> %s\n"), msg.text);
  }
  return 0;
}