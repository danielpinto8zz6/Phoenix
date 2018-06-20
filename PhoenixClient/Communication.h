#pragma once

BOOL connectPipes(Client *client);
DWORD WINAPI gameReceiver(LPVOID lpParam);
void handleCommand(Client *client, Message message);
BOOL joinGame(Client *client);
DWORD WINAPI messageReceiver(LPVOID lpParam);