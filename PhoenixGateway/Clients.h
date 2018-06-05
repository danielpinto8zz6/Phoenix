#pragma once

DWORD WINAPI manageClient(LPVOID lpParam);
DWORD WINAPI manageClients(LPVOID lpParam);
BOOL sendMessageToAllClients(Data *data, Message *message);
BOOL sendMessageToClient(HANDLE hClientPipe, Message *message);