#pragma once

BOOL addClient(Data *data, TCHAR username[50], int id);
void clientLogin(Data *data, Message message);
int getClientIndex(Data *data, int id);
BOOL removeClient(Data *data, int id);