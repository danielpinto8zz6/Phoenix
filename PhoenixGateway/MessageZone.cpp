#include "stdafx.h"

#include "Clients.h"
#include "MessageZone.h"

DWORD WINAPI receiveMessagesFromServer(LPVOID lpParam) {
  Data *data = (Data *)lpParam;
  MessageData *messageData = data->messageData;
  Message message;

  DWORD dwWaitResult;

  int position;

  messageData->STOP = FALSE;

  while (!messageData->STOP) {
    dwWaitResult =
        WaitForSingleObject(messageData->gatewayMessageUpdateEvent, INFINITE);
    if (dwWaitResult == WAIT_OBJECT_0) {
      readDataFromSharedMemory(messageData->sharedMessage, &message,
                               sizeof(Message), &messageData->hMutex);

      switch (message.cmd) {
      case GAME_OVER:
      case IN_GAME:
      case LOGGED:
        position = getClientIndex(data, message.clientId);
        if (position != -1) {
          writeDataToPipeAsync(data->client[position].hPipeMessage,
                               data->hEvent, &message, sizeof(Message));
        }
        break;
      case SERVER_DISCONNECTED:
        error(TEXT(
            "Server disconnected! Closing gateway & informing clients..."));
        broadcastMessageToClients(data, &message);
        return 0;
        break;
      default:
        broadcastMessageToClients(data, &message);
        break;
      }
    }
  }
  return 0;
}
