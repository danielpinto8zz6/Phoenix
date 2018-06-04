#include "stdafx.h"

#include "MessageZone.h"

DWORD WINAPI receiveMessagesFromGateway(LPVOID lpParam) {
  MessageData *messageData = (MessageData *)lpParam;

  Message msg;

  messageData->STOP = FALSE;

  DWORD current = peekMessageData(messageData);

  while (!messageData->STOP) {
    // Do not get data whitout permission
    if (peekMessageData(messageData) > current) {
      readDataFromSharedMemory(messageData->sharedMessage, &msg,
                               sizeof(Message), &messageData->hMutex);
      current = msg.num;

      /**
       * TODO: Perform actions related to info received
       */
      _tprintf(TEXT("DEBUG : Received -> %s\n"), msg.text);
    }

    // We can send data now
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
