#pragma once

#include "structs.h"

DWORD WINAPI receiveMessagesFromServer(LPVOID lpParam);
BOOL sendMessageToServer(MessageData *messageData, Message *msg);