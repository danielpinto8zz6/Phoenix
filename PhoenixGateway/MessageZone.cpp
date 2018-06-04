#include "stdafx.h"

#include "MessageZone.h"

DWORD WINAPI receiveMessagesFromServer(LPVOID lpParam) {
  MessageData *messageData = (MessageData *)lpParam;

  DWORD current = -1;

  while (messageData->STOP) {
    if (peekMessageData(messageData) > current) {
      readDataFromSharedMemory(messageData->sharedMessage,
                               &messageData->message, sizeof(Game),
                               &messageData->hMutex);
      current = messageData->message.num;

      system("cls");

      _tprintf(TEXT("DEBUG : %d\n"), current);

      system("pause");
    }
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
  writeDataToSharedMemory(messageData->sharedMessage, msg, sizeof(Message),
                          &messageData->hMutex);

  return TRUE;
}