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
      handleCommand(data, messageData->message);
    }
  }
  return 0;
}

void handleCommand(Data *data, Message message) {

  switch (message.cmd) {
  case LOGIN:
    clientLogin(data, message);
    break;
  case CLIENT_DISCONNECTED:
    break;
  case GATEWAY_DISCONNECTED:
    break;
  case KEYDOWN:
    MessageBox(NULL, TEXT("Key pressedt!"), TEXT("KEYDOWN"),
               MB_OK | MB_ICONINFORMATION);
    break;
  case KEYUP:
    MessageBox(NULL, TEXT("Key pressedt!"), TEXT("KEYUP"),
               MB_OK | MB_ICONINFORMATION);
    break;
  case KEYLEFT:
    MessageBox(NULL, TEXT("Key pressedt!"), TEXT("KEYLEFT"),
               MB_OK | MB_ICONINFORMATION);
    break;
  case KEYRIGHT:
    MessageBox(NULL, TEXT("Key pressedt!"), TEXT("KEYRIGHT"),
               MB_OK | MB_ICONINFORMATION);
    break;
  case KEYSPACE:
    MessageBox(NULL, TEXT("Key pressedt!"), TEXT("KEYSPACE"),
               MB_OK | MB_ICONINFORMATION);
    break;
  }
}