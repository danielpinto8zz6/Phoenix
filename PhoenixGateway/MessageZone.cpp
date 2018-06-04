#include "stdafx.h"

#include "MessageZone.h"

DWORD WINAPI receiveMessagesFromServer(LPVOID lpParam) {
  MessageData *messageData = (MessageData *)lpParam;

  DWORD current = peekMessageData(messageData);

  while (messageData->STOP) {
    // Do not get data whitout permission
    WaitForSingleObject(messageData->smRead, INFINITE);

    if (peekMessageData(messageData) > current) {
      readDataFromSharedMemory(messageData->sharedMessage, &messageData->message,
                               sizeof(Game), &messageData->hMutex);
      current = messageData->message.num;

      system("cls");

       _tprintf(TEXT("DEBUG : %d\n"), current);

       system("pause");
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

BOOL sendMessageToServer(MessageData *messageData, Message *msg) {
  WaitForSingleObject(messageData->smWrite, INFINITE);

  writeDataToSharedMemory(messageData->sharedMessage, msg, sizeof(Message),
                          &messageData->hMutex);

  ReleaseSemaphore(messageData->smRead, 1, NULL);

  return TRUE;
}