#pragma once

BOOL addClient(Data *data, TCHAR username[50], int id);
void clientLogin(Data *data, Message message);
BOOL removeClient(Data *data, int id);