#pragma once

BOOL clientLogin(Client *client);
DWORD WINAPI dataReceiver(LPVOID lpParam);
BOOL makeConnection(Client *client);
BOOL writeGatewayAsync(Client *client, Message message);