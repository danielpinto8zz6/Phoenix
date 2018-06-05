#pragma once

DWORD WINAPI receiveMessagesFromServer(LPVOID lpParam);
BOOL sendMessageToServer(MessageData *messageData, Message *msg);