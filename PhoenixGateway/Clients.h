#pragma once

BOOL addClient(Data *data, HANDLE client);
int broadcastGameToClients(Data *data, Game *game, HANDLE writeReady);
int getClientIndex(Data *data, HANDLE client);
DWORD WINAPI manageClient(LPVOID lpParam);
DWORD WINAPI manageClients(LPVOID lpParam);
BOOL removeClient(Data *data, HANDLE client);
BOOL writeGameToClientAsync(HANDLE hPipe, Game *game, HANDLE writeReady);