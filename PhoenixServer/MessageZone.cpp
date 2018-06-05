#include "stdafx.h"

#include "MessageZone.h"

DWORD WINAPI receiveMessagesFromGateway(LPVOID lpParam) {
  MessageData *messageData = (MessageData *)lpParam;

  Message msg;
  DWORD dwWaitResult;

  messageData->STOP = FALSE;

  while (!messageData->STOP) {
    dwWaitResult =
        WaitForSingleObject(messageData->serverMessageUpdateEvent, INFINITE);
    if (dwWaitResult == WAIT_OBJECT_0) {
      readDataFromSharedMemory(messageData->sharedMessage, &msg,
                               sizeof(Message), &messageData->hMutex);
      debug(TEXT("%d Bytes received"), sizeof(Message));
      sendMessageToGateway(messageData, &msg);
    }
  }
  return 0;
}

BOOL sendMessageToGateway(MessageData *messageData, Message *msg) {
  writeDataToSharedMemory(messageData->sharedMessage, msg, sizeof(Message),
                          &messageData->hMutex);
  if (!SetEvent(messageData->gatewayMessageUpdateEvent)) {
    error(TEXT("SetEvent failed"));
  }

  return TRUE;
}