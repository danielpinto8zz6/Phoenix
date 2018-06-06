#include "stdafx.h"

#include "MessageZone.h"
#include "Clients.h"

DWORD WINAPI receiveMessagesFromServer(LPVOID lpParam) {
  Data *data = (Data *)lpParam;
  MessageData *messageData = data->messageData;

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
      if (messageData->message.cmd == CLOSING){
        debug(TEXT("Server is closing..."));
        sendMessageToAllClients(data, &messageData->message);
      }
    }
  }
  return 0;
}

BOOL sendMessageToServer(MessageData *messageData, Message *msg) {
  writeDataToSharedMemory(messageData->sharedMessage, msg, sizeof(Message),
                          &messageData->hMutex, messageData->serverMessageUpdateEvent);

  return TRUE;
}