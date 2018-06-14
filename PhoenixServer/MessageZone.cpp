#include "stdafx.h"

#include "Clients.h"
#include "MessageZone.h"

DWORD WINAPI receiveMessagesFromGateway(LPVOID lpParam) {
  Data *data = (Data *)lpParam;

  MessageData *messageData = data->messageData;

  DWORD dwWaitResult;

  messageData->STOP = FALSE;

  while (!messageData->STOP) {
    dwWaitResult =
        WaitForSingleObject(messageData->serverMessageUpdateEvent, INFINITE);
    if (dwWaitResult == WAIT_OBJECT_0) {
      readDataFromSharedMemory(messageData->sharedMessage,
                               &messageData->message, sizeof(Message),
                               &messageData->hMutex);
      debug(TEXT("%d Bytes received"), sizeof(Message));
      switch (messageData->message.cmd) {
      case LOGIN:
        clientLogin(data, messageData->message);
        break;
      case CLIENT_DISCONNECTED:
        debug(TEXT("%d disconnected"), messageData->message.clientId);
        break;
      }
    }
  }
  return 0;
}
