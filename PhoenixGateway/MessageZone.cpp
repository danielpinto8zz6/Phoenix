﻿#include "stdafx.h"

#include "Clients.h"
#include "MessageZone.h"

DWORD WINAPI receiveMessagesFromServer(LPVOID lpParam) {
  Data *data = (Data *)lpParam;
  MessageData *messageData = data->messageData;
  Message message;

  DWORD dwWaitResult;

  messageData->STOP = FALSE;

  while (!messageData->STOP) {
    dwWaitResult =
        WaitForSingleObject(messageData->gatewayMessageUpdateEvent, INFINITE);
    if (dwWaitResult == WAIT_OBJECT_0) {
      readDataFromSharedMemory(messageData->sharedMessage, &message,
                               sizeof(Message), &messageData->hMutex);
      /**
       * TODO: Perform actions related to info received
       */
      debug(TEXT("%d Bytes received"), sizeof(Message));

      // if (message.cmd == LOGGED) {
      //   int x = getClientById(data, message.clientId);
      //   if (x != -1) {
      //     writeGameToClientAsync(data->client[x].hPipe, &game,
      //                            data->writeReady);
      //   }
      // }
      // broadcastGameToClients(data, &game, data->writeReady);
    }
  }
  return 0;
}
