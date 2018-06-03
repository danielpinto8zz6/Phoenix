#pragma once

#include "structs.h"

BOOL initMessageZone(MessageData *messageData);
DWORD peekMessageData(MessageData *data);
DWORD WINAPI receiveMessagesFromServer(LPVOID lpParam);
BOOL sendMessageToServer(MessageData *messageData, Message *msg);