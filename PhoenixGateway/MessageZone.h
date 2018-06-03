#pragma once

#include "structs.h"

DWORD peekMessageData(MessageData *data);
DWORD WINAPI receiveMessagesFromServer(LPVOID lpParam);
BOOL sendMessageToServer(MessageData *messageData, Message *msg);