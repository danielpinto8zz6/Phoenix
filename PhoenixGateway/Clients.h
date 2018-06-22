#pragma once

int getClientIndex(Data *data, int id);
int addClient(Data *data, int id, HANDLE hPipeMessage, HANDLE hPipeGame);
int broadcastGameToClients(Data *data, Game *game);
int broadcastMessageToClients(Data *data, Message *message);
DWORD WINAPI manageClient(LPVOID lpParam);
DWORD WINAPI manageClients(LPVOID lpParam);
BOOL removeClient(Data *data, int id);