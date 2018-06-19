#pragma once

BOOL addClient(Data *data, TCHAR username[50], int id);
void clientLogin(Data *data, Message message);
int getClientIndex(Game *game, int id);
BOOL removeClient(Game *game, int id);