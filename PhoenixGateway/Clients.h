#pragma once

int broadcastGameToClients(Data *data, Game *game);
int broadcastMessageToClients(Data *data, Message *message);
int getClientIndex(Data *data, int clientId);
DWORD WINAPI manageClient(LPVOID lpParam);
DWORD WINAPI manageClients(LPVOID lpParam);
BOOL removeClient(Data *data, int clientId);