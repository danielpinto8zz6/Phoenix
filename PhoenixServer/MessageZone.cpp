#include "stdafx.h"

#include "MessageZone.h"

DWORD WINAPI receiveMessagesFromGateway(LPVOID lpParam) {
  MessageData *messageData = (MessageData *)lpParam;

  DWORD dwWaitResult;

  messageData->STOP = FALSE;

  while (!messageData->STOP) {
    dwWaitResult =
        WaitForSingleObject(messageData->serverMessageUpdateEvent, INFINITE);
    if (dwWaitResult == WAIT_OBJECT_0) {
      readDataFromSharedMemory(messageData->sharedMessage, &messageData->message,
                               sizeof(Message), &messageData->hMutex);
      debug(TEXT("%d Bytes received"), sizeof(Message));
      if (messageData->message.cmd == CLOSING) {
        debug(TEXT("Gateway is closing..."));
      }
    }
  }
  return 0;
}

BOOL sendMessageToGateway(MessageData *messageData, Message *msg) {
  writeDataToSharedMemory(messageData->sharedMessage, msg, sizeof(Message),
                          messageData->hMutex,
                          messageData->gatewayMessageUpdateEvent);
  return TRUE;
}