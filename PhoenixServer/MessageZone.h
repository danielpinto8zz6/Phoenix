#pragma once

DWORD WINAPI receiveMessagesFromGateway(LPVOID lpParam);
void handleCommand(Data *data, Message message);

