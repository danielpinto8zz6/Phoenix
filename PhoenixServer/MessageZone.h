#pragma once

DWORD WINAPI receiveMessagesFromGateway(LPVOID lpParam);
BOOL sendMessageToGateway(MessageData *messageData, Message *msg);