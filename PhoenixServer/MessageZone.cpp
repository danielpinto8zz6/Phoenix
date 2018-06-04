#include "stdafx.h"

#include "MessageZone.h"

DWORD WINAPI receiveMessagesFromGateway(LPVOID lpParam) {
  MessageData *messageData = (MessageData *)lpParam;

  Message msg;

  messageData->STOP = FALSE;

  messageData->message.num = 0;

  DWORD current = peekMessageData(messageData);

  while (!messageData->STOP) {
    // Do not get data whitout permission
    WaitForSingleObject(messageData->smRead, INFINITE);

    if (peekMessageData(messageData) > current) {
      readDataFromSharedMemory(messageData->sharedMessage, &msg,
                               sizeof(Message), &messageData->hMutex);
      current = msg.num;

      /**
       * TODO: Perform actions related to info received
       */
      _tprintf(TEXT("DEBUG : Received -> %s\n"), msg.client.username);
    }

    // We can send data now
    ReleaseSemaphore(messageData->smWrite, 1, NULL);
  }
  return 0;
}

DWORD peekMessageData(MessageData *data) {
  DWORD num;
  WaitForSingleObject(data->hMutex, INFINITE);
  num = data->sharedMessage->num;
  ReleaseMutex(data->hMutex);
  return num;
}
