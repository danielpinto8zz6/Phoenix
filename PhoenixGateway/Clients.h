#pragma once

BOOL addClient(HANDLE listClients[MAXCLIENTS], HANDLE client);
int broadcastClients(HANDLE clients[MAXCLIENTS], Game *game, HANDLE writeReady);
DWORD WINAPI manageClient(LPVOID lpParam);
DWORD WINAPI manageClients(LPVOID lpParam);
BOOL removeClient(HANDLE clients[MAXCLIENTS], HANDLE client);
VOID startClients(HANDLE listClients[MAXCLIENTS]);
BOOL writeClientAsync(HANDLE hPipe, Game *game, HANDLE writeReady);